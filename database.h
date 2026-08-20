#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QDateTime>

struct UserInfo
{
    int id;
    QString username;
    QString status;      //"online", "offline", "banned"
    QDateTime lastSeen;
    QString ipAddress;
    int messagesCount;
};

struct MessageInfo
{
    int id;
    QString fromUser;
    QString toUser;      // empty or "ALL" for public
    bool isPrivate;
    QString content;
    QDateTime timestamp;
};

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool connectToDatabase(const QString &host = "localhost", int port = 5432, const QString &dbName = "messenger", const QString &user = "postgres", const QString &password = "postgres");

    bool isConnected() const;
    QString lastError() const;

    // Users
    QList<UserInfo> getAllUsers();
    bool banUser(int userId);
    bool unbanUser(int userId);
    bool setUserStatus(int userId, const QString &status);
    UserInfo getUserById(int userId);

    // Messages
    QList<MessageInfo> getAllMessages();
    QList<MessageInfo> getMessagesByUser(const QString &username);
    bool deleteMessage(int messageId);

    // For demo / testing without real Postgres
    void loadDemoData();

signals:
    void connectionStatusChanged(bool connected);

private:
    QSqlDatabase m_db;
    QString m_lastError;
    bool m_useDemoData = false;
    QList<UserInfo> m_demoUsers;
    QList<MessageInfo> m_demoMessages;

    void initDemoData();
};

#endif // DATABASE_H
