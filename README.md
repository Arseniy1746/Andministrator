#Messenger Server Admin Panel (Qt)

## Возможности

### Пользователи
- Просмотр списка всех пользователей (ID, имя, статус, last seen, IP, количество сообщений)
- Фильтрация по статусу (Online / Offline / Banned)
- Поиск по имени или ID
- **Отключение** пользователя (force disconnect)
- **Бан** и **разбан** пользователей

### Сообщения
- Просмотр **всех** сообщений, включая **приватные**
- Фильтр: все / только публичные / только приватные
- Фильтр по пользователю
- Поиск по содержимому
- Удаление сообщений

### Прочее
- Журнал действий администратора
- Авто-обновление данных каждые 30 секунд
- Поддержка PostgreSQL + автоматический fallback на demo-данные
- Современный и удобный интерфейс (QSS)

## Сборка

## База данных (PostgreSQL)

Пример:

```sql
CREATE TABLE users (
    id          SERIAL PRIMARY KEY,
    username    VARCHAR(64) UNIQUE NOT NULL,
    status      VARCHAR(16) DEFAULT 'offline',  -- online / offline / banned
    last_seen   TIMESTAMP,
    ip_address  VARCHAR(45)
);

CREATE TABLE messages (
    id            SERIAL PRIMARY KEY,
    from_user_id  INTEGER REFERENCES users(id),
    to_user_id    INTEGER REFERENCES users(id),  -- NULL = public
    is_private    BOOLEAN DEFAULT false,
    content       TEXT NOT NULL,
    created_at    TIMESTAMP DEFAULT NOW()
);
```

Если PostgreSQL недоступен, приложение автоматически работает с встроенными demo-данными.

Подключение к БД можно настроить через меню **Settings → Database Connection...**.

## Структура проекта

```
ServerAdmin/
├── ServerAdmin.pro
├── main.cpp
├── mainwindow.h / .cpp / .ui
├── database.h / .cpp          # Работа с БД + demo data
├── usermanager.h / .cpp       # Симуляция онлайн-сессий
├── resources.qrc
└── README.md
```

## Примечания

- `UserManager` сейчас симулирует disconnect. 
