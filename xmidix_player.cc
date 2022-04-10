#include "xmidix_player.h"

#include <spdlog/spdlog.h>
#include <QFileDialog>
#include <QSlider>
#include <QEvent>
#include <QKeyEvent>

xmidix_player::xmidix_player(QWidget *parent, QList<QUrl> files)
    : QMainWindow(parent),
      ui{},
      seq{},
      config_window{this},
      model{this},
      progress_timer{this},
      tray_icon{QIcon(":/icons/MIDI_LOGO.svg"), this},
      settings{this},
      status{play_status::STOPPED},
      last_tick{0} {

  setAttribute(Qt::WA_X11NetWmWindowTypeUtility);
  setWindowTitle(WINDOW_TITLE);
  tray_icon.show();

  model.replace(files);

  ui.setupUi(this);
  ui.listView->setModel(&model);
  ui.listView->installEventFilter(this);

  setup_actions();
  setup_connections();
  config_client_update();

  load_settings();
}

void xmidix_player::load_settings() {
  auto client = settings.value(SETTINGS_CLIENT);
  auto port = settings.value(SETTINGS_PORT);

  auto client_id = client.isNull() ? 14 : client.toInt();
  auto port_id = port.isNull() ? 0 : port.toInt();

  seq.connect(client_id, port_id);
  config_window.set_selected_client(client_id, port_id);

  auto tray = settings.value(SETTINGS_SYSTRAY);
  auto tray_visible = tray.isNull() || tray.toBool();
  tray_icon.setVisible(tray_visible);
  config_window.set_systray(tray_visible);

  auto chanbar = settings.value(SETTINGS_CHANBAR);
  auto chanbar_visible = chanbar.isNull() || chanbar.toBool();
  ui.vus->setVisible(chanbar_visible);
  config_window.set_chanbar(chanbar_visible);
}

void xmidix_player::setup_action(QAction *a, const QKeySequence &s) {
  a->setShortcut(s);
  addAction(a);
}

// to satisfy c++20 pedantry
inline constexpr int operator|(Qt::Modifier m, Qt::Key k) { return (int) m | (int) k; }
void xmidix_player::setup_actions() {
  setup_action(&play_action, Qt::Key_Space);
  setup_action(&load_action, Qt::CTRL | Qt::Key_O);
  setup_action(&stop_action, Qt::CTRL | Qt::Key_Space);
  setup_action(&conf_action, Qt::CTRL | Qt::Key_P);
  setup_action(&back_action, Qt::CTRL | Qt::Key_Left);
  setup_action(&forw_action, Qt::CTRL | Qt::Key_Right);
  setup_action(&shuf_action, {});
}

void xmidix_player::setup_connections() {
  connect(&seq, &xmidix_seq::song_complete,
          this, &xmidix_player::song_complete);
  connect(&seq, &xmidix_seq::song_looping,
          [] { spdlog::info("song looping."); });

  connect(&progress_timer, &QTimer::timeout,
          this, &xmidix_player::tick_update);

  connect(this, &xmidix_player::status_update,
          this, &xmidix_player::status_updated);
  connect(this, &xmidix_player::clients_updated,
          &config_window, &xmidix_config::clients_updated);
  connect(this, &xmidix_player::song_change,
          this, &xmidix_player::play);

  connect(&config_window, &xmidix_config::clients_update,
          this, &xmidix_player::config_client_update);
  connect(&config_window, &xmidix_config::config_updated,
          this, &xmidix_player::config_updated);

  connect(ui.listView, &QListView::activated,
          this, &xmidix_player::playlist_activated);
  connect(ui.slider, &QSlider::sliderReleased,
          this, &xmidix_player::slider_seek);

  connect(ui.play_button, &QPushButton::clicked,
          &play_action, &QAction::trigger);
  connect(ui.load_button, &QPushButton::clicked,
          &load_action, &QAction::trigger);
  connect(ui.stop_button, &QPushButton::clicked,
          &stop_action, &QAction::trigger);
  connect(ui.config_button, &QPushButton::clicked,
          &conf_action, &QAction::trigger);
  connect(ui.backward_button, &QPushButton::clicked,
          &back_action, &QAction::trigger);
  connect(ui.forward_button, &QPushButton::clicked,
          &forw_action, &QAction::trigger);
  connect(ui.shuffle_button, &QPushButton::clicked,
          &shuf_action, &QAction::trigger);

  connect(&play_action, &QAction::triggered,
          this, &xmidix_player::play_button_click);
  connect(&load_action, &QAction::triggered,
          this, &xmidix_player::load_button_click);
  connect(&stop_action, &QAction::triggered,
          this, &xmidix_player::stop_button_click);
  connect(&conf_action, &QAction::triggered,
          this, &xmidix_player::config_button_click);
  connect(&back_action, &QAction::triggered,
          [&] { emit song_change(model.previous_item()); });
  connect(&forw_action, &QAction::triggered,
          [&] { emit song_change(model.next_item()); });
  connect(&shuf_action, &QAction::triggered,
          [&] { model.shuffle(); });

  connect(&tray_icon, &QSystemTrayIcon::activated,
          [&] { this->setVisible(!this->isVisible()); });
  connect(ui.piano_button, &QPushButton::clicked,
          [&] { pianos.setVisible(!pianos.isVisible()); });

  connect(&seq, &xmidix_seq::midi_event,
          ui.vus, &xmidix_vu_array::midi_event);
  connect(&seq, &xmidix_seq::midi_event,
          &pianos, &xmidix_piano_array::midi_event);
}

xmidix_player::~xmidix_player() {

}

void xmidix_player::play_button_click() {
  switch (status) {
    case play_status::PLAYING:
      emit status_update(play_status::PAUSED);
      seq.stop();
      break;
    case play_status::PAUSED:
      emit status_update(play_status::PLAYING);
      seq.unpause();
      break;
    case play_status::STOPPED:
      emit song_change(model.current_item());
      break;
  }
}

void xmidix_player::stop_button_click() {
  emit status_update(play_status::STOPPED);
  seq.stop();
}

void xmidix_player::load_button_click() {
  auto cwd = settings.value(SETTINGS_CWD).toString();
  auto files = QFileDialog::getOpenFileUrls(
      this, "OPEN MIDI FILES",
      QUrl::fromLocalFile(cwd),
      "MIDI FILES (*.mid *.midi)");

  if (!files.empty()) {
    auto f = QFileInfo(files[0].toLocalFile());
    settings.setValue(SETTINGS_CWD, f.dir().canonicalPath());

    model.replace(files);
  }
}

void xmidix_player::playlist_activated(const QModelIndex &index) {
  model.set_index(index.row());
  emit song_change(model.current_item());
}

void xmidix_player::play(const QUrl &url) {
  auto path = url.path().toStdString();

  xmidix_file midi(path);
  if (midi.events.empty()) {
    spdlog::warn("bad midi file = {}", path);
    return;
  }

  auto events = midi.events;
  auto last_event = events[events.size() - 1];
  last_tick = last_event.time.tick;
  spdlog::info("last tick of song = {}", last_tick);

  spdlog::info("playing = {}", url.path().toStdString());
  emit status_update(play_status::PLAYING);
  seq.load(midi);
  seq.play();

  setWindowTitle(
      fmt::format("{} - {}", WINDOW_TITLE, url.fileName().toStdString()).c_str());

  auto idx = model.current_index();
  ui.listView->clearSelection();
  ui.listView->setCurrentIndex(idx);
  ui.listView->scrollTo(idx);
}

void xmidix_player::config_button_click() {
  config_window.show();
}

void xmidix_player::config_updated() {
  auto client = config_window.get_selected_client();
  settings.setValue(SETTINGS_CLIENT, client.client_id);
  settings.setValue(SETTINGS_PORT, client.port_id);
  seq.connect(client.client_id, client.port_id);

  auto tray = config_window.ui.systray_check->isChecked();
  settings.setValue(SETTINGS_SYSTRAY, tray);
  tray_icon.setVisible(tray);

  auto chanbar = config_window.ui.channel_status_check->isChecked();
  settings.setValue(SETTINGS_CHANBAR, chanbar);
  ui.vus->setVisible(chanbar);
}

void xmidix_player::config_client_update() {
  emit clients_updated(seq.enumerate_clients());
}

void xmidix_player::status_updated(play_status s) {
  ui.vus->clear();
  pianos.clear();

  status = s;
  switch (status) {
    case play_status::STOPPED:
      setWindowTitle(WINDOW_TITLE);
      ui.play_button->setIcon(QIcon::fromTheme("media-playback-start"));
      ui.slider->setValue(0);
      ui.slider->setEnabled(false);
      progress_timer.stop();
      break;
    case play_status::PLAYING:
      ui.play_button->setIcon(QIcon::fromTheme("media-playback-pause"));
      ui.slider->setEnabled(true);
      progress_timer.start(std::chrono::seconds{1});
      break;
    case play_status::PAUSED:
      ui.play_button->setIcon(QIcon::fromTheme("media-playback-start"));
      progress_timer.stop();
      break;
  }
}

void xmidix_player::song_complete() {
  if (status == play_status::PLAYING) {
    spdlog::debug("song complete.");
    emit song_change(model.next_item());
    ui.vus->clear();
  }
}

void xmidix_player::tick_update() {
  if (!ui.slider->isSliderDown()) {
    auto tick = seq.get_tick();
    auto percent = (static_cast<double>(tick) / last_tick) * 100;
    ui.slider->setValue(static_cast<int>(percent));
  }
}

void xmidix_player::slider_seek() {
  auto pos = ui.slider->sliderPosition();
  auto tick = (static_cast<double>(pos) / 100) * last_tick;
  seq.seek(static_cast<unsigned int>(tick));
}

bool xmidix_player::eventFilter(QObject *watched, QEvent *event) {
//  if (event->type() == QEvent::KeyRelease) {
//    auto e = reinterpret_cast<QKeyEvent *>(event);
//    if (e->key() == Qt::Key_Delete) {
//      auto indices = ui->listView->selectionModel()->selectedIndexes();
//      for (auto idx : indices) {
//        model->removeRow(idx.row());
//      }
//    }
//  }
  return QObject::eventFilter(watched, event);
}
