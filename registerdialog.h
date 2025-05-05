#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QString>
#include "chatcore.h"

namespace Ui { class RegisterDialog; }

class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(ChatCore* core, QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void onBrowseAvatar();   // abre el explorador
    void tryRegister();      // valida y guarda

private:
    Ui::RegisterDialog *ui;
    ChatCore* m_core{};
    QString   m_avatarPath;  // ruta elegida
};

#endif
