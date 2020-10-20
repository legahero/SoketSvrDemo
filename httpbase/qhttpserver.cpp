/*
 * 来自www.github.com/legahero/HttpJsonServer的框架修改
  */

#include <QTcpSocket>
#include <QVariant>
#include <QDebug>
#include <QtSql>

#include "threadhandle.h"
#include "qasynhttpsocket.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpserver.h"
#include "staticfilecontroller.h"
#include "rapidxml-1.13/rapidxml.hpp"

//QHash<int, QString> STATUS_CODES;

QHttpServer::QHttpServer(QObject *parent,int numConnections) :
    QAsynTcpServer(parent,numConnections)
{    


}

QHttpServer::~QHttpServer()
{
}

void QHttpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "QHttpServer:incomingConnection,ThreadId:"<<QThread::currentThreadId()  ;

    //继承重写此函数后，QQAsynTcpServer默认的判断最大连接数失效，自己实现
    if (m_ClientList->size() > maxPendingConnections())
    {
        QTcpSocket tcp;
        tcp.setSocketDescriptor(socketDescriptor);        
        tcp.disconnectFromHost();
        qDebug() << "tcpClient->size() > maxPendingConnections(),disconnectFromHost";
        return;
    }
    auto th = ThreadHandle::getClass().getThread();
    QAsynHttpSocket* tcpTemp = new QAsynHttpSocket(socketDescriptor);
    QString ip =  tcpTemp->peerAddress().toString();
    qint16 port = tcpTemp->peerPort();

    //NOTE:断开连接的处理，从列表移除，并释放断开的Tcpsocket，线程管理计数减1,此槽必须实现
    connect(tcpTemp,SIGNAL(sockDisConnect(const int ,const QString &,const quint16, QThread *)),
            this,SLOT(sockDisConnectSlot(const int ,const QString &,const quint16, QThread *)));

    //必须在QAsynHttpSocket的线程中执行
    connect(tcpTemp, SIGNAL(newRequest(QHttpRequest *, QHttpResponse *)), this,
            SLOT(handleRequest(QHttpRequest *, QHttpResponse *)), Qt::DirectConnection);

    tcpTemp->moveToThread(th);//把tcp类移动到新的线程，从线程管理类中获取
    m_ClientList->insert(socketDescriptor,tcpTemp);//插入到连接信息中

    qDebug() << "QHttpServer m_ClientList add:"<<socketDescriptor  ;
}

//释放线程资源
void QHttpServer::sockDisConnectSlot(int handle,const QString & ip, quint16 prot,QThread * th)
{
    qDebug() << "QHttpServer:sockDisConnectSlot,ThreadId:"<<QThread::currentThreadId()  ;

    qDebug() << "QHttpServer m_ClientList size:"<<m_ClientList->size()  ;
    qDebug() << "QHttpServer m_ClientList:remove:"<<handle  ;
    m_ClientList->remove(handle);//连接管理中移除断开连接的socket
    ThreadHandle::getClass().removeThread(th); //告诉线程管理类那个线程里的连接断开了,释放数量
    qDebug() << "QHttpServer m_ClientList size:"<<m_ClientList->size()  ;
}


/*处理新的http 请求，这里处理业务*/
void QHttpServer::handleRequest(QHttpRequest *req, QHttpResponse *resp)
{
    qDebug() << "QHttpServer:handleRequest,ThreadId:"<<QThread::currentThreadId();


    qDebug() <<"path:"<< req->path()<<"body:"<<req->body();
    qDebug() <<"headers:"<< req->headers();

    if(req->path().indexOf('.')>=0)
    {
        QString configFileName=searchConfigFile();
        // Configure static file controller
        QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat);
        fileSettings->beginGroup("docroot");
        StaticFileController* staticFileController=new StaticFileController(fileSettings);

        //StaticFileController staticFileController(fileSettings);
        staticFileController->Handler(*req, *resp);
        return ;
    }

    /*
    QJsonDocument doc=QJsonDocument::fromBinaryData(req->body());
    QJsonObject recv_obj=doc.object();//这是接收到的json对象

    QConnectPool* dbpool=QMultiDbManager::getDb("sql2014");
    if(dbpool!=NULL)
    {
        QSqlDatabase db=dbpool->openSession();
        QSqlQuery query(db);
        query.exec("SELECT top 1 * FROM tLogin ");

        QJsonObject resp_obj; //返回json对象
        while (query.next()) {
            resp_obj.insert("LoginName",query.value("LoginName").toString());
            resp_obj.insert("Pwd", query.value("LoginName").toString());
        }
        dbpool->closeSession(db);

        QByteArray data = QJsonDocument(resp_obj).toJson(QJsonDocument::Compact);

        resp->setHeader("Content-Type", "text/html");
        resp->setHeader("Content-Length", QString::number(data.length()));
        resp->writeHead(200);
        resp->end(data);
    }else
    {
        qDebug() <<"get  QMultiDbManager::getDb fail";
        //resp->setHeader("Content-Type", "text/html");
        resp->writeHead(403);
    }
    */
    //转发数据
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest tranreq;

    QUrl url=req->url();
    QString path=url.path();
    QString query=url.query();

    QHash<QString,QVariant> param ;
    QString binDir=QCoreApplication::applicationDirPath();
    QString fileName("config.ini");
    QFile file(binDir+"/"+fileName);
    if (file.exists())
    {
        QString configFileName=binDir+"/"+fileName;
        QSettings settings(configFileName,QSettings::IniFormat);

        QStringList groups=settings.childGroups();
        for(int i = 0; i < groups.size(); i++)
        {
            settings.beginGroup(groups[i]);

            param["http_host"]=settings.value("http_host","localhost");
            param["http_port"]=settings.value("http_port","9100").toString();

        }
    }
    QString szUrl="";
    //req.setUrl(QUrl("http://192.168.1.135:8080/cgi-bin/appserver/CompanyAction?function=HttpLogin"));
    szUrl=szUrl.sprintf("http://%s:%s/",qPrintable(param["http_host"].toString()),qPrintable(param["http_port"].toString()));
    tranreq.setUrl(QUrl( szUrl));
    QNetworkReply *reply=NULL;
    if(req->method()==QHttpRequest::HttpMethod::HTTP_PUT)
    {
        reply=manager->post(tranreq, req->body());
    }else
    {
        reply=manager->get(tranreq);
    }

    QTimer timer;
    timer.setInterval(30000);  // 设置超时时间 30 秒
    timer.setSingleShot(true);  // 单次触发

    QByteArray responseData;
    QEventLoop eventLoop;
    connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    timer.start();
    eventLoop.exec();       //block until finish

    if (timer.isActive()) {  // 处理响应
        timer.stop();
        if (reply->error() != QNetworkReply::NoError) {
            // 错误处理
            qDebug() << "Error String : " << reply->errorString();
        } else {
            QVariant variant = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            int nStatusCode = variant.toInt();
            // 根据状态码做进一步数据处理
            //QByteArray bytes = reply->readAll();
            qDebug() << "Status Code : " << nStatusCode;
        }
    } else {  // 处理超时
        disconnect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        reply->abort();
        reply->deleteLater();
        qDebug() << "Timeout";
    }
    responseData = reply->readAll();
    if(responseData.length()>0)
    {
        //resp->setHeader("Content-Type", "text/html");
        resp->setHeader("Content-Length", QString::number(responseData.length()));
        resp->writeHead(200);
        resp->end(responseData);
    }else
    {
        resp->writeHead(403);
    }
    reply->deleteLater();
    delete manager;

    rapidxml::xml_document<char> doc;
    QString body = QString(req->body());
    doc.parse<0>(const_cast<char *>(body.toStdString().c_str()));
    rapidxml::xml_node<char> * node = doc.first_node("node name");
    std::string node_val = node->value();
    /*
    for(rapidxml::xml_node<char> * node = parent_node->first_node("node name");
        node != NULL; node = node->next_sibling())
    {

    }
    */

    resp->flush();

    req->deleteLater();
    resp->deleteLater();
    qDebug() <<"handleRequest end";

}

/** Search the configuration file */
QString QHttpServer::searchConfigFile()
{
    QString binDir=QCoreApplication::applicationDirPath();
    QString appName=QCoreApplication::applicationName();
    QString fileName(appName+".ini");

    QStringList searchList;
    searchList.append(binDir);
    searchList.append(binDir+"/etc");
    searchList.append(binDir+"/../etc");
    searchList.append(binDir+"/../../etc"); // for development without shadow build
    searchList.append(binDir+"/../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(QDir::rootPath()+"etc/opt");
    searchList.append(QDir::rootPath()+"etc");

    foreach (QString dir, searchList)
    {
        QFile file(dir+"/"+fileName);
        if (file.exists())
        {
            // found
            fileName=QDir(file.fileName()).canonicalPath();
            qDebug("Using config file %s",qPrintable(fileName));
            return fileName;
        }
    }

    // not found
    foreach (QString dir, searchList)
    {
        qWarning("%s/%s not found",qPrintable(dir),qPrintable(fileName));
    }
    //qFatal("Cannot find config file %s",qPrintable(fileName));
    return 0;
}
