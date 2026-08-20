#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHeaderView>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_db(new Database(this))
    , m_userManager(new UserManager(this))
{
    ui->setupUi(this);
    setupUiStyles();
    setupConnections();

    // Try connect (falls back to demo data automatically)
    m_db->connectToDatabase();

    if (m_db->isConnected()) {
        ui->label_status->setText("● Connected (Demo / DB)");
        ui->label_status->setStyleSheet("color: #0f9d58; font-weight: bold;");
    } else {
        ui->label_status->setText("● Disconnected");
        ui->label_status->setStyleSheet("color: #d93025; font-weight: bold;");
    }

    loadUsers();
    loadMessages();
    populateUserFilter();
    logAction("Admin panel started");

    // Auto-refresh every 30 seconds
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRefresh);
    timer->start(30000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUiStyles()
{
    // Modern dark-ish / clean stylesheet
    QString style = R"(
        QMainWindow {
            background-color: #f5f7fa;
        }
        QTabWidget::pane {
            border: 1px solid #d0d7de;
            border-radius: 8px;
            background: white;
            top: -1px;
        }
        QTabBar::tab {
            background: #eaeef2;
            border: 1px solid #d0d7de;
            border-bottom: none;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            padding: 10px 22px;
            margin-right: 3px;
            font-weight: 600;
            color: #57606a;
        }
        QTabBar::tab:selected {
            background: white;
            color: #1a73e8;
            border-bottom: 2px solid #1a73e8;
        }
        QTabBar::tab:hover:!selected {
            background: #f0f3f6;
        }
        QTableWidget {
            background-color: white;
            border: 1px solid #d0d7de;
            border-radius: 6px;
            gridline-color: #eaeef2;
            selection-background-color: #dbeafe;
            selection-color: #1e3a5f;
            font-size: 13px;
            color: #24292f;
            alternate-background-color: #f0f4f8;
        }
        QTableWidget::item {
            padding: 6px;
            color: #24292f;
            background-color: transparent;
        }
        QTableWidget::item:selected {
            background-color: #dbeafe;
            color: #1e3a5f;
        }
        QTableWidget::item:alternate {
            background-color: #f0f4f8;
            color: #24292f;
        }
        QHeaderView::section {
            background-color: #f6f8fa;
            color: #24292f;
            padding: 8px;
            border: none;
            border-bottom: 2px solid #d0d7de;
            font-weight: 600;
        }
        QLineEdit, QComboBox {
            border: 1px solid #d0d7de;
            border-radius: 6px;
            padding: 6px 12px;
            background: white;
            color: #24292f;
            font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 2px solid #1a73e8;
            color: #24292f;
        }
        QComboBox QAbstractItemView {
            background: white;
            color: #24292f;
            selection-background-color: #dbeafe;
            selection-color: #1e3a5f;
            border: 1px solid #d0d7de;
        }
        QComboBox::drop-down {
            border: none;
            width: 28px;
        }
        QComboBox::down-arrow {
            /* keep default arrow */
        }
        QPushButton {
            background-color: #1a73e8;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #1557b0;
        }
        QPushButton:pressed {
            background-color: #0d47a1;
        }
        QPushButton:disabled {
            background-color: #a0c4f1;
            color: #e8f0fe;
        }
        QPushButton#btnBan {
            background-color: #d93025;
        }
        QPushButton#btnBan:hover {
            background-color: #b3261e;
        }
        QPushButton#btnBan:disabled {
            background-color: #f5b7b1;
        }
        QPushButton#btnUnban {
            background-color: #0f9d58;
        }
        QPushButton#btnUnban:hover {
            background-color: #0b8043;
        }
        QPushButton#btnUnban:disabled {
            background-color: #a8d5c0;
        }
        QPushButton#btnDisconnect {
            background-color: #f9ab00;
            color: #1a1a1a;
        }
        QPushButton#btnDisconnect:hover {
            background-color: #e09600;
        }
        QPushButton#btnDisconnect:disabled {
            background-color: #fde293;
        }
        QPushButton#btnDeleteMessage {
            background-color: #d93025;
        }
        QTextEdit {
            border: 1px solid #d0d7de;
            border-radius: 6px;
            background: #fafbfc;
            color: #24292f;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 12px;
        }
        QStatusBar {
            background: #eaeef2;
        }
        QMenuBar {
            background: #ffffff;
            border-bottom: 1px solid #d0d7de;
        }
        QMenuBar::item:selected {
            background: #e8f0fe;
        }
        QLabel#label_title {
            font-size: 22px;
            font-weight: bold;
            color: #1a73e8;
        }
    )";
    this->setStyleSheet(style);

    //Table column widths
    ui->tableUsers->horizontalHeader()->setStretchLastSection(true);
    ui->tableUsers->setColumnWidth(0, 60);
    ui->tableUsers->setColumnWidth(1, 140);
    ui->tableUsers->setColumnWidth(2, 100);
    ui->tableUsers->setColumnWidth(3, 160);
    ui->tableUsers->setColumnWidth(4, 130);

    ui->tableMessages->horizontalHeader()->setStretchLastSection(true);
    ui->tableMessages->setColumnWidth(0, 60);
    ui->tableMessages->setColumnWidth(1, 110);
    ui->tableMessages->setColumnWidth(2, 110);
    ui->tableMessages->setColumnWidth(3, 90);
    ui->tableMessages->setColumnWidth(4, 400);

    ui->tableUsers->verticalHeader()->setVisible(false);
    ui->tableMessages->verticalHeader()->setVisible(false);

    //Make tables read-only (no editing cells)
    ui->tableUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableMessages->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::setupConnections()
{
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::onRefresh);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onActionExit);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->actionDatabase, &QAction::triggered, this, &MainWindow::onActionDatabase);

    connect(ui->tableUsers, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onUserSelectionChanged);
    connect(ui->tableMessages, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onMessageSelectionChanged);

    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectUser);
    connect(ui->btnBan, &QPushButton::clicked, this, &MainWindow::onBanUser);
    connect(ui->btnUnban, &QPushButton::clicked, this, &MainWindow::onUnbanUser);
    connect(ui->btnDeleteMessage, &QPushButton::clicked, this, &MainWindow::onDeleteMessage);
    connect(ui->btnClearLogs, &QPushButton::clicked, this, &MainWindow::onClearLogs);

    connect(ui->lineEditSearchUsers, &QLineEdit::textChanged,
            this, &MainWindow::onSearchUsers);
    connect(ui->comboFilterStatus, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterStatusChanged);

    connect(ui->lineEditSearchMessages, &QLineEdit::textChanged,
            this, &MainWindow::onSearchMessages);
    connect(ui->comboMsgType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMsgTypeChanged);
    connect(ui->comboMsgUser, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMsgUserChanged);
}

void MainWindow::loadUsers()
{
    m_allUsers = m_db->getAllUsers();

    //Sync online status with UserManager for demo
    for (auto &u : m_allUsers)
    {
        if (u.status != "banned")
        {
            if (m_userManager->isOnline(u.id))
                u.status = "online";
            else if (u.status != "banned")
                u.status = "offline";
        }
    }

    applyUserFilters();
}

void MainWindow::loadMessages()
{
    m_allMessages = m_db->getAllMessages();
    //Sort by timestamp descending
    std::sort(m_allMessages.begin(), m_allMessages.end(),
              [](const MessageInfo &a, const MessageInfo &b)
              {
                  return a.timestamp > b.timestamp;
              });
    applyMessageFilters();
}

void MainWindow::populateUserFilter()
{
    ui->comboMsgUser->clear();
    ui->comboMsgUser->addItem("All users", QVariant());
    QSet<QString> names;
    for (const auto &u : m_allUsers)
        names.insert(u.username);
    QStringList sorted = names.values();
    sorted.sort();
    for (const QString &n : sorted)
        ui->comboMsgUser->addItem(n, n);
}

void MainWindow::applyUserFilters()
{
    QString search = ui->lineEditSearchUsers->text().trimmed().toLower();
    QString statusFilter = ui->comboFilterStatus->currentText();

    ui->tableUsers->setRowCount(0);
    int visible = 0;

    for (const auto &u : m_allUsers)
    {
        if (!search.isEmpty())
        {
            bool match = u.username.toLower().contains(search) ||
                         QString::number(u.id).contains(search);
            if (!match) continue;
        }
        if (statusFilter != "All statuses")
        {
            if (statusFilter.toLower() != u.status.toLower())
                continue;
        }

        int row = ui->tableUsers->rowCount();
        ui->tableUsers->insertRow(row);

        const QColor textColor(0x24, 0x29, 0x2f); //dark text for readability

        auto *idItem = new QTableWidgetItem(QString::number(u.id));
        idItem->setData(Qt::UserRole, u.id);
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setForeground(textColor);
        ui->tableUsers->setItem(row, 0, idItem);

        auto *nameItem = new QTableWidgetItem(u.username);
        nameItem->setForeground(textColor);
        ui->tableUsers->setItem(row, 1, nameItem);

        auto *statusItem = new QTableWidgetItem(statusToDisplay(u.status));
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(statusColor(u.status));
        QFont f = statusItem->font();
        f.setBold(true);
        statusItem->setFont(f);
        ui->tableUsers->setItem(row, 2, statusItem);

        auto *seenItem = new QTableWidgetItem(u.lastSeen.toString("yyyy-MM-dd HH:mm:ss"));
        seenItem->setForeground(textColor);
        ui->tableUsers->setItem(row, 3, seenItem);

        auto *ipItem = new QTableWidgetItem(u.ipAddress);
        ipItem->setForeground(textColor);
        ui->tableUsers->setItem(row, 4, ipItem);

        auto *cntItem = new QTableWidgetItem(QString::number(u.messagesCount));
        cntItem->setTextAlignment(Qt::AlignCenter);
        cntItem->setForeground(textColor);
        ui->tableUsers->setItem(row, 5, cntItem);

        ++visible;
    }

    ui->labelUsersCount->setText(QString("Total users: %1 (showing %2)")
                                     .arg(m_allUsers.size()).arg(visible));
    onUserSelectionChanged();
}

void MainWindow::applyMessageFilters()
{
    QString search = ui->lineEditSearchMessages->text().trimmed().toLower();
    int typeIdx = ui->comboMsgType->currentIndex(); //0=all, 1=public, 2=private
    QString userFilter = ui->comboMsgUser->currentData().toString();

    ui->tableMessages->setRowCount(0);
    int visible = 0;

    for (const auto &m : m_allMessages)
    {
        if (!search.isEmpty() && !m.content.toLower().contains(search) &&
            !m.fromUser.toLower().contains(search) &&
            !m.toUser.toLower().contains(search))
        {
            continue;
        }
        if (typeIdx == 1 && m.isPrivate) continue;
        if (typeIdx == 2 && !m.isPrivate) continue;
        if (!userFilter.isEmpty() && m.fromUser != userFilter && m.toUser != userFilter)
            continue;

        int row = ui->tableMessages->rowCount();
        ui->tableMessages->insertRow(row);

        const QColor textColor(0x24, 0x29, 0x2f);

        auto *idItem = new QTableWidgetItem(QString::number(m.id));
        idItem->setData(Qt::UserRole, m.id);
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setForeground(textColor);
        ui->tableMessages->setItem(row, 0, idItem);

        auto *fromItem = new QTableWidgetItem(m.fromUser);
        fromItem->setForeground(textColor);
        ui->tableMessages->setItem(row, 1, fromItem);

        auto *toItem = new QTableWidgetItem(m.toUser);
        toItem->setForeground(textColor);
        ui->tableMessages->setItem(row, 2, toItem);

        auto *typeItem = new QTableWidgetItem(m.isPrivate ? "🔒 Private" : "🌐 Public");
        typeItem->setTextAlignment(Qt::AlignCenter);
        if (m.isPrivate)
            typeItem->setForeground(QColor("#d93025"));
        else
            typeItem->setForeground(QColor("#0f9d58"));
        ui->tableMessages->setItem(row, 3, typeItem);

        auto *contentItem = new QTableWidgetItem(m.content);
        contentItem->setForeground(textColor);
        ui->tableMessages->setItem(row, 4, contentItem);

        auto *timeItem = new QTableWidgetItem(m.timestamp.toString("yyyy-MM-dd HH:mm:ss"));
        timeItem->setForeground(textColor);
        ui->tableMessages->setItem(row, 5, timeItem);

        ++visible;
    }

    ui->labelMessagesCount->setText(QString("Total messages: %1 (showing %2)")
                                        .arg(m_allMessages.size()).arg(visible));
    onMessageSelectionChanged();
}

QString MainWindow::statusToDisplay(const QString &status) const
{
    if (status == "online")  return "🟢 Online";
    if (status == "offline") return "⚪ Offline";
    if (status == "banned")  return "🔴 Banned";
    return status;
}

QColor MainWindow::statusColor(const QString &status) const
{
    if (status == "online")  return QColor("#0f9d58");
    if (status == "offline") return QColor("#57606a");
    if (status == "banned")  return QColor("#d93025");
    return QColor("#24292f");
}

int MainWindow::selectedUserId() const
{
    auto items = ui->tableUsers->selectedItems();
    if (items.isEmpty()) return -1;
    int row = items.first()->row();
    auto *item = ui->tableUsers->item(row, 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

int MainWindow::selectedMessageId() const
{
    auto items = ui->tableMessages->selectedItems();
    if (items.isEmpty()) return -1;
    int row = items.first()->row();
    auto *item = ui->tableMessages->item(row, 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

void MainWindow::onRefresh()
{
    loadUsers();
    loadMessages();
    populateUserFilter();
    logAction("Data refreshed");
    updateStatusBar("Data refreshed at " + QDateTime::currentDateTime().toString("HH:mm:ss"));
}

void MainWindow::onUserSelectionChanged()
{
    int id = selectedUserId();
    bool hasSelection = id > 0;
    ui->btnDisconnect->setEnabled(false);
    ui->btnBan->setEnabled(false);
    ui->btnUnban->setEnabled(false);

    if (!hasSelection) return;

    UserInfo u = m_db->getUserById(id);
    //Prefer local list status
    for (const auto &uu : m_allUsers)
    {
        if (uu.id == id) { u = uu; break; }
    }

    if (u.status == "banned")
    {
        ui->btnUnban->setEnabled(true);
    } else
    {
        ui->btnBan->setEnabled(true);
        if (u.status == "online")
            ui->btnDisconnect->setEnabled(true);
    }
}

void MainWindow::onMessageSelectionChanged()
{
    ui->btnDeleteMessage->setEnabled(selectedMessageId() > 0);
}

void MainWindow::onDisconnectUser()
{
    int id = selectedUserId();
    if (id <= 0) return;

    UserInfo u = m_db->getUserById(id);
    for (const auto &uu : m_allUsers)
        if (uu.id == id) { u = uu; break; }

    auto reply = QMessageBox::question(this, "Disconnect user",
        QString("Force disconnect user <b>%1</b> (ID %2)?").arg(u.username).arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    if (m_userManager->disconnectUser(id))
    {
        m_db->setUserStatus(id, "offline");
        logAction(QString("Disconnected user %1 (ID %2)").arg(u.username).arg(id));
        loadUsers();
        updateStatusBar(QString("User %1 disconnected").arg(u.username));
    } else
    {
        QMessageBox::information(this, "Info", "User is already offline.");
    }
}

void MainWindow::onBanUser()
{
    int id = selectedUserId();
    if (id <= 0) return;

    UserInfo u;
    for (const auto &uu : m_allUsers)
        if (uu.id == id) { u = uu; break; }

    auto reply = QMessageBox::warning(this, "Ban user",
        QString("Ban user <b>%1</b> (ID %2)?\n\n"
                "The user will be immediately disconnected and will not be able to log in.")
            .arg(u.username).arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    m_userManager->disconnectUser(id);
    if (m_db->banUser(id))
    {
        logAction(QString("BANNED user %1 (ID %2)").arg(u.username).arg(id));
        loadUsers();
        updateStatusBar(QString("User %1 banned").arg(u.username));
    } else
    {
        QMessageBox::critical(this, "Error", "Failed to ban user: " + m_db->lastError());
    }
}

void MainWindow::onUnbanUser()
{
    int id = selectedUserId();
    if (id <= 0) return;

    UserInfo u;
    for (const auto &uu : m_allUsers)
        if (uu.id == id) { u = uu; break; }

    auto reply = QMessageBox::question(this, "Unban user",
        QString("Unban user <b>%1</b> (ID %2)?").arg(u.username).arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    if (m_db->unbanUser(id))
    {
        logAction(QString("Unbanned user %1 (ID %2)").arg(u.username).arg(id));
        loadUsers();
        updateStatusBar(QString("User %1 unbanned").arg(u.username));
    } else
    {
        QMessageBox::critical(this, "Error", "Failed to unban user: " + m_db->lastError());
    }
}

void MainWindow::onDeleteMessage()
{
    int id = selectedMessageId();
    if (id <= 0) return;

    auto reply = QMessageBox::question(this, "Delete message",
        QString("Permanently delete message ID %1?").arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    if (m_db->deleteMessage(id))
    {
        logAction(QString("Deleted message ID %1").arg(id));
        loadMessages();
        updateStatusBar(QString("Message %1 deleted").arg(id));
    } else
    {
        QMessageBox::critical(this, "Error", "Failed to delete message: " + m_db->lastError());
    }
}

void MainWindow::onSearchUsers(const QString &)
{
    applyUserFilters();
}

void MainWindow::onFilterStatusChanged(int)
{
    applyUserFilters();
}

void MainWindow::onSearchMessages(const QString &)
{
    applyMessageFilters();
}

void MainWindow::onMsgTypeChanged(int)
{
    applyMessageFilters();
}

void MainWindow::onMsgUserChanged(int)
{
    applyMessageFilters();
}

void MainWindow::onClearLogs()
{
    ui->textEditLogs->clear();
    logAction("Logs cleared");
}

void MainWindow::onActionExit()
{
    QApplication::quit();
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, "About Server Admin",
        "<h2>Messenger Server Admin Panel</h2>"
        "<p>Qt + C++ administration tool for the messenger server.</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>View all users and their status</li>"
        "<li>View public &amp; private messages</li>"
        "<li>Disconnect / Ban / Unban users</li>"
        "<li>Delete messages</li>"
        "<li>Search &amp; filter</li>"
        "</ul>"
        "<p>Supports PostgreSQL (with demo data fallback).</p>"
        "<p>Built for educational purposes.</p>");
}

void MainWindow::onActionDatabase()
{
    bool ok;
    QString host = QInputDialog::getText(this, "Database", "Host:",
                                         QLineEdit::Normal, "localhost", &ok);
    if (!ok) return;
    int port = QInputDialog::getInt(this, "Database", "Port:", 5432, 1, 65535, 1, &ok);
    if (!ok) return;
    QString dbName = QInputDialog::getText(this, "Database", "Database name:",
                                           QLineEdit::Normal, "messenger", &ok);
    if (!ok) return;
    QString user = QInputDialog::getText(this, "Database", "User:",
                                         QLineEdit::Normal, "postgres", &ok);
    if (!ok) return;
    QString pass = QInputDialog::getText(this, "Database", "Password:",
                                         QLineEdit::Password, "", &ok);
    if (!ok) return;

    if (m_db->connectToDatabase(host, port, dbName, user, pass))
    {
        ui->label_status->setText("● Connected to Database");
        ui->label_status->setStyleSheet("color: #0f9d58; font-weight: bold;");
        onRefresh();
        logAction(QString("Connected to DB %1@%2:%3/%4").arg(user, host).arg(port).arg(dbName));
    } else
    {
        ui->label_status->setText("● Connection failed (using demo)");
        ui->label_status->setStyleSheet("color: #f9ab00; font-weight: bold;");
        QMessageBox::warning(this, "Connection",
                             "Could not connect to PostgreSQL.\nUsing demo data.\n\nError: " +
                             m_db->lastError());
    }
}

void MainWindow::logAction(const QString &action)
{
    QString line = QString("[%1] %2")
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
                            action);
    ui->textEditLogs->append(line);
}

void MainWindow::updateStatusBar(const QString &text)
{
    ui->statusbar->showMessage(text, 5000);
}
