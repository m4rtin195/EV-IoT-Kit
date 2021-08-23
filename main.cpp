#include <iostream>
#include <QApplication>
#include "window.h"
#include "logger.h"

using namespace std;

int main(int argc, char *argv[])
{
    //cout << "first." << endl;

    QApplication app(argc, argv);
    //cout << "app inst created." << endl;

    //cout << "going to create Window." << endl;
    Window w;
    //cout << "window created." << endl;

#if defined (_WIN32) || defined (_WIN64)
    w.show(); //w.showFullScreen();
#endif
#if defined (__linux__)
    w.show();
    //w.showFullScreen();
#endif
    //cout << "window showed." << endl;

    qInstallMessageHandler(Window::QMessageOutput);

    //cout << "app.exec()" << endl;
    return app.exec();
}
