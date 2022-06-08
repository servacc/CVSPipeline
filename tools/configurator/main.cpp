#include "mainwindow.hpp"

#include <cvs/logger/logging.hpp>

#include <QApplication>

int main(int argc, char *argv[]) {
  cvs::logger::initLoggers();

  QApplication a(argc, argv);
  MainWindow   w;
  w.show();
  return a.exec();
}
