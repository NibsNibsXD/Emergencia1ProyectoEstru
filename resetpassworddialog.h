#ifndef RESETPASSWORDDIALOG_H
#define RESETPASSWORDDIALOG_H

#include <QDialog>
#include "chatcore.h"

namespace Ui { class ResetPasswordDialog; }

class ResetPasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ResetPasswordDialog(ChatCore* core,
                                 QWidget *parent = nullptr);
    ~ResetPasswordDialog();

private slots:
    void onReset();

private:
    Ui::ResetPasswordDialog *ui;
    ChatCore* m_core;
};

#endif
