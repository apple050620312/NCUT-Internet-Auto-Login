#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QApplication::setOrganizationName("NCUT");
  QApplication::setApplicationName("NCUTAutoLoginQt");
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  MainWindow w; w.show();
  return app.exec();
}

