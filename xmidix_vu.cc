#include "xmidix_vu.h"

#include <QLayout>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFont>
#include <spdlog/spdlog.h>

xmidix_vu::xmidix_vu(QWidget *parent, std::shared_ptr<QImage> grad)
    : QWidget{parent}, on{false}, velocity{0}, gradient{grad} {
//  setFixedWidth(10);
//  setFixedHeight(10);
//  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setMinimumWidth(11);
  setMinimumHeight(16);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  setContentsMargins(0, 0, 0, 0);
}

void xmidix_vu::paintEvent(QPaintEvent *event) {
  QPainter painter{this};

  if (on) {
    painter.fillRect(rect(), gradient->pixelColor(velocity, 0));
  }

  QPainterPath path;
  path.addRect(rect());
  painter.strokePath(path, QPen(Qt::black, 2, Qt::SolidLine));

  QFont font{"Fixed", 10};
  font.setStyleStrategy(QFont::NoAntialias);
  painter.setFont(font);

  painter.drawText(rect(), Qt::AlignCenter, QString("%0").arg(velocity, 2, 16, QLatin1Char('0')).toUpper());
}

void xmidix_vu::trigger(bool note_on, uint8_t note, uint8_t note_velocity) {
  on = note_on;
  velocity = note_velocity;
  repaint();
}

xmidix_vu_array::xmidix_vu_array(QWidget *parent)
    : QFrame{parent} {
  setLayout(new QHBoxLayout(this));
  layout()->setContentsMargins(0, 6, 0, 0);
  layout()->setSpacing(1);
  layout()->setAlignment(Qt::AlignCenter);

  QLinearGradient grad(QPoint(0, 0), QPoint(127, 0));
  grad.setColorAt(1, Qt::red);
  grad.setColorAt(0.75, Qt::yellow);
  grad.setColorAt(0, Qt::green);

  auto img = std::make_shared<QImage>(128, 1, QImage::Format_RGB32);
  QPainter painter{img.get()};
  painter.fillRect(img->rect(), QBrush(grad));
  painter.end();

  for (int i = 0; i < 16; i++) {
    auto vu = std::make_shared<xmidix_vu>(this, img);
    layout()->addWidget(vu.get());
    vus.emplace_back(vu);
  }

  updateGeometry();
}

void xmidix_vu_array::midi_event(const snd_seq_event_t &event) {
  auto type = event.type;
  if (type == SND_SEQ_EVENT_NOTEOFF || type == SND_SEQ_EVENT_NOTEON || type == SND_SEQ_EVENT_KEYPRESS) {
    auto data = event.data.note;
    auto chan = data.channel;

    if (chan < 0 || chan >= 16) {
      spdlog::warn("got invalid channel = {}", chan);
      return;
    }

    bool on = type != SND_SEQ_EVENT_NOTEOFF;
    uint8_t velocity = type != SND_SEQ_EVENT_NOTEOFF ? data.velocity : 0;

    vus[data.channel]->trigger(on, data.note, velocity);
  }
}

void xmidix_vu_array::clear() {
  for (const auto &vu : vus) {
    vu->trigger(false);
  }
}
