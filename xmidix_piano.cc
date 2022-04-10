#include "xmidix_piano.h"

#include <spdlog/spdlog.h>
#include <map>
#include <QPainter>
#include <QVBoxLayout>

#define TRIGGER_COLOR Qt::red
#define WHITE_COLOR   Qt::white
#define BLACK_COLOR   Qt::black
#define OUTLINE_COLOR Qt::black

bool is_white_key(int idx) {
  switch (idx % 12) {
    case 1:
    case 3:
    case 6:
    case 8:
    case 10:
      return false;
    default:
      return true;
  }
}

xmidix_piano::xmidix_piano(QWidget *parent, std::shared_ptr<QImage> grad)
    : QWidget{parent}, gradient{grad} {
  setMinimumHeight(30);
  setMinimumWidth(128 * 6);
//  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void xmidix_piano::paintEvent(QPaintEvent *event) {
  QPainter painter{this};
  painter.setPen(OUTLINE_COLOR);

  double white_width = (width() - 1) / 75.0;
  double black_width = white_width * 0.60;
  double black_adjust = black_width / 2.0;
  double x = 0;

  std::map<uint8_t, double> black_offsets;

  // draw white keys
  for (int i = 0; i < 128; i++) {
    auto velocity = key_state[i];
    auto color = velocity == 0 ? WHITE_COLOR : gradient->pixelColor(velocity, 0);

    if (is_white_key(i)) {
      QRectF r(x, 0, white_width, height() - 1);
//      painter.fillRect(r, on ? TRIGGER_COLOR : WHITE_COLOR);
      painter.fillRect(r, color);
      painter.drawRect(r);
      x += white_width;
    } else {
      // save x position for next loop
      black_offsets[i] = x - black_adjust;
    }
  }

  // draw black keys using offsets
  for (const auto&[i, offset] : black_offsets) {
    auto velocity = key_state[i];
    auto color = velocity == 0 ? BLACK_COLOR : gradient->pixelColor(velocity, 0);

//    QRectF r(offset, 0, black_width, height() / 2.0);
    QRectF r(offset, 0, black_width, height() * 0.6);
//    painter.fillRect(r, on ? TRIGGER_COLOR : BLACK_COLOR);
    painter.fillRect(r, color);
    painter.drawRect(r);
  }
}

xmidix_piano_array::xmidix_piano_array(QWidget *parent)
    : QDialog{parent} {
  setAttribute(Qt::WA_X11NetWmWindowTypeUtility);
  setWindowTitle("ＸＭＩＤＩＸＰＩＡＮＯＸ");
//  setFixedHeight(518);
//  setFixedWidth(600);
//  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  setLayout(new QVBoxLayout(this));

  QLinearGradient grad(QPoint(0, 0), QPoint(127, 0));
  grad.setColorAt(1, Qt::red);
  grad.setColorAt(0.75, Qt::yellow);
  grad.setColorAt(0, Qt::green);

  auto img = std::make_shared<QImage>(128, 1, QImage::Format_RGB32);
  QPainter painter{img.get()};
  painter.fillRect(img->rect(), QBrush(grad));
  painter.end();

  for (int i = 0; i < 16; i++) {
    auto piano = std::make_shared<xmidix_piano>(this, img);
    layout()->addWidget(piano.get());
    pianos.emplace_back(piano);
  }

  updateGeometry();
}

void xmidix_piano_array::midi_event(const snd_seq_event_t &event) {
  auto type = event.type;
  if (type == SND_SEQ_EVENT_NOTEOFF || type == SND_SEQ_EVENT_NOTEON || type == SND_SEQ_EVENT_KEYPRESS) {
    auto data = event.data.note;
    auto chan = data.channel;

    if (chan < 0 || chan >= 16) {
      spdlog::warn("got invalid channel = {}", chan);
      return;
    }

    uint8_t velocity = type != SND_SEQ_EVENT_NOTEOFF ? data.velocity : 0;
    pianos[data.channel]->set_key(data.note, velocity);
  }
}

void xmidix_piano_array::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  position = pos();
}

void xmidix_piano_array::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  move(position);
}
