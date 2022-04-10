#ifndef XMIDIX__XMIDIX_PIANO_H_
#define XMIDIX__XMIDIX_PIANO_H_

#include <QWidget>
#include <QFrame>
#include <QDialog>
#include <alsa/asoundlib.h>

class xmidix_piano : public QWidget {
 public:
  explicit xmidix_piano(QWidget *parent = nullptr, std::shared_ptr<QImage> grad = nullptr);

  void clear() {
    for (int i = 0; i < 128; i++)
      key_state[i] = false;

    repaint();
  }

  void set_key(int key, uint8_t velocity) {
    key_state[key] = velocity;
    repaint();
  }

 protected:
  void paintEvent(QPaintEvent *event) override;

 private:
  uint8_t key_state[128] = {0};
  std::shared_ptr<QImage> gradient;
};

class xmidix_piano_array : public QDialog {
 Q_OBJECT
 public:
  explicit xmidix_piano_array(QWidget *parent = nullptr);
  void clear() {
    for (const auto &p : pianos)
      p->clear();
  }

 public slots:
  void midi_event(const snd_seq_event_t &event);

 protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;

 private:
  std::vector<std::shared_ptr<xmidix_piano>> pianos;
  QPoint position;
};

#endif //XMIDIX__XMIDIX_PIANO_H_
