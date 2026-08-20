#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QSet>
#include <QString>

// Simulates active connections / sessions on the server side.
// In a real system this would talk to the actual messenger server process
// (via shared memory, sockets, Redis, etc.).
class UserManager : public QObject
{
    Q_OBJECT
public:
    explicit UserManager(QObject *parent = nullptr);

    void setOnline(int userId);
    void setOffline(int userId);
    bool isOnline(int userId) const;
    bool disconnectUser(int userId);   //force disconnect
    QSet<int> onlineUsers() const;

signals:
    void userDisconnected(int userId);
    void userStatusChanged(int userId, const QString &status);

private:
    QSet<int> m_onlineUsers;
};

#endif // USERMANAGER_H
