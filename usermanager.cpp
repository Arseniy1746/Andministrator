#include "usermanager.h"
#include <QDebug>

UserManager::UserManager(QObject *parent)
    : QObject(parent)
{
    //Some users start online for demo
    m_onlineUsers.insert(1);
    m_onlineUsers.insert(2);
    m_onlineUsers.insert(5);
    m_onlineUsers.insert(7);
}

void UserManager::setOnline(int userId)
{
    m_onlineUsers.insert(userId);
    emit userStatusChanged(userId, "online");
}

void UserManager::setOffline(int userId)
{
    m_onlineUsers.remove(userId);
    emit userStatusChanged(userId, "offline");
}

bool UserManager::isOnline(int userId) const
{
    return m_onlineUsers.contains(userId);
}

bool UserManager::disconnectUser(int userId)
{
    if (!m_onlineUsers.contains(userId))
    {
        return false; //already offline
    }
    m_onlineUsers.remove(userId);
    qDebug() << "Force disconnected user" << userId;
    emit userDisconnected(userId);
    emit userStatusChanged(userId, "offline");
    return true;
}

QSet<int> UserManager::onlineUsers() const
{
    return m_onlineUsers;
}
