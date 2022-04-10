#include "xmidix_config.h"

xmidix_config::xmidix_config(QWidget *parent)
    : QDialog(parent), ui{} {
  ui.setupUi(this);
  output_device = ui.output_device;

  connect(ui.midi_refresh_button, &QPushButton::clicked,
          [this] { emit clients_update(); });
  connect(ui.buttonBox, &QDialogButtonBox::accepted,
          this, &xmidix_config::accepted_handler);
  connect(ui.buttonBox, &QDialogButtonBox::rejected,
          this, &xmidix_config::rejected_handler);
}

xmidix_client_info xmidix_config::get_selected_client() {
  return clients[output_index];
}

void xmidix_config::reload_settings() {
//  ui.systray_check->setChecked(settings.)
}

void xmidix_config::set_selected_client(int client_id, int port_id) {
  for (int i = 0; i < clients.size(); i++) {
    auto client = clients[i];
    if (client.client_id == client_id && client.port_id == port_id) {
      output_device->setCurrentIndex(i);
      return;
    }
  }
}

void xmidix_config::clients_updated(const std::vector<xmidix_client_info> &c) {
  clients = c;
  output_index = output_device->currentIndex();
  output_device->clear();

  for (const auto &client : clients) {
    auto txt = fmt::format("{}:{}\t{} - {}",
                           client.client_id, client.port_id,
                           client.client_name, client.port_name);
    output_device->addItem(QString::fromStdString(txt));
  }

  output_device->setCurrentIndex(output_index);
}

void xmidix_config::accepted_handler() {
  bool updated = false;
  if (output_device->currentIndex() != output_index) {
    output_index = output_device->currentIndex();
    updated = true;
  }

  if (ui.systray_check->isChecked() != systray) {
    updated = true;
    systray = ui.systray_check->isChecked();
  }

  if (ui.channel_status_check->isChecked() != chanbar) {
    updated = true;
    chanbar = ui.channel_status_check->isChecked();
  }

  if (updated) {
    emit config_updated();
  }
}

void xmidix_config::rejected_handler() {
  output_device->setCurrentIndex(output_index);
}

void xmidix_config::showEvent(QShowEvent *event) {
  reload_settings();
  QDialog::showEvent(event);
}
