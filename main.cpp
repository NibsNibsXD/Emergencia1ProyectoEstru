#include "mainwindow.h"
#include "logindialog.h"
#include "chatcore.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatCore core;

    LoginDialog dlg(&core);
    if(dlg.exec() != QDialog::Accepted) return 0;

    MainWindow w(&core);
    w.show();
    return a.exec();
}
