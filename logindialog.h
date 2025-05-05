#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "chatcore.h"

namespace Ui { class LoginDialog; }

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(ChatCore* core, QWidget* parent = nullptr);
    ~LoginDialog();

protected:
    void accept() override;           // ← sobrescribimos

private:
    bool tryLogin();                  // devuelve true/false

    Ui::LoginDialog *ui;
    ChatCore* m_core;
};

#endif
