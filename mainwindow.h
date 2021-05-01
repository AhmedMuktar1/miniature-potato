#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMqttClient>
#include "mainhub.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void publish(QString,QString);
    void setClientPort(int);
    void SetDetails(int num,QString set);
    void ConnectMe();

private slots:
    void updateLogStateChange();
    void brokerDisconnected();
    void on_ConnectButton_clicked();
    void on_loginButton_clicked();
    void addFriend(QString username);
    void updateOnline(QString username, int num);
    void on_AddRecipient_clicked();
    void updateGUIOnline();
    void WTFile(QString Chatname, QString username, QString message);
    void on_comboBox_activated(const QString &arg1);
    void UpdateChat(QString recipient);
    void on_sendMessage_clicked();
    void delay(int n);
    void on_signInButton_clicked();

    void on_Groupbutton_clicked();

    void on_CancelGroupChat_clicked();

    void on_addToGC_clicked();

    void on_RemoveGCRecipient_clicked();

    void on_shutdownButton_2_clicked();

    void on_CreateGroupChat_clicked();

    void on_comboBox_2_activated(const QString &arg1);
    void updateGC(int a);
    void Subscribe(QString);
private:
    Ui::MainWindow *ui;
    QMqttClient *m_client;
    mainhub *Mainhub;

};
#endif // MAINWINDOW_H
