#pragma once

#include <QIdentityProxyModel>

class HeaderProxyModel : public QIdentityProxyModel {
 public:
  explicit HeaderProxyModel(QObject *parent = nullptr);

  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

 private:
  using Data       = QMap<int, QVariant>;
  using HeaderData = QMap<int, Data>;

  QMap<Qt::Orientation, HeaderData> headers;
};
