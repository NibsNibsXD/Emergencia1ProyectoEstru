#include "resetpassworddialog.h"
#include "ui_resetpassworddialog.h"

#include <QMessageBox>

ResetPasswordDialog::ResetPasswordDialog(ChatCore *core, QWidget *parent)
    : QDialog(parent), ui(new Ui::ResetPasswordDialog), m_core(core)
{
    ui->setupUi(this);

    connect(ui->resetButton,&QPushButton::clicked,
            this,&ResetPasswordDialog::onReset);
}

ResetPasswordDialog::~ResetPasswordDialog(){ delete ui; }

void ResetPasswordDialog::onReset()
{
    std::string user   = ui->userEdit->text().toStdString();
    std::string ans    = ui->answerEdit->text().toStdString();
    std::string newP   = ui->newPassEdit->text().toStdString();
    std::string confirm= ui->confEdit->text().toStdString();

    if(newP!=confirm){
        QMessageBox::warning(this,"Error","La confirmación no coincide");
        return;
    }

    if(!m_core->resetPassword(user,ans,newP)){
        QMessageBox::warning(this,"Error",
                             "Usuario inexistente, respuesta incorrecta o contraseña < 6");
        return;
    }
    QMessageBox::information(this,"Contraseña",
                             "Se actualizó la contraseña");
    accept();
}
