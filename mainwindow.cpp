#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "usersettingsdialog.h"
#include "logindialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QPixmap>
#include <fstream>

/* ------------- helpers globales ------------- */
static QListWidget*        gList = nullptr;
static const ChatCore*     gCore = nullptr;

/* Añade un elemento formateado a la QListWidget */
static void addItemInitial(const std::string& c)
{
    int n = gCore->unreadCount(c);
    QString lbl = QString::fromStdString(c);
    if(n) lbl += " (" + QString::number(n) + ')';

    auto* it = new QListWidgetItem(lbl, gList);
    it->setData(Qt::UserRole, QString::fromStdString(c));
}

/* ruta historial  a_b_chat.txt */
static QString histPath(const std::string& a,const std::string& b){
    return QString::fromStdString((a<b? a+'_'+b : b+'_'+a) + "_chat.txt");
}

/* ========================================================= */
MainWindow::MainWindow(ChatCore* core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_core(core)
{
    ui->setupUi(this);

    /* -------- conexiones principales -------- */
    connect(ui->sendButton,      &QPushButton::clicked,
            this,                &MainWindow::onSendClicked);

    connect(ui->contactsListWidget, &QListWidget::itemClicked,
            this,                   &MainWindow::onContactSelected);

    connect(ui->addContactButton,&QPushButton::clicked,
            this,                &MainWindow::onAddContact);

    connect(ui->undoButton,      &QPushButton::clicked,
            this,                &MainWindow::onUndoClicked);

    connect(ui->debugRecvButton, &QPushButton::clicked,
            this,                &MainWindow::onDebugReceive);

    /* ---- botón Configuración ---- */
    connect(ui->settingsButton,&QPushButton::clicked,
            this,&MainWindow::openUserSettings);

    /* ---- combo de orden ---- */
    connect(ui->sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortChanged);

    /* ---- poblar lista inicial ---- */
    reloadContacts();

    /* ---- avatar ---- */
    QString av = QString::fromStdString(m_core->currentUser().avatar);
    if(!av.isEmpty() && ui->avatarLabel){
        QPixmap px(av);
        ui->avatarLabel->setPixmap(px.scaled(ui->avatarLabel->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
    }
}

MainWindow::~MainWindow(){ delete ui; }

/* ========== Configuración / Logout ========== */
void MainWindow::openUserSettings()
{
    UserSettingsDialog dlg(m_core, this);
    connect(&dlg,&UserSettingsDialog::logoutRequested,
            this,&MainWindow::onLoggedOut);

    int res = dlg.exec();

    /* refrescar avatar si hubo cambios y no se cerró sesión */
    if(res==QDialog::Accepted && m_core->loggedIn()){
        QString av = QString::fromStdString(m_core->currentUser().avatar);
        if(!av.isEmpty() && ui->avatarLabel){
            ui->avatarLabel->setPixmap(QPixmap(av).scaled(
                ui->avatarLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
        }
    }
}

void MainWindow::onLoggedOut()
{
    close();

    LoginDialog dlg(m_core, nullptr);
    if(dlg.exec()==QDialog::Accepted){
        MainWindow *w = new MainWindow(m_core);
        w->show();
    }else{
        qApp->quit();
    }
}

/* ==========  Recarga / orden  ========== */
void MainWindow::reloadContacts()
{
    ui->contactsListWidget->clear();

    ContactSort mode = ContactSort::Alphabetical;
    switch(ui->sortCombo->currentIndex()){
    case 1: mode = ContactSort::LastMessageDesc; break;
    case 2: mode = ContactSort::ChatLengthDesc;  break;
    }
    gList = ui->contactsListWidget;
    gCore = m_core;

    for(const auto& c : m_core->contactsSorted(mode))
        addItemInitial(c);

    gList = nullptr; gCore = nullptr;
}

void MainWindow::onSortChanged(int){ reloadContacts(); }

/* ------------- helpers internos ------------- */
QListWidgetItem* MainWindow::findItem(const QString& user) const
{
    for(int i=0;i<ui->contactsListWidget->count();++i){
        auto* it = ui->contactsListWidget->item(i);
        if(it->data(Qt::UserRole).toString() == user) return it;
    }
    return nullptr;
}

/* ------------- envío de mensaje -------------- */
void MainWindow::onSendClicked()
{
    if(m_currentPeer.isEmpty()) return;
    QString txt = ui->messageLineEdit->text().trimmed();
    if(txt.isEmpty()) return;

    ui->chatTextEdit->append("Yo: " + txt);

    Message<std::string> m{
        m_core->currentUser().username,
        m_currentPeer.toStdString(),
        time(nullptr),
        txt.toStdString(),
        MsgType::TEXT
    };
    m_core->saveMessage(m);
    m_undo.push(m);
    ui->messageLineEdit->clear();
}

/* ---------- seleccionar contacto ------------ */
void MainWindow::onContactSelected()
{
    m_currentPeer = ui->contactsListWidget->currentItem()
    ->data(Qt::UserRole).toString();
    m_undo = {};

    loadHistory(m_currentPeer);

    auto q = m_core->takeUnread(m_currentPeer.toStdString());
    while(!q.empty()){
        auto msg = q.dequeue();
        ui->chatTextEdit->append(
            QString::fromStdString(msg.from + ": " + msg.content));
    }
    if(auto* it=findItem(m_currentPeer)) it->setText(m_currentPeer);

    m_core->markSeen(m_currentPeer.toStdString());
}

/* ---------- agregar contacto ---------------- */
void MainWindow::onAddContact()
{
    bool ok;
    QString u = QInputDialog::getText(this,"Agregar contacto","Usuario:",
                                      QLineEdit::Normal,"",&ok);
    if(!ok || u.isEmpty()) return;

    if(!m_core->userExists(u.toStdString())){
        QMessageBox::warning(this,"Error","El usuario no existe"); return;
    }
    if(m_core->currentUser().contacts.contains(u.toStdString())){
        QMessageBox::information(this,"Info","Ya está en la lista"); return;
    }

    m_core->currentUser().contacts.push_back(u.toStdString());
    m_core->saveUser(m_core->currentUser());
    m_core->persistContact(u.toStdString());

    reloadContacts();                      // respeta orden actual
}

/* ---------- deshacer ------------------------ */
void MainWindow::onUndoClicked()
{
    if(m_currentPeer.isEmpty() || m_undo.empty()){
        QMessageBox::information(this,"Deshacer","Nada que deshacer"); return;
    }
    m_undo.pop();
    m_core->removeLastFromHistory(
        m_core->currentUser().username,
        m_currentPeer.toStdString());
    loadHistory(m_currentPeer);
}

/* ---------- mensaje simulado ---------------- */
void MainWindow::onDebugReceive()
{
    if(ui->contactsListWidget->count()==0) return;

    QString sender = ui->contactsListWidget->item(0)
                         ->data(Qt::UserRole).toString();

    Message<std::string> m{
        sender.toStdString(),
        m_core->currentUser().username,
        time(nullptr),
        "Mensaje simulado",
        MsgType::TEXT
    };
    m_core->saveMessage(m);

    if(sender != m_currentPeer){
        m_core->storeUnread(m);
        int n = m_core->unreadCount(sender.toStdString());
        if(auto* it=findItem(sender))
            it->setText(sender + " (" + QString::number(n) + ")");
    }else{
        ui->chatTextEdit->append(sender + ": Mensaje simulado");
    }
}

/* ---------- cargar historial ---------------- */
void MainWindow::loadHistory(const QString& peer)
{
    ui->chatTextEdit->clear();
    std::ifstream f(histPath(m_core->currentUser().username,
                             peer.toStdString()).toStdString());
    std::string line;
    while(std::getline(f,line)){
        auto m = Message<std::string>::deserialize(line);
        QString q = QString("[%1] %2: %3")
                        .arg(QString::fromStdString(timestampToStr(m.ts)))
                        .arg(QString::fromStdString(m.from))
                        .arg(QString::fromStdString(m.content));
        ui->chatTextEdit->append(q);
    }
}
