#include "database.h"
#include <QDebug>
#include <QRandomGenerator>

Database::Database(QObject *parent)
    : QObject(parent)
{
    initDemoData();
}

Database::~Database()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

bool Database::connectToDatabase(const QString &host, int port, const QString &dbName, const QString &user, const QString &password)
{
    //Try real Postgres first
    m_db = QSqlDatabase::addDatabase("QPSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (m_db.open())
    {
        m_useDemoData = false;
        m_lastError.clear();
        emit connectionStatusChanged(true);
        qDebug() << "Connected to PostgreSQL successfully";
        return true;
    }

    // Fallback to demo data if Postgres is not available
    m_lastError = m_db.lastError().text();
    qWarning() << "PostgreSQL connection failed:" << m_lastError;
    qWarning() << "Falling back to demo data mode";
    m_useDemoData = true;
    emit connectionStatusChanged(true); // still "connected" in demo mode
    return true; // we consider demo mode as successful for the UI
}

bool Database::isConnected() const
{
    return m_useDemoData || m_db.isOpen();
}

QString Database::lastError() const
{
    return m_lastError;
}

void Database::initDemoData()
{
    m_demoUsers.clear();
    m_demoMessages.clear();

    // Demo users
    QStringList names = {"alice", "bob", "charlie", "diana", "eve", "frank", "grace", "henry"};
    QStringList ips = {"192.168.1.10", "192.168.1.15", "10.0.0.5", "172.16.0.8",
                       "192.168.0.22", "10.0.1.3", "192.168.2.50", "10.10.10.1"};
    QStringList statuses = {"online", "online", "offline", "banned", "online", "offline", "online", "offline"};

    for (int i = 0; i < names.size(); ++i)
    {
        UserInfo u;
        u.id = i + 1;
        u.username = names[i];
        u.status = statuses[i];
        u.lastSeen = QDateTime::currentDateTime().addSecs(-QRandomGenerator::global()->bounded(0, 86400 * 3));
        u.ipAddress = ips[i];
        u.messagesCount = QRandomGenerator::global()->bounded(5, 120);
        m_demoUsers.append(u);
    }

    //Demo messages
    QStringList publicMsgs =
    {
        "Hello everyone!",
        "How is the project going?",
        "Does anyone know the deadline?",
        "Great work on the last commit!",
        "Meeting at 15:00 today",
        "Who fixed the bug in login?",
        "Server is running smoothly",
        "Don't forget to pull the latest changes"
    };

    QStringList privateMsgs =
    {
        "Can you review my PR?",
        "I need help with the database schema",
        "Secret: the password is ... just kidding",
        "Are you free for a call?",
        "Please don't share this info",
        "Thanks for the tip!",
        "I'll send the files later",
        "Keep this between us"
    };

    int msgId = 1;
    //Public messages
    for (int i = 0; i < publicMsgs.size(); ++i)
    {
        MessageInfo m;
        m.id = msgId++;
        m.fromUser = names[i % names.size()];
        m.toUser = "ALL";
        m.isPrivate = false;
        m.content = publicMsgs[i];
        m.timestamp = QDateTime::currentDateTime().addSecs(-QRandomGenerator::global()->bounded(100, 50000));
        m_demoMessages.append(m);
    }

    //Private messages
    for (int i = 0; i < privateMsgs.size(); ++i)
    {
        MessageInfo m;
        m.id = msgId++;
        m.fromUser = names[i % names.size()];
        m.toUser = names[(i + 3) % names.size()];
        m.isPrivate = true;
        m.content = privateMsgs[i];
        m.timestamp = QDateTime::currentDateTime().addSecs(-QRandomGenerator::global()->bounded(100, 50000));
        m_demoMessages.append(m);
    }

    // More realistic mixed messages
    QStringList extraPublic =
        {
        "Anyone free for pair programming?",
        "Just pushed the fix for issue #42",
        "Tests are green now 🎉",
        "Reminder: code freeze on Friday",
        "New design mockups are in Figma",
        "Who is on call this weekend?",
        "Database migration completed successfully",
        "Please update your local branches",
        "Great job everyone on the release!",
        "Lunch break in 10 minutes?"
    };

    QStringList extraPrivate =
        {
        "Can you check the logs on production?",
        "I left some comments on your PR",
        "The client asked for a small change",
        "Don't merge yet, waiting for review",
        "Here's the link to the document",
        "Call me when you have a minute",
        "I think we should discuss this offline",
        "Password reset token was sent",
        "Your access has been updated",
        "Thanks, that solved the problem!"
    };

    for (int i = 0; i < 20; ++i)
    {
        MessageInfo m;
        m.id = msgId++;
        m.fromUser = names[QRandomGenerator::global()->bounded(names.size())];
        bool priv = QRandomGenerator::global()->bounded(2);
        m.isPrivate = priv;
        if (priv) {
            m.toUser = names[QRandomGenerator::global()->bounded(names.size())];
            while (m.toUser == m.fromUser)
                m.toUser = names[QRandomGenerator::global()->bounded(names.size())];
            m.content = extraPrivate[i % extraPrivate.size()];
        } else {
            m.toUser = "ALL";
            m.content = extraPublic[i % extraPublic.size()];
        }
        m.timestamp = QDateTime::currentDateTime().addSecs(-QRandomGenerator::global()->bounded(50, 100000));
        m_demoMessages.append(m);
    }
}

void Database::loadDemoData()
{
    m_useDemoData = true;
    initDemoData();
}

QList<UserInfo> Database::getAllUsers()
{
    if (m_useDemoData)
    {
        return m_demoUsers;
    }

    QList<UserInfo> users;
    QSqlQuery query(m_db);
    // djust query to your actual schema
    query.prepare(R"(
        SELECT u.id, u.username, u.status, u.last_seen, u.ip_address,
               (SELECT COUNT(*) FROM messages m WHERE m.from_user_id = u.id) as msg_count
        FROM users u
        ORDER BY u.id
    )");

    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        qWarning() << "getAllUsers error:" << m_lastError;
        return users;
    }

    while (query.next())
    {
        UserInfo u;
        u.id = query.value(0).toInt();
        u.username = query.value(1).toString();
        u.status = query.value(2).toString();
        u.lastSeen = query.value(3).toDateTime();
        u.ipAddress = query.value(4).toString();
        u.messagesCount = query.value(5).toInt();
        users.append(u);
    }
    return users;
}

bool Database::banUser(int userId)
{
    if (m_useDemoData)
    {
        for (auto &u : m_demoUsers)
        {
            if (u.id == userId)
            {
                u.status = "banned";
                return true;
            }
        }
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET status = 'banned' WHERE id = :id");
    query.bindValue(":id", userId);
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool Database::unbanUser(int userId)
{
    if (m_useDemoData)
    {
        for (auto &u : m_demoUsers)
        {
            if (u.id == userId)
            {
                u.status = "offline";
                return true;
            }
        }
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET status = 'offline' WHERE id = :id");
    query.bindValue(":id", userId);
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool Database::setUserStatus(int userId, const QString &status)
{
    if (m_useDemoData)
    {
        for (auto &u : m_demoUsers)
        {
            if (u.id == userId)
            {
                u.status = status;
                return true;
            }
        }
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET status = :status WHERE id = :id");
    query.bindValue(":status", status);
    query.bindValue(":id", userId);
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

UserInfo Database::getUserById(int userId)
{
    if (m_useDemoData)
    {
        for (const auto &u : m_demoUsers)
        {
            if (u.id == userId) return u;
        }
        return UserInfo{};
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT u.id, u.username, u.status, u.last_seen, u.ip_address,
               (SELECT COUNT(*) FROM messages m WHERE m.from_user_id = u.id)
        FROM users u WHERE u.id = :id
    )");
    query.bindValue(":id", userId);
    if (query.exec() && query.next())
    {
        UserInfo u;
        u.id = query.value(0).toInt();
        u.username = query.value(1).toString();
        u.status = query.value(2).toString();
        u.lastSeen = query.value(3).toDateTime();
        u.ipAddress = query.value(4).toString();
        u.messagesCount = query.value(5).toInt();
        return u;
    }
    return UserInfo{};
}

QList<MessageInfo> Database::getAllMessages()
{
    if (m_useDemoData)
    {
        return m_demoMessages;
    }

    QList<MessageInfo> messages;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT m.id, fu.username, COALESCE(tu.username, 'ALL'), m.is_private, m.content, m.created_at
        FROM messages m
        JOIN users fu ON fu.id = m.from_user_id
        LEFT JOIN users tu ON tu.id = m.to_user_id
        ORDER BY m.created_at DESC
    )");

    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return messages;
    }

    while (query.next())
    {
        MessageInfo m;
        m.id = query.value(0).toInt();
        m.fromUser = query.value(1).toString();
        m.toUser = query.value(2).toString();
        m.isPrivate = query.value(3).toBool();
        m.content = query.value(4).toString();
        m.timestamp = query.value(5).toDateTime();
        messages.append(m);
    }
    return messages;
}

QList<MessageInfo> Database::getMessagesByUser(const QString &username)
{
    QList<MessageInfo> all = getAllMessages();
    QList<MessageInfo> filtered;
    for (const auto &m : all)
    {
        if (m.fromUser == username || m.toUser == username)
            filtered.append(m);
    }
    return filtered;
}

bool Database::deleteMessage(int messageId)
{
    if (m_useDemoData)
    {
        for (int i = 0; i < m_demoMessages.size(); ++i)
        {
            if (m_demoMessages[i].id == messageId)
            {
                m_demoMessages.removeAt(i);
                return true;
            }
        }
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);
    if (!query.exec())
    {
        m_lastError = query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}
