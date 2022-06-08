#pragma once

#include <QMainWindow>
#include <memory>

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();  // = default;

 private:
  class Private;
  std::unique_ptr<Private> m;
};
