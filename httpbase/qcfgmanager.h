#ifndef QCFGMANAGER_H
#define QCFGMANAGER_H

#include <QObject>
#include <QHash>
#include "qconnectpool.h"

/* 配置管理器
 * 加载配置，写配置
 * 加载多个数据库配置
 * */

class DbCfg {
public:
    QString hostName;
    QString dbname;
    QString username;
    QString password;
    QString dbtype;
};

class QCfgManager : public QObject
{
    Q_OBJECT
public:
    explicit QCfgManager(QObject *parent = 0);
    ~QCfgManager();

    QString GetCommCfg(QString key,QString defaultValue);
    void WriteCommCfg(QString key,QString Value);

    void LoadDbCfg();
    void WriteDbCfg(QHash<QString,DbCfg> list);
    //根据数据库分组名DbName获取该数据库连接池
    static QConnectPool* getDb(QString DbName);
signals:

public slots:

private:
    static QHash<QString,QConnectPool*> m_DbList;

    static QMutex mutex;
};

#endif // QCFGMANAGER_H
