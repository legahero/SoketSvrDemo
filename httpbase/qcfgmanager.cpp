#include "qcfgmanager.h"

QMutex QCfgManager::mutex;
QHash<QString,QConnectPool*>  QCfgManager::m_DbList;

QCfgManager::QCfgManager(QObject *parent) : QObject(parent)
{
    QCoreApplication::setOrganizationName("tsky");
    QCoreApplication::setOrganizationDomain("tsky.com");
    QCoreApplication::setApplicationName("TranServer");
    //QSettings settings(QSettings::SystemScope, "tsky", "TranServer");


    LoadDbCfg();
}

QString QCfgManager::GetCommCfg(QString key,QString defaultValue)
{
    QString ret="";
    QSettings settings;
    settings.beginGroup("Common");
    if(settings.contains(key))
    {
        ret=settings.value(key,defaultValue).toString();
    }else
    {
        ret=defaultValue;
        settings.setValue(key,defaultValue);
    }
    settings.endGroup();
    return ret;
}
void QCfgManager::WriteCommCfg(QString key,QString Value)
{
    QSettings settings;
    settings.beginGroup("Common");
    settings.setValue(key,Value);
    settings.endGroup();

    settings.sync();

}

void QCfgManager::LoadDbCfg()
{
    QSettings settings;
    settings.beginGroup("Db");
    QStringList groups = settings.childGroups();
    int size =groups.size();
    settings.endGroup();

    if(size>0)
    {
        for(int i = 0; i < size; i++)
        {
            settings.beginGroup(groups[i]);

            /*
            hostName     = "localhost";//主机名
            databaseName = "quick";//需要访问的数据库
            username     = "root";//用户名
            password     = "****";//密码
            databaseType = "QMYSQL";//数据库类型
            */

            QHash<QString,QVariant> param ;
            param["host"]=settings.value("host","localhost").toString();
            param["database"]=settings.value("database","").toString();
            param["username"]=settings.value("username","root").toString();
            param["password"]=settings.value("password","").toString();
            param["dbtype"]=settings.value("dbtype","QMYSQL").toString();
            //缺省1000个连接
            param["maxConnectionCount"]=settings.value("maxConnectionCount","1000").toInt();
            settings.endGroup();

            QConnectPool* db=new QConnectPool(param);

            if(!m_DbList.contains(groups[i]))
            {
                m_DbList[groups[i]]=db;
            }
        }
    }

}
void QCfgManager::WriteDbCfg(QHash<QString,DbCfg> list)
{
    QSettings settings;
    QHash<QString,DbCfg>::const_iterator item;

    for (item = list.constBegin(); item != list.constEnd(); ++item)
    {
        DbCfg cfg=item.value();
        settings.setValue("Db/"+item.key()+"hostName", cfg.hostName);
        settings.setValue("Db/"+item.key()+"username", cfg.username);
        settings.setValue("Db/"+item.key()+"password", cfg.password);
        settings.setValue("Db/"+item.key()+"database", cfg.dbname);
        settings.setValue("Db/"+item.key()+"dbtype", cfg.dbtype);
    }

}
QCfgManager::~QCfgManager()
{
    QHash<QString,QConnectPool*>::const_iterator i;
    for (i = m_DbList.constBegin(); i != m_DbList.constEnd(); ++i)
    {
        i.value()->deleteLater();
    }
}

QConnectPool* QCfgManager::getDb(QString DbName)
{
    QMutexLocker locker(&mutex);
    if(m_DbList.contains(DbName))
        return m_DbList[DbName];
    else
        return NULL;
}
