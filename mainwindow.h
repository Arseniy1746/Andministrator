#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include "database.h"
#include "usermanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRefresh();
    void onUserSelectionChanged();
    void onMessageSelectionChanged();
    void onDisconnectUser();
    void onBanUser();
    void onUnbanUser();
    void onDeleteMessage();
    void onSearchUsers(const QString &text);
    void onFilterStatusChanged(int index);
    void onSearchMessages(const QString &text);
    void onMsgTypeChanged(int index);
    void onMsgUserChanged(int index);
    void onClearLogs();
    void onActionExit();
    void onActionAbout();
    void onActionDatabase();

private:
    Ui::MainWindow *ui;
    Database *m_db;
    UserManager *m_userManager;
    QList<UserInfo> m_allUsers;
    QList<MessageInfo> m_allMessages;

    void setupUiStyles();
    void setupConnections();
    void loadUsers();
    void loadMessages();
    void populateUserFilter();
    void applyUserFilters();
    void applyMessageFilters();
    void logAction(const QString &action);
    void updateStatusBar(const QString &text);
    QString statusToDisplay(const QString &status) const;
    QColor statusColor(const QString &status) const;
    int selectedUserId() const;
    int selectedMessageId() const;
};

#endif // MAINWINDOW_H
