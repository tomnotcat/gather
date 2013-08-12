/*
 * Copyright (C) 2013 Tom Wong. All rights reserved.
 */
#include "gtusermanager_p.h"
#include "gtuserclient.h"
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtNetwork/QHostAddress>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#define MAX_ACCOUNT 5

GT_BEGIN_NAMESPACE

class GtUserManagerPrivate
{
    Q_DECLARE_PUBLIC(GtUserManager)

public:
    GtUserManagerPrivate(GtUserManager *q, QThread *t);
    ~GtUserManagerPrivate();

public:
    void openRegDatabase(const QString &regdb);

public:
    static QString makeCheckCode(const QString &usr, const QString &pwd);

public:
    GtUserManager *q_ptr;
    GtUserManagerProxy *m_proxy;
    QThread *m_thread;
    GtUserClient *m_client;
    QString m_session;
    int m_state;

    QSqlDatabase m_regDatabase;
    QList<GtUserManager::Account> m_accounts;
    QMutex m_mutex;
};

GtUserManagerProxy::GtUserManagerProxy(GtUserManagerPrivate *p)
    : m_priv(p)
{
}

GtUserManagerProxy::~GtUserManagerProxy()
{
}

bool GtUserManagerProxy::connectToHost()
{
    GtUserClient *client = m_priv->m_client;
    QHostAddress host(QHostAddress::LocalHost);
    quint16 port = 8701;

    return client->connect(host, port);
}

void GtUserManagerProxy::login(const QString &username,
                               const QString &password,
                               bool rememberMe,
                               bool autoLogin)
{
    GtUserClient *client = m_priv->m_client;

    if (client->state() == GtUserClient::UnconnectedState) {
        if (!connectToHost()) {
            emit m_priv->q_ptr->loginError(client->error());
            client->clearError();
            return;
        }
    }

    if (!client->login(username, password)) {
        emit m_priv->q_ptr->loginError(client->error());
        client->clearError();
        return;
    }

    QMetaObject::invokeMethod(m_priv->q_ptr, "loginSuccess",
                              Q_ARG(QString, username),
                              Q_ARG(QString, password),
                              Q_ARG(bool, rememberMe),
                              Q_ARG(bool, autoLogin));
}

void GtUserManagerProxy::logout()
{
    GtUserClient *client = m_priv->m_client;
    client->logout();
}

void GtUserManagerProxy::stateChanged(int state)
{
    GtUserClient *client = m_priv->m_client;

    m_priv->m_mutex.lock();
    m_priv->m_state = state;
    m_priv->m_session = client->session();
    m_priv->m_mutex.unlock();

    emit m_priv->q_ptr->stateChanged(state, client->error());
    client->clearError();
}

GtUserManagerPrivate::GtUserManagerPrivate(GtUserManager *q, QThread *t)
    : q_ptr(q)
    , m_thread(t)
    , m_client(0)
    , m_state(GtUserClient::UnconnectedState)
{
    m_client = new GtUserClient();
    m_proxy = new GtUserManagerProxy(this);

    QObject::connect(m_client, SIGNAL(stateChanged(int)),
                     m_proxy, SLOT(stateChanged(int)));

    if (t) {
        m_client->moveToThread(t);
        m_proxy->moveToThread(t);
    }

    QMetaObject::invokeMethod(m_proxy, "connectToHost");
}

GtUserManagerPrivate::~GtUserManagerPrivate()
{
    delete m_proxy;
    delete m_client;
}

void GtUserManagerPrivate::openRegDatabase(const QString &regdb)
{
    // find sqlite driver
    m_regDatabase = QSqlDatabase::addDatabase("QSQLITE", "regdb");
    m_regDatabase.setDatabaseName(regdb);

    if (!m_regDatabase.open()) {
        qWarning() << "open registry database error:"
                   << m_regDatabase.lastError();
        return;
    }

    QSqlQuery query(m_regDatabase);

    // account table
    QString sql = "CREATE TABLE IF NOT EXISTS account "
                  "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "username VARCHAR(255), "
                  "password VARCHAR(255), "
                  "auto_login INTEGER, "
                  "check_code VARCHAR(255), "
                  "last_login TIMESTAMP)";

    if (!query.exec(sql))
        qWarning() << "create account table error:" << query.lastError();

    // load accounts
    sql = "SELECT * FROM account ORDER BY last_login DESC";
    if (query.exec(sql)) {
        if (query.first()) {
            int count = 0;

            do {
                QString username = query.value(1).toString();
                QString password = query.value(2).toString();
                int autoLogin = query.value(3).toInt();
                QString checkCode = query.value(4).toString();

                if (makeCheckCode(username, password) == checkCode) {
                    m_accounts.append(GtUserManager::Account(
                        username, password, !password.isEmpty(), autoLogin));
                    ++count;
                }
                else {
                    qWarning() << "invalid account check code:" << username;
                }
            }
            while (count < MAX_ACCOUNT && query.next());
        }
    }
    else {
        qWarning() << "query account table error:" << query.lastError();
    }
}

QString GtUserManagerPrivate::makeCheckCode(const QString &usr, const QString &pwd)
{
    return QString();
}

GtUserManager::GtUserManager(const QString &regdb,
                             QThread *thread,
                             QObject *parent)
    : QObject(parent)
    , d_ptr(new GtUserManagerPrivate(this, thread))
{
    if (!regdb.isEmpty())
        d_ptr->openRegDatabase(regdb);
}

GtUserManager::~GtUserManager()
{
}

QList<GtUserManager::Account> GtUserManager::accounts() const
{
    Q_D(const GtUserManager);
    return d->m_accounts;
}

void GtUserManager::login(const Account &account)
{
    Q_D(GtUserManager);

    QMetaObject::invokeMethod(d->m_proxy, "login",
                              Q_ARG(QString, account.username),
                              Q_ARG(QString, account.password),
                              Q_ARG(bool, account.rememberMe),
                              Q_ARG(bool, account.autoLogin));
}

void GtUserManager::logout()
{
    Q_D(GtUserManager);

    QMetaObject::invokeMethod(d->m_proxy, "logout");
}

QString GtUserManager::session() const
{
    QMutexLocker locker(&d_ptr->m_mutex);
    QString session(d_ptr->m_session);
    return session;
}

int GtUserManager::state() const
{
    QMutexLocker locker(&d_ptr->m_mutex);
    int state = d_ptr->m_state;
    return state;
}

void GtUserManager::loginSuccess(const QString &username,
                                 const QString &password,
                                 bool rememberMe,
                                 bool autoLogin)
{
    Q_D(GtUserManager);

    // update cache
    QList<Account>::iterator it;
    for (it = d->m_accounts.begin(); it != d->m_accounts.end(); ++it) {
        if ((*it).username == username) {
            d->m_accounts.erase(it);
            break;
        }
    }

    d->m_accounts.prepend(Account(username, password, rememberMe, autoLogin));
    if (d->m_accounts.size() > MAX_ACCOUNT)
        d->m_accounts.pop_back();

    // update database
    QSqlQuery query(d->m_regDatabase);
    query.prepare("SELECT username FROM account WHERE username=:username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qWarning() << "query account error:" << query.lastError();
        return;
    }

    if (query.first()) {
        query.prepare("UPDATE account SET password=:password, "
                      "auto_login=:auto_login, "
                      "check_code=:check_code, "
                      "last_login=datetime('now', 'localtime') "
                      "WHERE username=:username");
    }
    else {
        query.prepare("INSERT INTO account (username, password, "
                      "auto_login, check_code, last_login) "
                      "VALUES(:username, :password, :auto_login, "
                      ":check_code, datetime('now', 'localtime'))");
    }

    QString checkCode = d->makeCheckCode(username, password);
    query.bindValue(":username", username);

    if (rememberMe)
        query.bindValue(":password", password);
    else
        query.bindValue(":password", "");

    query.bindValue(":auto_login", autoLogin ? 1 : 0);
    query.bindValue(":check_code", checkCode);

    if (!query.exec()) {
        qWarning() << "update account error:" << query.lastError();
        return;
    }
}

GT_END_NAMESPACE
