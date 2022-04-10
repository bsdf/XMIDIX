#ifndef ARP__XMIDI_PLAYER_H_
#define ARP__XMIDI_PLAYER_H_

#include "ui_xmidix.h"
#include "xmidix_vu.h"
#include "xmidix_seq.h"
#include "xmidix_file.h"
#include "xmidix_piano.h"
#include "xmidix_config.h"
#include "xmidix_playlist.h"

#include <QUrl>
#include <QList>
#include <QTimer>
#include <QObject>
#include <QAction>
#include <QSettings>
#include <QMainWindow>
#include <QSystemTrayIcon>

#define WINDOW_TITLE    "ＸＭＩＤＩＸ"
#define SETTINGS_CLIENT "client"
#define SETTINGS_PORT   "port"
#define SETTINGS_CWD    "cwd"
#define SETTINGS_SYSTRAY "systray"
#define SETTINGS_CHANBAR "chanbar"

enum class play_status {
  STOPPED, PLAYING, PAUSED
};

class xmidix_player : public QMainWindow {
 Q_OBJECT

 public:
  explicit xmidix_player(QWidget *parent = nullptr, QList<QUrl> files = {});
  ~xmidix_player() override;

 public slots:
  void play(const QUrl &url);

  void config_client_update();
  void config_updated();

  void playlist_activated(const QModelIndex &index);
  void load_button_click();
  void play_button_click();
  void stop_button_click();
  void config_button_click();

  void status_updated(play_status status);
  void song_complete();
  void tick_update();
  void slider_seek();

 signals:
  void song_change(const QUrl &url);
  void clients_updated(const std::vector<xmidix_client_info> &c);
  void status_update(play_status s);

 protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

 private:
  void load_settings();
  void setup_connections();
  void setup_actions();
  void setup_action(QAction *a, const QKeySequence &s);

  play_status status;
  unsigned int last_tick;

  Ui::MainWindow ui;
  xmidix_config config_window;
  xmidix_seq seq;
  xmidix_playlist model;
  xmidix_piano_array pianos;

  QTimer progress_timer;
  QSettings settings;
  QSystemTrayIcon tray_icon;

  QAction play_action;
  QAction load_action;
  QAction stop_action;
  QAction conf_action;
  QAction back_action;
  QAction forw_action;
  QAction shuf_action;
};

#endif //ARP__XMIDI_PLAYER_H_
