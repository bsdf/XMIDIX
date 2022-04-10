#include "xmidixed.h"

#include <spdlog/spdlog.h>
#include <string>

#include <QApplication>
#include <QFileDialog>

std::string type_to_name(snd_seq_event_type_t type) {
  switch (type) {
    // snd_seq_result_t
    case SND_SEQ_EVENT_SYSTEM:
      return "SYSTEM";
    case SND_SEQ_EVENT_RESULT:
      return "RESULT";

      // snd_seq_ev_note_t
    case SND_SEQ_EVENT_NOTE:
      return "NOTE";
    case SND_SEQ_EVENT_NOTEON:
      return "NOTEON";
    case SND_SEQ_EVENT_NOTEOFF:
      return "NOTEOFF";
    case SND_SEQ_EVENT_KEYPRESS:
      return "KEYPRESS";

      // snd_seq_ev_ctrl_t
    case SND_SEQ_EVENT_CONTROLLER:
      return "CONTROLLER";
    case SND_SEQ_EVENT_PGMCHANGE:
      return "PGMCHANGE";
    case SND_SEQ_EVENT_CHANPRESS:
      return "CHANPRESS";
    case SND_SEQ_EVENT_PITCHBEND:
      return "PITCHBEND";
    case SND_SEQ_EVENT_CONTROL14:
      return "CONTROL14";
    case SND_SEQ_EVENT_NONREGPARAM:
      return "NONREGPARAM";
    case SND_SEQ_EVENT_REGPARAM:
      return "REGPARAM";
    case SND_SEQ_EVENT_SONGPOS:
      return "SONGPOS";
    case SND_SEQ_EVENT_SONGSEL:
      return "SONGSEL";
    case SND_SEQ_EVENT_QFRAME:
      return "QFRAME";
    case SND_SEQ_EVENT_TIMESIGN:
      return "TIMESIGN";
    case SND_SEQ_EVENT_KEYSIGN:
      return "KEYSIGN";

      // snd_seq_ev_queue_control_t
    case SND_SEQ_EVENT_START:
      return "START";
    case SND_SEQ_EVENT_CONTINUE:
      return "CONTINUE";
    case SND_SEQ_EVENT_STOP:
      return "STOP";
    case SND_SEQ_EVENT_SETPOS_TICK:
      return "SETPOS_TICK";
    case SND_SEQ_EVENT_SETPOS_TIME:
      return "SETPOS_TIME";
    case SND_SEQ_EVENT_TEMPO:
      return "TEMPO";
    case SND_SEQ_EVENT_CLOCK:
      return "CLOCK";
    case SND_SEQ_EVENT_TICK:
      return "TICK";
    case SND_SEQ_EVENT_QUEUE_SKEW:
      return "QUEUE_SKEW";
    case SND_SEQ_EVENT_SYNC_POS:
      return "SYNC_POS";

      // event date type = none
    case SND_SEQ_EVENT_TUNE_REQUEST:
      return "TUNE_REQUEST";
    case SND_SEQ_EVENT_RESET:
      return "RESET";
    case SND_SEQ_EVENT_SENSING:
      return "SENSING";

      // event date type = any
    case SND_SEQ_EVENT_ECHO:
      return "ECHO";
    case SND_SEQ_EVENT_OSS:
      return "OSS";

      // snd_seq_addr_t
    case SND_SEQ_EVENT_CLIENT_START:
      return "CLIENT_START";
    case SND_SEQ_EVENT_CLIENT_EXIT:
      return "CLIENT_EXIT";
    case SND_SEQ_EVENT_CLIENT_CHANGE:
      return "CLIENT_CHANGE";
    case SND_SEQ_EVENT_PORT_START:
      return "PORT_START";
    case SND_SEQ_EVENT_PORT_EXIT:
      return "PORT_EXIT";
    case SND_SEQ_EVENT_PORT_CHANGE:
      return "PORT_CHANGE";

      // snd_seq_connect_t
    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
      return "PORT_SUBSCRIBED";
    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
      return "PORT_UNSUBSCRIBED";

      // event type = any (fixed)
    case SND_SEQ_EVENT_USR0:
      return "USR0";
    case SND_SEQ_EVENT_USR1:
      return "USR1";
    case SND_SEQ_EVENT_USR2:
      return "USR2";
    case SND_SEQ_EVENT_USR3:
      return "USR3";
    case SND_SEQ_EVENT_USR4:
      return "USR4";
    case SND_SEQ_EVENT_USR5:
      return "USR5";
    case SND_SEQ_EVENT_USR6:
      return "USR6";
    case SND_SEQ_EVENT_USR7:
      return "USR7";
    case SND_SEQ_EVENT_USR8:
      return "USR8";
    case SND_SEQ_EVENT_USR9:
      return "USR9";

      // snd_seq_ev_ext_t
    case SND_SEQ_EVENT_SYSEX:
      return "SYSEX";
    case SND_SEQ_EVENT_BOUNCE:
      return "BOUNCE";
    case SND_SEQ_EVENT_USR_VAR0:
      return "USR_VAR0";
    case SND_SEQ_EVENT_USR_VAR1:
      return "USR_VAR1";
    case SND_SEQ_EVENT_USR_VAR2:
      return "USR_VAR2";
    case SND_SEQ_EVENT_USR_VAR3:
      return "USR_VAR3";
    case SND_SEQ_EVENT_USR_VAR4:
      return "USR_VAR4";

      // NOP
    case SND_SEQ_EVENT_NONE:
    default:
      return "NONE";
  }
}

QStandardItem *get_channel_item(const snd_seq_event_t &event) {
  int channel;
  switch (event.type) {
    case SND_SEQ_EVENT_NOTE:
    case SND_SEQ_EVENT_NOTEON:
    case SND_SEQ_EVENT_NOTEOFF:
    case SND_SEQ_EVENT_KEYPRESS: {
      auto data = event.data.note;
      channel = data.channel;
      break;
    }

    case SND_SEQ_EVENT_CONTROLLER:
    case SND_SEQ_EVENT_PGMCHANGE:
    case SND_SEQ_EVENT_CHANPRESS:
    case SND_SEQ_EVENT_PITCHBEND:
    case SND_SEQ_EVENT_CONTROL14:
    case SND_SEQ_EVENT_NONREGPARAM:
    case SND_SEQ_EVENT_REGPARAM:
    case SND_SEQ_EVENT_SONGPOS:
    case SND_SEQ_EVENT_SONGSEL:
    case SND_SEQ_EVENT_QFRAME:
    case SND_SEQ_EVENT_TIMESIGN:
    case SND_SEQ_EVENT_KEYSIGN: {
      auto data = event.data.control;
      channel = data.channel;
      break;
    }

    default:
      return nullptr;
  }
  return new QStandardItem(QString("%0").arg(channel));
}

QStandardItem *get_data_item(const snd_seq_event_t &event) {
  switch (event.type) {
    case SND_SEQ_EVENT_NOTE:
    case SND_SEQ_EVENT_NOTEON:
    case SND_SEQ_EVENT_NOTEOFF:
    case SND_SEQ_EVENT_KEYPRESS: {
      auto data = event.data.note;
      return new QStandardItem(
          QString("note: %1 velocity: %2")
              .arg(data.note, 4)
              .arg(data.velocity));
    }

    case SND_SEQ_EVENT_CONTROLLER:
    case SND_SEQ_EVENT_PGMCHANGE:
    case SND_SEQ_EVENT_CHANPRESS:
    case SND_SEQ_EVENT_PITCHBEND:
    case SND_SEQ_EVENT_CONTROL14:
    case SND_SEQ_EVENT_NONREGPARAM:
    case SND_SEQ_EVENT_REGPARAM:
    case SND_SEQ_EVENT_SONGPOS:
    case SND_SEQ_EVENT_SONGSEL:
    case SND_SEQ_EVENT_QFRAME:
    case SND_SEQ_EVENT_TIMESIGN:
    case SND_SEQ_EVENT_KEYSIGN: {
      auto data = event.data.control;
      return new QStandardItem(
          QString("param: %1 value: %2")
              .arg(data.param, 3)
              .arg(data.value));
    }

    case SND_SEQ_EVENT_SYSEX:
    case SND_SEQ_EVENT_BOUNCE:
    case SND_SEQ_EVENT_USR_VAR0:
    case SND_SEQ_EVENT_USR_VAR1:
    case SND_SEQ_EVENT_USR_VAR2:
    case SND_SEQ_EVENT_USR_VAR3:
    case SND_SEQ_EVENT_USR_VAR4: {
      auto data = event.data.ext;
      auto len = data.len;
      auto p = static_cast<uint8_t *>(data.ptr);

      QString str("");
      for (int i = 0; i < len; i++) {
        str += QString::asprintf("%02X ", p[i]);
      }
      return new QStandardItem(str);
    }

//    case SND_SEQ_EVENT_RESET: {
//      return nullptr;
//    }

    default:
      return nullptr;
  }
}

QList<QStandardItem *> event_to_row(const snd_seq_event_t &event) {
  auto tick = new QStandardItem(QString("%0").arg(event.time.tick));
  auto type = new QStandardItem(type_to_name(event.type).c_str());

  return {tick, type, get_channel_item(event), get_data_item(event)};
}

xmidixed::xmidixed(QWidget *parent)
    : QMainWindow(parent),
      ui{std::make_unique<Ui::MainWindow>()},
      model{std::make_unique<QStandardItemModel>()} {
  setAttribute(Qt::WA_X11NetWmWindowTypeUtility);
  ui->setupUi(this);

  tree = ui->treeView;
  tree->setSortingEnabled(false);

  open_action = std::make_unique<QAction>(this);
  open_action->setShortcut(QKeySequence("Ctrl+o"));
  addAction(open_action.get());

  jump_action = std::make_unique<QAction>(this);
  jump_action->setShortcut(QKeySequence("Ctrl+j"));
  addAction(jump_action.get());

  filter_action = std::make_unique<QAction>(this);
  filter_action->setShortcut(QKeySequence("Ctrl+f"));
  addAction(filter_action.get());

  connect(ui->open_button, &QPushButton::clicked,
          open_action.get(), &QAction::trigger);
  connect(ui->jump_button, &QPushButton::clicked,
          jump_action.get(), &QAction::trigger);
  connect(ui->filter_button, &QPushButton::clicked,
          filter_action.get(), &QAction::trigger);

  connect(open_action.get(), &QAction::triggered,
          [&] {
            auto file = QFileDialog::getOpenFileName(this, "OPEN MIDI FILE", "", "MIDI FILES (*.mid *.midi)");
            xmidix_file midi(file.toStdString());
            load(midi);
          });
  connect(jump_action.get(), &QAction::triggered,
          [] { spdlog::info("jump clicked"); });
  connect(filter_action.get(), &QAction::triggered,
          [] { spdlog::info("filter clicked"); });
}

void xmidixed::load(const xmidix_file &file) {
  model->clear();
  model->setHorizontalHeaderLabels({"ＴＩＣＫ", "ＭＳＧ", "ＣＨ", "ＤＡＴＡ"});

  for (const auto &event : file.get_events()) {
    model->appendRow(event_to_row(event));
  }

  tree->setModel(model.get());
}

int main(int argc, char **argv) {
  QApplication a(argc, argv);
//  xmidix_file f("/home/david/code/xmidi/test/test3.mid");
  xmidix_file f("/home/david/Desktop/FALCOM_MIDI/FALCOM_MIDI_ARCHIVE/e4g03p.mid");
  xmidixed window;

  window.load(f);
  window.show();

  return a.exec();
}