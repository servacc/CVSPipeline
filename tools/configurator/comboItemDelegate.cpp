#include "comboItemDelegate.hpp"

#include <QComboBox>

ComboItemDelegate::ComboItemDelegate(QObject *parent) {}

QWidget *ComboItemDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem & /* option */,
                                         const QModelIndex & /* index */) const {
  QComboBox *editor = new QComboBox(parent);
  editor->setModel(model);
  editor->setRootModelIndex(root_index);

  return editor;
}

void ComboItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  auto value = index.model()->data(index, Qt::UserRole).toInt();

  QComboBox *combo_box = static_cast<QComboBox *>(editor);
  combo_box->setCurrentIndex(value);
}

void ComboItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  QComboBox *combo_box = static_cast<QComboBox *>(editor);
  model->setData(index, combo_box->currentIndex(), Qt::UserRole);
  model->setData(index, combo_box->currentText(), Qt::DisplayRole);
}

void ComboItemDelegate::updateEditorGeometry(QWidget                    *editor,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex & /* index */) const {
  editor->setGeometry(option.rect);
}

void ComboItemDelegate::setModel(QAbstractItemModel *m, const QModelIndex &r) {
  model      = m;
  root_index = r;
}
