#ifndef XMIDIX__XMIDIX_PLAYLIST_H_
#define XMIDIX__XMIDIX_PLAYLIST_H_

#include <QObject>
#include <QAbstractListModel>
#include <QUrl>
#include <spdlog/spdlog.h>

using item_t = QUrl;
using list_t = QList<item_t>;

class xmidix_playlist : public QAbstractListModel {
 Q_OBJECT
 public:
  explicit xmidix_playlist(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &index) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  bool removeRows(int row, int count, const QModelIndex &parent) override;

  void append(const list_t &l);
  void replace(const list_t &l);
  void clear();
  void shuffle();
  void set_index(int index) { idx = index; }

  QModelIndex current_index() { return index(idx); }
  item_t current_item();
  item_t next_item();
  item_t previous_item();

  list_t items;

 private:
  int idx{0};
};

#endif //XMIDIX__XMIDIX_PLAYLIST_H_
