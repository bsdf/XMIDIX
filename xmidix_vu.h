#ifndef XMIDIX__XMIDIX_VU_H_
#define XMIDIX__XMIDIX_VU_H_

#include <QWidget>
#include <QPainter>
#include <QFrame>
#include <QImage>

#include <alsa/seq_event.h>
#include <vector>
#include <memory>

class xmidix_vu : public QWidget {
 Q_OBJECT
 public:
  explicit xmidix_vu(QWidget *parent = nullptr, std::shared_ptr<QImage> grad = nullptr);
  void trigger(bool note_on, uint8_t note = 0, uint8_t note_velocity = 0);

 protected:
  void paintEvent(QPaintEvent *event) override;

 private:
  std::shared_ptr<QImage> gradient;
  bool on;
  uint8_t velocity;
};

class xmidix_vu_array : public QFrame {
 Q_OBJECT
 public:
  explicit xmidix_vu_array(QWidget *parent = nullptr);
  void clear();

 public slots:
  void midi_event(const snd_seq_event_t &event);

 private:
  std::vector<std::shared_ptr<xmidix_vu>> vus;

};

#endif //XMIDIX__XMIDIX_VU_H_
