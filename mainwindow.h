#pragma once
#include <QMainWindow>
#include <QListWidget>          // ← añade esta cabecera
#include "chatcore.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(ChatCore* core, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onContactSelected();
    void onAddContact();
    void onUndoClicked();
    void onDebugReceive();
    void openUserSettings();      // abre el diálogo
    void onLoggedOut();           // maneja regreso a login
    void reloadContacts();              // nuevo helper
    void onSortChanged(int);            // slot


private:
    void loadHistory(const QString& peer);
    QListWidgetItem* findItem(const QString& user) const;

    Ui::MainWindow *ui{};
    ChatCore* m_core{};
    QString   m_currentPeer;
    Stack<Message<std::string>> m_undo;
};
