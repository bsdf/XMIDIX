#ifndef XMIDI__XMIDI_SEQ_H_
#define XMIDI__XMIDI_SEQ_H_

#include <alsa/asoundlib.h>
#include <spdlog/spdlog.h>

#include <thread>
#include <memory>
#include <vector>
#include <QObject>

#include "xmidix_file.h"

Q_DECLARE_METATYPE(snd_seq_event_t);
[[maybe_unused]] static const int typeId = qRegisterMetaType<snd_seq_event_t>();

typedef struct {
  std::string client_name;
  int client_id;
  std::string port_name;
  int port_id;
} xmidix_client_info;

class xmidix_seq : public QObject {
 Q_OBJECT
 public:
  explicit xmidix_seq(int client = 14, int port = 0);
  ~xmidix_seq() override;

  void connect(int client = 14, int port = 0);
  void load(const xmidix_file &file);
  void play();
  void stop();
  void panic();
  void unpause();
  void set_repeat(bool r) { repeat = r; }
  void seek(unsigned int tick);

  std::vector<xmidix_client_info> enumerate_clients();
  [[nodiscard]] unsigned get_tick() const { return last_tick; }

 signals:
  void song_complete();
  void song_looping();
  void midi_event(const snd_seq_event_t &event);

 private:
  void thread_loop();
  void send_event(snd_seq_event_t ev);
  void all_notes_off(bool kill = false);
  void send_to_all(int cc, int v);
  int index_for_tick(unsigned int tick);

  snd_seq_t *seq;

  std::unique_ptr<std::thread> thread;
  std::vector<snd_seq_event_t> events;

  bool stopped{true};
  bool resume{false};
  bool repeat{false};

  int port_ref;
  int client_id;
  int port_id;

  unsigned int current_index;
  unsigned last_tick;
  unsigned current_tick;
  unsigned tempo;
  unsigned division;

  std::mutex seek_mutex;
};

#endif //XMIDI__XMIDI_SEQ_H_
