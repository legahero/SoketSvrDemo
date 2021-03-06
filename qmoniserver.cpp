﻿#include "qmoniserver.h"
#include "httpbase/threadhandle.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "httpbase/qcfgmanager.h"

QMoniServer::QMoniServer(QObject *parent,int numConnections) :
    QTcpServer(parent)
{
     m_ClientList = new  QHash<int,QAsynTcpSocket *>;
     setMaxPendingConnections(numConnections);
}

QMoniServer::~QMoniServer()
{
    this->clear();
    delete m_ClientList;
}

void QMoniServer::setMaxPendingConnections(int numConnections)
{
    this->QTcpServer::setMaxPendingConnections(numConnections);//调用Qtcpsocket函数，设置最大连接数，主要是使maxPendingConnections()依然有效
    this->maxConnections = numConnections;
}

void QMoniServer::incomingConnection(qintptr socketDescriptor) //多线程必须在此函数里捕获新连接
{
    qDebug() << "QAsynTcpServer:incomingConnection" ;
    //继承重写此函数后，QQAsynTcpServer默认的判断最大连接数失效，自己实现
    if (m_ClientList->size() > maxPendingConnections())
    {
        QTcpSocket tcp;
        tcp.setSocketDescriptor(socketDescriptor);
        tcp.disconnectFromHost();
        return;
    }
    auto th = ThreadHandle::getClass().getThread();
    auto tcpTemp = new QAsynTcpSocket(socketDescriptor);
    QString ip =  tcpTemp->peerAddress().toString();
    qint16 port = tcpTemp->peerPort();

    //NOTE:断开连接的处理，从列表移除，并释放断开的Tcpsocket，此槽必须实现，线程管理计数也是靠的他
    connect(tcpTemp,&QAsynTcpSocket::sockDisConnect,this,&QMoniServer::sockDisConnectSlot);

    //必须在QAsynTcpSocket的线程中执行    
    connect(tcpTemp, SIGNAL(newRequest(QAsynTcpSocket*,const QByteArray&)), this,SLOT(handleRequest(QAsynTcpSocket*,const QByteArray&)), Qt::DirectConnection);

    tcpTemp->moveToThread(th);//把tcp类移动到新的线程，从线程管理类中获取
    m_ClientList->insert(socketDescriptor,tcpTemp);//插入到连接信息中

}

//释放线程资源
void QMoniServer::sockDisConnectSlot(int handle,const QString & ip, quint16 prot,QThread * th)
{
    m_ClientList->remove(handle);//连接管理中移除断开连接的socket
    ThreadHandle::getClass().removeThread(th); //告诉线程管理类那个线程里的连接断开了,释放数量
}


void QMoniServer::clear()
{
    ThreadHandle::getClass().clear();//清除所有线程

    //切断所有连接，并且清除
    for (auto it = m_ClientList->begin(); it != m_ClientList->end(); ++it)
    {
        if(it.value()!=NULL)
        {

            QAsynTcpSocket* obj=it.value();
            obj->disconnectFromHost();
        }
    }
    m_ClientList->clear();
}

/*处理新的tcp 请求，这里处理业务*/
void QMoniServer::handleRequest(QAsynTcpSocket* from,const QByteArray &arr)
{
    qDebug() << "QMoniServer:handleRequest,ThreadId:"<<QThread::currentThreadId();
    //QObject* obj=sender();
    //QAsynTcpSocket* form=dynamic_cast<QAsynTcpSocket*>(sender());

    rapidxml::xml_document<char> doc;
    QString body = QString(arr);
    qDebug() <<"QMoniServer::handleRequest:"<< body;
    exchangeMsg.enqueue("Recv:  "+body);
    try {
        doc.parse<0>(const_cast<char *>(body.toStdString().c_str()));

        rapidxml::xml_node<char> * node = doc.first_node("node name");
        if(node!=NULL)
        {
            std::string node_val = node->value();
            /*
            for(rapidxml::xml_node<char> * node = parent_node->first_node("node name");
                node != NULL; node = node->next_sibling())
            {

            }
            */
        }
    } catch (rapidxml::parse_error &e) {
        qDebug() <<"QMoniServer::handleRequest: rapidxml error,"<< e.what();
    }

    if(from==0)
    {
       return;
    }else
    {
        QString resp="OK";
        if(body.contains("<envelope>"))
        {
            //在这里写自己的自动应答代码
            //resp.append(...);
        }


        from->write(resp.toUtf8());//应答数据
        exchangeMsg.enqueue("Resp:  "+resp);

        //form->waitForBytesWritten();
        if(!from->flush())
        {
            qDebug() <<"QMoniServer::handleRequest: resp error,flush data fail!! ";
        }
    }

}
QString QMoniServer::GetExchangeMsg()
{
    if(!exchangeMsg.isEmpty())
        return exchangeMsg.dequeue();
    else
        return NULL;
}

void  QMoniServer::SendMsg(QString msg)
{
    qDebug() <<"SendMsg:"<<msg;
    exchangeMsg.enqueue("SendMsg:"+msg);
    for (auto it = m_ClientList->begin(); it != m_ClientList->end(); ++it)
    {
        if(it.value()!=NULL)
        {
            QAsynTcpSocket* obj=it.value();
            try {

                obj->write(msg.toUtf8());
                obj->flush();
            } catch (...) {
                exchangeMsg.enqueue("SendMsg err:  "+obj->errorString()+",error no:"+obj->error());
            }

        }
    }
    qDebug() <<"SendMsg  over!";
}
