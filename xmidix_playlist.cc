#include "xmidix_playlist.h"

#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <random>

xmidix_playlist::xmidix_playlist(QObject *parent) : QAbstractListModel(parent) {
}

int xmidix_playlist::rowCount(const QModelIndex &index) const {
  return items.size();
}

QVariant xmidix_playlist::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};

  if (role == Qt::DisplayRole) {
    auto url = items[index.row()];
    QFileInfo info(url.toLocalFile());
    return info.dir().dirName() + "/" + info.fileName();
  }

  return {};
}

void xmidix_playlist::append(const list_t &l) {
  beginResetModel();
  items.append(l);
  endResetModel();
}

void xmidix_playlist::replace(const list_t &l) {
  beginResetModel();
  items.clear();
  items.append(l);
  endResetModel();
}

void xmidix_playlist::clear() {
  beginResetModel();
  items.clear();
  endResetModel();
}

item_t xmidix_playlist::current_item() {
  if (items.empty()) return {};

  return items[idx];
}

item_t xmidix_playlist::next_item() {
  if (items.empty()) return {};

  idx = idx == items.size() - 1 ? 0 : idx + 1;
  return items[idx];
}

item_t xmidix_playlist::previous_item() {
  if (items.empty()) return {};

  idx = idx == 0 ? items.size() - 1 : idx - 1;
  return items[idx];
}

void xmidix_playlist::shuffle() {
  beginResetModel();
  std::random_device rd;
  std::mt19937 g(rd());

  std::shuffle(items.begin(), items.end(), g);
  endResetModel();
}

bool xmidix_playlist::removeRows(int row, int count, const QModelIndex &parent) {
  beginRemoveRows(parent, row, row + count);
  for (int i = 0; i < count; i ++) {
    items.removeAt(row + i);
  }
  endRemoveRows();
  return true;
}
