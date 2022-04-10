#include "xmidix_file.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <QTextCodec>

#define PARSER_BUF_SIZE 16

std::string decode_string(MidiEvent *event) {
  auto txt = dynamic_cast<MidiMessage *>(event)->getMetaContent();
  auto codec = QTextCodec::codecForName("Shift-JIS");
  auto decoded = codec->toUnicode(txt.c_str());

  return decoded.toStdString();
}

void xmidix_file::handle_meta_event(MidiEvent *event, snd_seq_event_t *seq_event) {
  if (event->isTempo()) {
    snd_seq_ev_set_queue_tempo(seq_event, 0, event->getTempoMicro());
    return;
  }

  if (event->isText()) {
    texts.emplace_back(decode_string(event));
    return;
  }

  if (event->isMarkerText()) {
    markers.emplace_back(xmidix_marker_t{
      event->tick,
      decode_string(event)
    });
    return;
  }

  if (event->isTrackName()) {
    track_names[event->track] = decode_string(event);
    return;
  }
}

xmidix_file::xmidix_file(const std::string &filename) {
  MidiFile midifile;
  if (!midifile.read(filename)) {
    spdlog::warn("could not open MIDI file...");
    return;
  }

  midifile.joinTracks();
  ppqn = midifile.getTicksPerQuarterNote();

  snd_midi_event_t *parser;
  snd_midi_event_new(PARSER_BUF_SIZE, &parser);

  auto midi_events = midifile[0]; // only support single track atm
  for (int i = 0; i < midi_events.getEventCount(); i++) {
    auto event = midi_events[i];

    snd_seq_event_t seq_event;
    snd_seq_ev_clear(&seq_event);

    snd_midi_event_reset_encode(parser);
    auto result = snd_midi_event_encode(parser, event.data(), static_cast<long>(event.size()), &seq_event);
    if (result <= 0) {
      spdlog::warn("error encoding midi event at tick = {}", event.tick);
      continue;
    }

    if (event.isMeta()) {
      handle_meta_event(&event, &seq_event);
    }

    seq_event.time.tick = event.tick;
    events.emplace_back(seq_event);
  }

  for (const auto& t : texts) {
    spdlog::info("text = [{}]", t);
  }

  for (const auto& m : markers) {
    spdlog::info("marker = {}: [{}]", m.tick, m.name);
  }

  for (const auto& [k, v] : track_names) {
    spdlog::info("track = {}: [{}]", k, v);
  }

  snd_midi_event_free(parser);
}

xmidix_file::~xmidix_file() {
  spdlog::debug("destructing xmidix_file");
}
