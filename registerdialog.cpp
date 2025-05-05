#include "registerdialog.h"
#include "ui_registerdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

RegisterDialog::RegisterDialog(ChatCore* core, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , m_core(core)
{
    ui->setupUi(this);

    /* botón OK del buttonBox */
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this,          &RegisterDialog::tryRegister);

    /* botón “Elegir foto…” */
    connect(ui->browseAvatarButton, &QPushButton::clicked,
            this,                   &RegisterDialog::onBrowseAvatar);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

/* ---------- seleccionar imagen ---------- */
void RegisterDialog::onBrowseAvatar()
{
    QString fn = QFileDialog::getOpenFileName(
        this,
        "Selecciona imagen de avatar",
        QString(),
        "Imágenes (*.png *.jpg *.jpeg *.bmp)");

    if(fn.isEmpty()) return;

    m_avatarPath = fn;
    /* miniatura (opcional) */
    ui->avatarPreview->setPixmap(
        QPixmap(fn).scaled(64,64,Qt::KeepAspectRatio,Qt::SmoothTransformation));
}

/* ---------- registro ---------- */
void RegisterDialog::tryRegister()
{
    if(ui->passLineEdit->text() != ui->confirmLineEdit->text()){
        QMessageBox::warning(this,"Error","Las contraseñas no coinciden");
        return;
    }

    User u;
    u.username = ui->userLineEdit->text().toStdString();
    u.fullname = ui->nameLineEdit->text().toStdString();
    u.email    = ui->emailLineEdit->text().toStdString();
    u.password = ui->passLineEdit->text().toStdString();
    u.age      = ui->ageSpin->value();
    u.avatar   = m_avatarPath.toStdString();                 // ← ruta elegida
    u.secQ     = ui->secQLineEdit->text().toStdString();
    u.secA     = ui->secALineEdit->text().toStdString();

    if(m_core->userExists(u.username)){
        QMessageBox::warning(this,"Error","Ese usuario ya existe");
        return;
    }
    if(!isValidEmail(u.email)){
        QMessageBox::warning(this,"Error","Correo electrónico inválido");
        return;
    }

    m_core->saveUser(u);
    QMessageBox::information(this,"Registro","✔ Registro exitoso");
    accept();                              // cerrar diálogo
}
