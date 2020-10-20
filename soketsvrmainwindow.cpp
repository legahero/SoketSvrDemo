#include "soketsvrmainwindow.h"
#include "ui_soketsvrmainwindow.h"
#include "qmoniserver.h"
#include <QTimer>

SoketSvrMainWindow::SoketSvrMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SoketSvrMainWindow)
{
    ui->setupUi(this);
}
QMoniServer *server=NULL;
QTimer* pTimer=NULL;
SoketSvrMainWindow::~SoketSvrMainWindow()
{
    if(pTimer!=NULL)
    {
        if(pTimer->isActive())
            pTimer->stop();
        delete pTimer;
    }

    if(server!=NULL)
    {
        server->close();
    }
    delete server;

    delete ui;
}


void SoketSvrMainWindow::on_pbStart_clicked()
{    
    if(server==NULL) server = new QMoniServer(this);
    if(server->listen(QHostAddress::Any, ui->lePort->text().toUInt() ))
    {
        ui->teLog->append("tcp服务已经启动!");
        ui->pbStart->setDisabled(true);

        pTimer=new QTimer(this);
        connect(pTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));
        pTimer->start(2000);

    }else
    {
        ui->teLog->append("tcp服务启动失败!");
    }

}

void SoketSvrMainWindow::OnTimer()
{
    QString msg=server->GetExchangeMsg();
    if(msg!=NULL&&!msg.isEmpty())
    {
        ui->teLog->append(msg);
    }
}

void SoketSvrMainWindow::on_pbSend_clicked()
{
    QString sendText=ui->teSend->toPlainText();
    if(server!=NULL&&!sendText.isEmpty())
    {
        server->SendMsg(sendText);
    }
}
