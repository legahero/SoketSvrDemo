#ifndef SOKETSVRMAINWINDOW_H
#define SOKETSVRMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SoketSvrMainWindow; }
QT_END_NAMESPACE

class SoketSvrMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    SoketSvrMainWindow(QWidget *parent = nullptr);
    ~SoketSvrMainWindow();

private slots:
    void on_pbStart_clicked();
    void OnTimer();
    void on_pbSend_clicked();

private:
    Ui::SoketSvrMainWindow *ui;
};
#endif // SOKETSVRMAINWINDOW_H
