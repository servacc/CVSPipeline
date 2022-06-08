#pragma once

#include <QStyledItemDelegate>

class ComboItemDelegate : public QStyledItemDelegate {
 public:
  ComboItemDelegate(QObject *parent = nullptr);

  void setModel(QAbstractItemModel *, const QModelIndex &);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget                    *editor,
                            const QStyleOptionViewItem &option,
                            const QModelIndex          &index) const override;

 private:
  QAbstractItemModel *model = nullptr;
  QModelIndex         root_index;
};
