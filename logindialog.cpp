#include "logindialog.h"
#include "ui_logindialog.h"

#include "registerdialog.h"
#include "resetpassworddialog.h"     // ← nuevo diálogo
#include <QMessageBox>

LoginDialog::LoginDialog(ChatCore *core, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_core(core)
{
    ui->setupUi(this);

    /* —— botón “Iniciar sesión” —— */
    connect(ui->loginButton, &QPushButton::clicked,
            this, &LoginDialog::accept);          // usa nuestro accept()

    /* —— botón “Registrar” —— */
    connect(ui->registerButton,&QPushButton::clicked,
            this,[this]{
                RegisterDialog d(m_core,this);
                d.exec();
            });

    /* —— botón “¿Olvidaste tu contraseña?” —— */
    connect(ui->forgotButton,&QPushButton::clicked,
            this,[this]{
                ResetPasswordDialog d(m_core,this);
                d.exec();                         // modal; vuelve al login
            });

    /* Enter en la línea de contraseña = intentar login */
    connect(ui->passLineEdit,&QLineEdit::returnPressed,
            this,&LoginDialog::accept);
}

LoginDialog::~LoginDialog(){ delete ui; }

/* ---------- validación ---------- */
bool LoginDialog::tryLogin()
{
    std::string u = ui->userLineEdit->text().toStdString();
    std::string p = ui->passLineEdit->text().toStdString();

    if(m_core->loginUser(u,p)) return true;

    QMessageBox::warning(this,"Error","Usuario o contraseña incorrectos");
    return false;
}

/* ---------- aceptar ---------- */
void LoginDialog::accept()
{
    if( tryLogin() )               // solo cierra si credenciales válidas
        QDialog::accept();
    /* de lo contrario permanece abierto */
}
