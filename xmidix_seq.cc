#include "xmidix_seq.h"

#include <chrono>

#define DEFAULT_TEMPO 500000

xmidix_seq::xmidix_seq(int client, int port)
    : QObject{nullptr}, seq{nullptr}, thread{nullptr}, port_ref{-1},
      last_tick{0}, division{1}, client_id{-1}, port_id{-1}, tempo{DEFAULT_TEMPO} {
  if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
    spdlog::error("could not open sequencer.");
    return;
  }

  snd_seq_set_client_name(seq, "XMIDI OUTPUT");
  snd_seq_create_simple_port(
      seq, "XMIDI DATA",
      SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC);

  snd_seq_nonblock(seq, true);
}

xmidix_seq::~xmidix_seq() {
  spdlog::debug("destructing xmidix_seq.");
  stop();
  snd_seq_close(seq);
  spdlog::debug("xmidix_seq destroyed.");
}

void xmidix_seq::connect(int client, int port) {
  if (client_id != -1) {
    snd_seq_disconnect_to(seq, 0, client_id, port_id);
  }

  port_ref = snd_seq_connect_to(seq, 0, client_id = client, port_id = port);
}

void xmidix_seq::thread_loop() {
  spdlog::info("xmidix_seq thread starting.. # events = {}", events.size());

start:
  unsigned start_from = 0;
  if (resume) {
    start_from = index_for_tick(last_tick);
    spdlog::debug("found resume tick {} at index {}", last_tick, start_from);
    resume = false;
  }

  spdlog::debug("beginning song");
  snd_seq_event_t event;
  current_tick = last_tick = events[start_from].time.tick;

  for (current_index = start_from; current_index < events.size(); current_index++) {
    if (stopped) {
      spdlog::info("xmidix_seq: stopped == true, breaking.");
      break;
    }

    {
      std::lock_guard lock(seek_mutex);
      event = events[current_index];
      last_tick = current_tick;
      current_tick = event.time.tick;
    }

    if (current_tick > last_tick) {
      snd_seq_drain_output(seq);

      auto delta = current_tick - last_tick;
      auto mspp = tempo / division;
      auto duration = std::chrono::microseconds{delta * mspp};

      if (duration > std::chrono::seconds{10}) {
        spdlog::warn("next tick is > 10s... something might be wrong: {}s",
                     std::chrono::duration_cast<std::chrono::seconds>(duration).count());
        duration = std::chrono::seconds{10};
      }

      std::this_thread::sleep_for(duration);
    }

    send_event(event);
  }

  if (!stopped && repeat) {
    spdlog::info("looping...");
    emit song_looping();
    goto start;
  }

  if (!stopped) {
    emit song_complete();
  }

  all_notes_off();
  spdlog::info("xmidix_seq thread complete.");
}

void xmidix_seq::load(const xmidix_file &file) {
  panic();
  events = file.events;
  division = file.ppqn;
  resume = false;
}

void xmidix_seq::play() {
  if (!stopped) return;

  thread = std::make_unique<std::thread>(&xmidix_seq::thread_loop, this);
  stopped = false;
}

void xmidix_seq::unpause() {
  if (!stopped) return;

  resume = true;
  play();
}

void xmidix_seq::stop() {
  snd_seq_drop_output_buffer(seq);
  snd_seq_drop_output(seq);
  all_notes_off(true);

  stopped = true;
  resume = false;

  if (thread != nullptr && thread->joinable()) {
    spdlog::info("waiting for midi thread to finish..");
    thread->join();
    spdlog::info("midi thread finished.");
    thread = nullptr;
  }
}

void xmidix_seq::panic() {
  all_notes_off(true);
  stop();
}

void xmidix_seq::send_event(snd_seq_event_t ev) {
  if (ev.type == SND_SEQ_EVENT_TEMPO) {
    snd_seq_ev_queue_control_t data = ev.data.queue;
    tempo = data.param.value;
    spdlog::debug("got tempo event = {}", tempo);
    return;
  }

  snd_seq_ev_set_source(&ev, port_ref);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_priority(&ev, 0);
  snd_seq_ev_set_direct(&ev);

  int result;
  if ((result = snd_seq_event_output(seq, &ev)) < 0) {
    spdlog::warn("error sending event at tick {}: {}", ev.time.tick, result);
  }

  snd_seq_drain_output(seq);
  emit midi_event(ev);
}

void xmidix_seq::all_notes_off(bool kill) {
  send_to_all(MIDI_CTL_ALL_NOTES_OFF, 0);
  if (kill) {
    send_to_all(MIDI_CTL_ALL_SOUNDS_OFF, 0);
  }
}

void xmidix_seq::send_to_all(int cc, int v) {
  for (int i = 0; i < 16; i++) {
    snd_seq_event_t e;
    snd_seq_ev_clear(&e);
    snd_seq_ev_set_controller(&e, i, cc, v);
    send_event(e);
  }
}

std::vector<xmidix_client_info> xmidix_seq::enumerate_clients() {
  std::vector<xmidix_client_info> clients;

  snd_seq_client_info_t *c_info;
  snd_seq_client_info_malloc(&c_info);
  snd_seq_client_info_set_client(c_info, -1);

  snd_seq_port_info_t *p_info;
  snd_seq_port_info_malloc(&p_info);

  while (snd_seq_query_next_client(seq, c_info) == 0) {
    auto c_id = snd_seq_client_info_get_client(c_info);
    auto c_name = snd_seq_client_info_get_name(c_info);

    snd_seq_port_info_set_client(p_info, c_id);
    snd_seq_port_info_set_port(p_info, -1);

    while (snd_seq_query_next_port(seq, p_info) == 0) {
      auto p_id = snd_seq_port_info_get_port(p_info);
      auto p_name = snd_seq_port_info_get_name(p_info);
      auto caps = snd_seq_port_info_get_capability(p_info);

      if (caps & SND_SEQ_PORT_CAP_WRITE) {
        clients.emplace_back(xmidix_client_info{
            c_name, c_id, p_name, p_id
        });
      }
    }
  }

  snd_seq_port_info_free(p_info);
  snd_seq_client_info_free(c_info);
  return clients;
}

int xmidix_seq::index_for_tick(unsigned int tick) {
  for (int i = 0; i < events.size(); i++) {
    if (tick <= events[i].time.tick) {
      return i;
    }
  }
  return 0;
}

void xmidix_seq::seek(unsigned int tick) {
  std::lock_guard lock(seek_mutex);
  all_notes_off(true);
  current_tick = last_tick = tick;
  current_index = index_for_tick(tick);
}