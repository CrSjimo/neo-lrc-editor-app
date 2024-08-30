#include <QApplication>

#include <NeoLrcEditorApp/MainWindow.h>

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    a.setStyle("fusion");

    MainWindow win;

    win.show();

    return a.exec();
}