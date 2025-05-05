#ifndef USERSETTINGSDIALOG_H
#define USERSETTINGSDIALOG_H

#include <QDialog>
#include "chatcore.h"

namespace Ui { class UserSettingsDialog; }

class UserSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserSettingsDialog(ChatCore* core,
                                QWidget *parent = nullptr);
    ~UserSettingsDialog();

signals:
    void logoutRequested();              // ← NUEVA SEÑAL

private slots:
    void onBrowseAvatar();
    void onChangePassword();
    void onDeleteAccount();
    void onLogoutClicked();              // ← NUEVO SLOT

private:
    Ui::UserSettingsDialog *ui;
    ChatCore* m_core;
    QString   m_avatarPath;
};

#endif
