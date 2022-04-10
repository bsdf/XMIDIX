#ifndef XMIDIX__XMIDIX_CONFIG_H_
#define XMIDIX__XMIDIX_CONFIG_H_

#include "ui_xmidix_config.h"
#include "xmidix_seq.h"

#include <memory>
#include <utility>
#include <QDialog>
#include <QObject>
#include <spdlog/spdlog.h>

class xmidix_config : public QDialog {
 Q_OBJECT
 public:
  explicit xmidix_config(QWidget *parent = nullptr);
  xmidix_client_info get_selected_client();
  void set_selected_client(int client_id, int port_id);

  void set_systray(bool val) {
    systray = val;
    ui.systray_check->setChecked(val);
  }

  void set_chanbar(bool val) {
    chanbar = val;
    ui.channel_status_check->setChecked(val);
  }

  Ui::config_window ui;

 signals:
  void config_updated();
  void clients_update();

 public slots:
  void clients_updated(const std::vector<xmidix_client_info>& c);
  void accepted_handler();
  void rejected_handler();

 protected:
  void showEvent(QShowEvent *event) override;

 private:
  void reload_settings();

  std::vector<xmidix_client_info> clients;

  QComboBox *output_device;
  int output_index = 0;
  bool systray{true};
  bool chanbar{true};
};

#endif //XMIDIX__XMIDIX_CONFIG_H_
