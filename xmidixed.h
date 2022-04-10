#ifndef XMIDIX__XMIDIXED_H_
#define XMIDIX__XMIDIXED_H_

#include "ui_xmidixed.h"
#include "xmidix_file.h"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QAction>

#include <alsa/asoundlib.h>
#include <memory>
#include <vector>

class xmidixed : public QMainWindow {
 public:
  explicit xmidixed(QWidget *parent = nullptr);
  void load(const xmidix_file &file);
 private:
  std::unique_ptr<Ui::MainWindow> ui;
  std::unique_ptr<QStandardItemModel> model;
  std::unique_ptr<QAction> open_action;
  std::unique_ptr<QAction> jump_action;
  std::unique_ptr<QAction> filter_action;
  QTreeView *tree;
};

#endif //XMIDIX__XMIDIXED_H_
