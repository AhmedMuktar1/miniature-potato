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
#include "simplecrypt.h"

QString Username = "default";
int FriendCount = 0;
QString Users[50][2];
SimpleCrypt crypto(5346);
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEditHost->setEnabled(false);
    ui->spinBoxPort->setEnabled(false);
    ui->ConnectButton->setEnabled(false);
    ui->sendMessage->setEnabled(false);
    ui->AddRecipient->setEnabled(false);
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
            addFriend(sender);
        } else if(secondMessage == "Friend Yes"){
            addFriend(sender);
        }
        const QString content = QDateTime::currentDateTime().toString()
                + QLatin1String(" Received Topic: ")
                + topic.name()
                + QLatin1String(" Message: ")
                + message
                + QLatin1Char('\n');
        //ui->plainTextEdit->insertPlainText(content);
        QString e = message;
        QString Sender = e.section(":",0,0);
        QString MessageIn = e.section(":",1,1);
        WTFile(Sender,Sender,MessageIn);
        if (sender == ui->recipientTitle->text()){UpdateChat(sender);}
    });

    connect(m_client, &QMqttClient::pingResponseReceived, this, [this]() {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        //ui->plainTextEdit->insertPlainText(content);
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
    //ui->plainTextEdit->insertPlainText(content);
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
        m_client->connectToHost();
        ui->lineEditHost->setEnabled(false);
        ui->spinBoxPort->setEnabled(false);
        ui->ConnectButton->setText("Disconnect");
        delay(1);
        ConnectMe();
        ui->AddRecipient->setEnabled(true);
        if(ui->recipientTitle->text() != "") {ui->sendMessage->setEnabled(true);}
    } else {
        m_client->disconnectFromHost();
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->ConnectButton->setText("Connect");
        ui->AddRecipient->setEnabled(false);
        ui->sendMessage->setEnabled(false);
    }
}

void MainWindow::on_loginButton_clicked()
{
    QString UserName;
    QString Password;
    QString username = ui->loginUsernameInput->text();
    QString password = ui->loginPasswordInput->text();

    QFile file("//home//ntu-user//SDIChatApplication//userdata.txt");
    if ((username == "") & (password == "")) {
        QMessageBox::warning(this, "Login", "Username and password incorrect!");
    }
    else {
        if(!file.open(QFile::ReadOnly | QFile::Text)){
            QMessageBox::warning(this, "Warning", "File not open.");
        }
            QTextStream in(&file);
            QString line;
            while(!in.atEnd()){
                QString UserName = crypto.decryptToString(in.readLine());
                QString Password = crypto.decryptToString(in.readLine());
                int x = QString::compare(username, UserName, Qt::CaseSensitive);
                int y = QString::compare(password, Password, Qt::CaseSensitive);
                if(x == 0 && y == 0) {
                    ui->loginPasswordInput->setEnabled(false);
                    ui->loginUsernameInput->setEnabled(false);
                    ui->loginButton->setEnabled(false);
                    ui->signInButton->setEnabled(false);
                    ui->lineEditHost->setEnabled(true);
                    ui->spinBoxPort->setEnabled(true);
                    ui->ConnectButton->setEnabled(true);
                    QMessageBox::information(this, "Login", "Username and password is correct");
                    Username = username;
                    QString answer = readAllFromFile(1,Username+"FriendsList.txt");
                    for (int i=0;i<answer.section(":",0,0);i++) {
                        Users[i][0] = answer.section(":",i+1,i+1);
                        ui->comboBox->addItem(Users[i][0]);
                    }
                }
            }
            file.close();
    }
}

void MainWindow::on_AddRecipient_clicked()
{//*********************************** change to not accept null input
    QString recipient = ui->AddRecipientlineEdit->text();
    int num = 0;
    if(recipient != "" ){
        for (int i = 0;i<=FriendCount;i++) {
            if(recipient == Users[i][0]){
                QMessageBox::warning(this, "Add Friend", "You are already friends with " + recipient);
                num = 1;
            }
        }
        if(num == 0){
            Recipient = ui->AddRecipientlineEdit->text();
            publish(Recipient,Username+":" + "Friend?");
        }

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

        WTFile(Username+"FriendsList","updateFriendsList",username);
    }
    updateUIOnline();
}

void MainWindow::WTFile(QString Chatname, QString username, QString message){
    filehandler file;
    QString answer;
    answer = writeToFile(Chatname+".txt",username,message);
    if (answer == "File Opening Error"){
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not open file"));
    }
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

void MainWindow::updateUIOnline(QString sender){
    for (int i = 0;i <= FriendCount;i++) {
        ui->comboBox->addItem(sender);
    }
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    ui->recipientTitle->setText(ui->comboBox->currentText());
    UpdateChat(ui->recipientTitle->text());
    if(ui->recipientTitle->text() != "") {ui->sendMessage->setEnabled(true);}
}

void MainWindow::on_sendMessage_clicked()
{
    publish(ui->recipientTitle->text(),Username+":"+ui->messageInput->text());
    WTFile(ui->recipientTitle->text(),Username,ui->messageInput->text());
    UpdateChat(ui->recipientTitle->text());
}

void MainWindow::UpdateChat(QString recipient){
    QString OldMessages = readAllFromFile(2,recipient);
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setPlainText(OldMessages);
    ui->plainTextEdit->setCenterOnScroll(true);
    ui->plainTextEdit->ensureCursorVisible();
}

void MainWindow::delay(int n)
{
    QTime dieTime= QTime::currentTime().addSecs(n);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::on_signInButton_clicked()
{
    QString SignupUsername = ui->loginUsernameInput->text();
    QString SignupPassword = ui->loginPasswordInput->text();
    if(SignupUsername.length()>=4 && SignupPassword.length()>=4){
        QString usernameEncrypted =  crypto.encryptToString(SignupUsername);
        QString passwordEncrypted =  crypto.encryptToString(SignupPassword);
        WTFile("userdata","Signup",usernameEncrypted+":"+passwordEncrypted);
        QFile file("//home//ntu-user//SDIChatApplication//"+SignupUsername+"FriendsList.txt");
        file.open(QIODevice::Append | QIODevice::Text);
        file.close();
        QMessageBox::information(this, "Sign Up", "congratulations, You've been signed up!");
    }else {
        QMessageBox::warning(this,"Sign Up Error", "Username and passowrd must be over 4 characters");
    }
}
