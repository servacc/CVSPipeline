#include "headerProxyModel.hpp"

HeaderProxyModel::HeaderProxyModel(QObject *parent)
    : QIdentityProxyModel{parent} {}

bool HeaderProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) {
  headers[orientation][section][role] = value;
  emit headerDataChanged(orientation, section, section);
  return true;
}

QVariant HeaderProxyModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (!headers.contains(orientation))
    return QIdentityProxyModel::headerData(section, orientation, role);

  if (headers[orientation].contains(section) && headers[orientation][section].contains(role))
    return headers[orientation][section][role];

  return QVariant{};
}
