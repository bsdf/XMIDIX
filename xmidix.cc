#include "xmidix_player.h"

#include <spdlog/spdlog.h>
#include <QApplication>
#include <QUrl>
#include <QList>
#include <QFileInfo>

typedef struct {
  QList<QUrl> files;
} args_t;

args_t parse_args(int argc, char **argv) {
  QList<QUrl> files;

  for (int i = 1; i < argc; i++) {
    auto arg = argv[i];
    if (QFileInfo::exists(arg)) {
      auto url = QUrl::fromLocalFile(arg);
      files.append(url);
    } else {
      spdlog::warn("file does not exist: {}", arg);
    }
  }

  return {files};
}

int main(int argc, char **argv) {
  spdlog::set_level(spdlog::level::debug);
  QApplication a(argc, argv);
  QCoreApplication::setOrganizationName("xeyes.org");
  QCoreApplication::setOrganizationDomain("xeyes.org");
  QCoreApplication::setApplicationName("ＸＭＩＤＩＸ");

  auto args = parse_args(argc, argv);

  xmidix_player w{nullptr, args.files};
  w.show();

  bool result = a.exec();
  return result;
}
