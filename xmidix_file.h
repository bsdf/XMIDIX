#ifndef ARP__XMIDI_FILE_H_
#define ARP__XMIDI_FILE_H_

#include <string>
#include <vector>
#include <map>

#include <alsa/asoundlib.h>
#include <MidiFile.h>
#include <MidiEventList.h>
#include <MidiEvent.h>

using namespace smf;

typedef struct {
  int tick;
  std::string name;
} xmidix_marker_t;

class xmidix_file {
 public:
  explicit xmidix_file(const std::string &filename);
  ~xmidix_file();

  std::vector<snd_seq_event_t> events;
  std::vector<xmidix_marker_t> markers;
  std::map<int, std::string> track_names;
  std::vector<std::string> texts;
  int ppqn;

 private:
  void handle_meta_event(MidiEvent *event, snd_seq_event_t *seq_event);
};

#endif //ARP__XMIDI_FILE_H_
