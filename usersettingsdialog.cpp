#include "usersettingsdialog.h"
#include "ui_usersettingsdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

UserSettingsDialog::UserSettingsDialog(ChatCore *core, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserSettingsDialog)
    , m_core(core)
{
    ui->setupUi(this);

    /* ------- avatar actual ------- */
    QString av = QString::fromStdString(m_core->currentUser().avatar);
    if(!av.isEmpty())
        ui->avatarLabel->setPixmap(QPixmap(av).scaled(
            ui->avatarLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));

    /* ------- conexiones ------- */
    connect(ui->browseButton,     &QPushButton::clicked,
            this,                 &UserSettingsDialog::onBrowseAvatar);

    connect(ui->changePassButton, &QPushButton::clicked,
            this,                 &UserSettingsDialog::onChangePassword);

    connect(ui->deleteAccButton,  &QPushButton::clicked,
            this,                 &UserSettingsDialog::onDeleteAccount);

    connect(ui->logoutButton,     &QPushButton::clicked,
            this,                 &UserSettingsDialog::onLogoutClicked);

    /* El buttonBox (OK/Cancel) ya llama a accept()/reject() por defecto */
}

UserSettingsDialog::~UserSettingsDialog(){ delete ui; }

/* ======= slots ======= */

void UserSettingsDialog::onBrowseAvatar()
{
    QString fn = QFileDialog::getOpenFileName(
        this, "Elegir imagen",
        QString(),
        "Imágenes (*.png *.jpg *.jpeg *.bmp)");
    if(fn.isEmpty()) return;

    if(m_core->updateAvatar(fn.toStdString())){
        ui->avatarLabel->setPixmap(QPixmap(fn).scaled(
            ui->avatarLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
        QMessageBox::information(this,"Avatar","Avatar actualizado");
        accept();                          // cierra indicando éxito
    }else{
        QMessageBox::warning(this,"Error","No se pudo actualizar avatar");
    }
}

void UserSettingsDialog::onChangePassword()
{
    QString oldP = ui->oldPassEdit->text();
    QString newP = ui->newPassEdit->text();
    QString conf = ui->confPassEdit->text();

    if(newP != conf){
        QMessageBox::warning(this,"Error","La confirmación no coincide");
        return;
    }
    if(!m_core->updatePassword(oldP.toStdString(),newP.toStdString())){
        QMessageBox::warning(this,"Error",
                             "Contraseña actual incorrecta "
                             "o nueva contraseña muy corta (≥6)");
        return;
    }
    QMessageBox::information(this,"Contraseña","Actualizada con éxito");
    accept();
}

void UserSettingsDialog::onDeleteAccount()
{
    auto r = QMessageBox::question(this,"Eliminar cuenta",
                                   "¿Seguro que deseas eliminar tu cuenta?\n"
                                   "Esta acción no se puede deshacer.",
                                   QMessageBox::Yes | QMessageBox::No);
    if(r == QMessageBox::No) return;

    if(m_core->deleteCurrentUser()){
        QMessageBox::information(this,"Cuenta","Cuenta eliminada");
        reject();                    // MainWindow gestionará la salida
    }else{
        QMessageBox::warning(this,"Error","No se pudo borrar la cuenta");
    }
}

void UserSettingsDialog::onLogoutClicked()
{
    m_core->logout();                // marca la sesión como cerrada
    emit logoutRequested();          // avisa a MainWindow
    reject();                        // cierra el diálogo
}
