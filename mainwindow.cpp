#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "filehandler.h"
#include <QtCore/QDateTime>
#include <QtMqtt/QMqttClient>
#include <QtWidgets/QMessageBox>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include "mainhub.h"
QString Username = "default";
int FriendCount = 0;
QString Users[50][2];
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->loginUsernameInput->setEnabled(false);
    ui->loginPasswordInput->setEnabled(false);

    m_client = new QMqttClient(this);
    m_client->setHostname(ui->lineEditHost->text());
    m_client->setPort(ui->spinBoxPort->value());

    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
    connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        QString Message = message;
        QString sender = Message.section(":", 0, 0);
        QString secondMessage = Message.section(":", 1, 1);
        if(secondMessage == "online?"){
            publish(sender,Username+":"+"I am 0nline:");
        } else if (secondMessage == "I am online"){
            updateOnline(sender,1);
        } else if (secondMessage == "I am going offline"){
            updateOnline(sender,0);
        } else if(secondMessage == "Friend?"){
            //open box to ask to be friends *******************************************
            publish(sender,Username+":Friend Yes");
        } else if(secondMessage == "Friend Yes"){
            addFriend(sender);
        }
        const QString content = QDateTime::currentDateTime().toString()
                + QLatin1String(" Received Topic: ")
                + topic.name()
                + QLatin1String(" Message: ")
                + message
                + QLatin1Char('\n');
        ui->plainTextEdit->insertPlainText(content);
        //writeToFile(message,message,content);
    });

    connect(m_client, &QMqttClient::pingResponseReceived, this, [this]() {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        ui->plainTextEdit->insertPlainText(content);
        //writeToFile("ping","ping","ping recived");
    });

    connect(ui->lineEditHost, &QLineEdit::textChanged, m_client, &QMqttClient::setHostname);
    connect(ui->spinBoxPort, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::setClientPort);
    updateLogStateChange();
}

QString Recipient = "default";
QString Message = "default";
QString Topic = "default";

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetDetails(int num,QString set){
    if(num == 0){
        Username = set;
    }else if(num == 1){
        Recipient = set;
    }else if(num == 2){
        Topic = set;
    }else if(num == 3){
        Message = set;
    }
};

void MainWindow::ConnectMe(){
    auto subscription = m_client->subscribe(Username);
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

void MainWindow::publish(QString topic,QString message){
    if (m_client->publish(topic, message.toUtf8()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}

void MainWindow::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client->state())
                    + QLatin1Char('\n');
    ui->plainTextEdit->insertPlainText(content);
    //writeToFile(Username,Username,content);
}

void MainWindow::brokerDisconnected()
{
    ui->lineEditHost->setEnabled(true);
    ui->spinBoxPort->setEnabled(true);
    ui->ConnectButton->setText(tr("Connect"));
}

void MainWindow::setClientPort(int p)
{
    m_client->setPort(p);
}


void MainWindow::on_ConnectButton_clicked()
{
    if (m_client->state() == QMqttClient::Disconnected) {
        ui->lineEditHost->setEnabled(false);
        ui->spinBoxPort->setEnabled(false);
        ui->loginUsernameInput->setEnabled(true);
        ui->loginPasswordInput->setEnabled(true);
        ui->ConnectButton->setText("Disconnect");
        m_client->connectToHost();
    } else {
        m_client->disconnectFromHost();
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->loginUsernameInput->setEnabled(false);
        ui->loginPasswordInput->setEnabled(false);
        ui->ConnectButton->setText("Connect");
    }
}

void MainWindow::on_loginButton_clicked()
{
    QString UserName;
    QString Password;
    QString username = ui->loginUsernameInput->text();
    QString password = ui->loginPasswordInput->text();

    //QFile file("userdata.txt");
    //if ((username == "") & (password == "")) {
    //    QMessageBox::warning(this, "Login", "Username and password incorrect!");
    //}
    //else {
    //    if(!file.open(QFile::ReadOnly | QFile::Text)){
    //        QMessageBox::warning(this, "Warning", "File not open.");
    //    }
    if (username == "Mukki" || "Ibby"){
//            QTextStream in(&file);
//            QString line;
//            while(!in.atEnd()){
//                QString UserName = in.readLine();
//                QString Password = in.readLine();
//                QMessageBox::information(this, UserName, Password);
//                int x = QString::compare(username, UserName, Qt::CaseSensitive);
//                int y = QString::compare(password, Password, Qt::CaseSensitive);
//                if(x == 0 && y == 0) {
                    QMessageBox::information(this, "Login", "Username and password is correct");
                    Username = username;
                    ConnectMe();
//            }
//            file.close();
//        }
    }
}

//void MainWindow::on_pushButton_clicked()
//{
//    Username = "Mukki";
//    ConnectMe();
//}

//void MainWindow::on_pushButton_2_clicked()
//{
//    publish(Username,"yay");
//}

void MainWindow::on_AddRecipient_clicked()
{//*********************************** change to not accept null input
    if(ui->AddRecipientlineEdit->text() != " " ){
        Recipient = ui->AddRecipientlineEdit->text();
        publish(Recipient,Username+":" + "Friend?");
    }
}

void MainWindow::addFriend(QString username){
    int num = 0;
    for (int  i = 0; i <= FriendCount;i++) {
        if(Users[i][1] == username){
            num = 1;
        }
    }
    if (num == 0){
        Users[FriendCount][0] = username;
        Users[FriendCount][1] = "Online";
        FriendCount++;
    }
    updateUIOnline();
}

void MainWindow::updateOnline(QString username, int num){
    if(num == 1){
        for (int i = 0;i <=FriendCount;i++) {
            if(Users[i][0] == username){
                Users[i][1] = "online";
            }
        }
    } else{
        for (int i = 0;i <=FriendCount;i++) {
            if(Users[i][0] == username){
                Users[i][1] = "offline";
            }
        }
    }
    //need to update ui **********************************************************

}

void MainWindow::updateUIOnline(){
    for (int i = 0;i <= FriendCount;i++) {
        ui->plainTextEdit->insertPlainText((Users[i][0]+" ... "+Users[i][1]));
    }
}
