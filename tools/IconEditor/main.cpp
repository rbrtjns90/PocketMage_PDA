#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("PocketMage Icon Editor");
    app.setApplicationVersion("1.0");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
