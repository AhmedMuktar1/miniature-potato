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
    void updateUIOnline();
private:
    Ui::MainWindow *ui;
    QMqttClient *m_client;
    mainhub *Mainhub;

};
#endif // MAINWINDOW_H
