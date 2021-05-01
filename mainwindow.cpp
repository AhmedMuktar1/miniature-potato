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
#include <QDebug>

QString Username = "default";
int FriendCount = 0;
QString Users[50][2];
QString GC[50][6];
SimpleCrypt crypto(5346);
int numinGC;

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
    ui->verticalWidget->setVisible(false);
    ui->Groupbutton->setEnabled(false);


    m_client = new QMqttClient(this);
    m_client->setHostname(ui->lineEditHost->text());
    m_client->setPort(ui->spinBoxPort->value());

    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
    connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        QString Message = message;
        qDebug() << "MESSAGE RECIVED: message " << Message;
        QString Topic = Message.section(":",0,0);
        QString sender = Message.section(":", 1, 1);
        QString secondMessage = Message.section(":", 2, 2);
        QString MessageToChatroom = Message.section(":",2,-1);
        QString ThirdMessagae = Message.section(":",3,-1);
        QString NewGCMessage = Message.section(":",2,-1);
        qDebug() << "MESSAGE RECIVED: NEW GC MESSAGE " << NewGCMessage;
        qDebug() << "MESSAGE RECIVED: Topic " << Topic;
        qDebug() << "MESSAGE RECIVED: sender " << sender;
        qDebug() << "MESSAGE RECIVED: secondMessage " << secondMessage;
        qDebug() << "MESSAGE RECIVED: ThirdMessage" << ThirdMessagae;

        if(sender == "online?"){
            publish(sender,Username+":"+"I am 0nline:");
        } else if (sender == "I am online"){
            updateOnline(Topic,1);
        } else if (sender == "I am going offline"){
            updateOnline(Topic,0);
        } else if(sender == "Friend?"){
            //open box to ask to be friends *******************************************
            publish(Topic,Username+":Friend Yes");
            addFriend(Topic);
            WTFile(Username+"Chatroom"+sender,"","");

        } else if(sender == "Friend Yes"){
            addFriend(Topic);
            WTFile(Username+"Chatroom"+sender,"","");

        }else if(sender == "New Group Chat"){
            WTFile(Username+"GC","newGC",NewGCMessage);
            WTFile(Username+"GC"+Message.section(":",2,2),"newGC","");
            ui->comboBox_2->addItem(Message.section(":",2,2));
            updateGC(1);
        }
        if(Topic == Username){
            qDebug() << "message recived for Chat with " << sender;
            WTFile(Username+"Chatroom"+sender,sender,MessageToChatroom);
            if (sender == ui->recipientTitle->text()){UpdateChat(Username+"Chatroom"+sender);}
        } else if (ui->comboBox_2->findText(Topic) != -1) {
            qDebug() << "Message recived for GC " << topic ;
            WTFile(Username + "GC"+Topic,sender,secondMessage);
            if (Topic == ui->recipientTitle->text()){UpdateChat(Username+"GC"+Topic);}
        }
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
        updateGC(0);
        ui->AddRecipient->setEnabled(true);
        if(ui->recipientTitle->text() != "") {ui->sendMessage->setEnabled(true);}
        ui->Groupbutton->setEnabled(true);
    } else {
        m_client->disconnectFromHost();
        ui->lineEditHost->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        ui->ConnectButton->setText("Connect");
        ui->AddRecipient->setEnabled(false);
        ui->sendMessage->setEnabled(false);
        ui->Groupbutton->setEnabled(false);
        ui->verticalWidget->setVisible(false);

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
                    int abc = answer.section(":",0,0).toInt();

                    for (int i=0;i<abc;i++) {
                        Users[i][0] = answer.section(":",i+1,i+1);
                        ui->comboBox->addItem(Users[i][0]);
                        FriendCount += 1;
                    }
                    updateGC(0);
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
    updateGUIOnline();
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

void MainWindow::updateGUIOnline(){
    for (int i = 0;i <= FriendCount;i++) {
        ui->comboBox->addItem(Users[i][0]);
    }
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    ui->recipientTitle->setText(arg1);
    UpdateChat(Username+"Chatroom"+arg1);
    if(ui->recipientTitle->text() != "") {ui->sendMessage->setEnabled(true);}
}

void MainWindow::on_sendMessage_clicked()
{
    QString title = ui->recipientTitle->text();
    qDebug() << "SendingMessage: sending message to " << title;
    QString messageToPublish = ui->recipientTitle->text()+":"+Username+":"+ui->messageInput->text();
    qDebug() << "SendingMessage: Message to publish" << messageToPublish;
    publish(ui->recipientTitle->text(),ui->recipientTitle->text()+":"+Username+":"+ui->messageInput->text());

    if(ui->comboBox->findText(title) != -1){
        qDebug() << "SendingMessage: updating chatroom";
        WTFile(Username+"Chatroom"+ui->recipientTitle->text(),Username,ui->messageInput->text());
        UpdateChat(Username+"Chatroom"+ui->recipientTitle->text());
    } else {
        qDebug() << "SendingMessage: updating GC";
        WTFile(Username+"GC"+ui->recipientTitle->text(),Username,ui->messageInput->text());
        UpdateChat(Username + "GC" +ui->recipientTitle->text());
    }
}

void MainWindow::UpdateChat(QString recipient){
    QString OldMessages = readAllFromFile(2,recipient);
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setPlainText(OldMessages);
    ui->plainTextEdit->setCenterOnScroll(true);
    ui->plainTextEdit->moveCursor(QTextCursor::End);
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

void MainWindow::on_Groupbutton_clicked()
{
   ui->verticalWidget->setVisible(true);
   for (int i=0;i<FriendCount;i++) {ui->comboBox_GC->addItem(Users[i][0]);}
}

void MainWindow::on_CancelGroupChat_clicked()
{
    ui->verticalWidget->setVisible(false);
}

void MainWindow::on_addToGC_clicked()
{
    bool pass = true;
    QString UserToAdd = ui->comboBox_GC->currentText();
    int UserToAddInt = ui->comboBox_GC->currentIndex();
    if(UserToAdd != ""){
        for (int i = 0;i>numinGC;i++) {
            if(UserToAdd == ui->comboBox_GCRR->itemText(i)){
                QMessageBox::warning(this,"Group Chat Error","This user has already been added");
                pass = false;
            }
        }
        if(pass == true){
            ui->comboBox_GCRR->addItem(UserToAdd);
            ui->comboBox_GC->removeItem(UserToAddInt);
            numinGC++;
        }
    }else {
        QMessageBox::warning(this,"Group Chat Error","Invalid Selection");
    }
}

void MainWindow::on_RemoveGCRecipient_clicked()
{
    QString UserToAdd = ui->comboBox_GCRR->currentText();
    int UserToAddInt = ui->comboBox_GCRR->currentIndex();
    QString updateSentence = "";
    if(UserToAdd != ""){
        ui->comboBox_GC->addItem(UserToAdd);
        ui->comboBox_GCRR->removeItem(UserToAddInt);
        numinGC--;
    }else {
        QMessageBox::warning(this,"Group Chat Error","Invalid Selection");
    }
}

void MainWindow::on_shutdownButton_2_clicked()
{
    exit(1);
}

void MainWindow::on_CreateGroupChat_clicked()
{
    int NumberOfRecipients = ui->comboBox_GCRR->count();
    if(ui->lineEditGCName->text() == ""){
        QMessageBox::warning(this,"Group Chat Error","Please Enter A Group Chat Name");
    } else if (NumberOfRecipients < 2 || NumberOfRecipients > 8){
        QMessageBox::warning(this,"Group Chat Error","There must be at least 2 and no more than 8 people to make a gc");
    } else if(ui->comboBox_2->findText(ui->lineEditGCName->text()) != -1){
        QMessageBox::warning(this,"Group Chat Error","This GroupChat already exists");
    } else {
        int num=0;
        QString CreateGCMessagae= "";
        for (int i=0;i<NumberOfRecipients;i++) {
            CreateGCMessagae = CreateGCMessagae + ui->comboBox_GCRR->itemText(i) + ",";
            num++;
        }
        for (int i=0;i<NumberOfRecipients;i++) {
            publish(ui->comboBox_GCRR->itemText(i),Username+":New Group Chat:"+ui->lineEditGCName->text()+":"+Username+":"+"0::" +QString::number(num)+":"+CreateGCMessagae);
        }
        WTFile(Username+"GC","newGC",ui->lineEditGCName->text()+":"+Username+":"+"0::" +QString::number(num)+":"+CreateGCMessagae);
        WTFile(Username+ui->lineEditGCName->text(),"newGC","");
        ui->comboBox_2->addItem(ui->lineEditGCName->text());
        updateGC(1);
    }
}

void MainWindow::on_comboBox_2_activated(const QString &arg1)
{
    ui->recipientTitle->setText(arg1);
    Subscribe(arg1);
    UpdateChat(Username+"GC"+arg1);
    ui->sendMessage->setEnabled(true);
    int a = ui->comboBox_2->currentIndex();
    ui->GCL1->setText("Admin: "+GC[a][1]);
    int AmountOfModerators = GC[a][2].toInt();
    if (AmountOfModerators == 1){
        ui->GCL9->setText("MOD:"+GC[a][3].section(",",0,0));
    } else if (AmountOfModerators == 2){
        ui->GCL9->setText("MOD:"+GC[a][3].section(",",0,0));
        ui->GCL8->setText("MOD:"+GC[a][3].section(",",1,1));
    } else if (AmountOfModerators == 3){
        ui->GCL9->setText("MOD:"+GC[a][3].section(",",0,0));
        ui->GCL8->setText("MOD:"+GC[a][3].section(",",1,1));
        ui->GCL7->setText("MOD:"+GC[a][3].section(",",2,2));
    }
    int b = 5;
    int AmountOfUsers = GC[a][4].toInt();
    if(AmountOfUsers == 8){
        ui->GCL9->setText(GC[a][b].section(",",7,7));
        ui->GCL8->setText(GC[a][b].section(",",6,6));
        ui->GCL7->setText(GC[a][b].section(",",5,5));
        ui->GCL6->setText(GC[a][b].section(",",4,4));
        ui->GCL5->setText(GC[a][b].section(",",3,3));
        ui->GCL4->setText(GC[a][b].section(",",2,2));
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }else if(AmountOfUsers == 7){
        ui->GCL8->setText(GC[a][b].section(",",6,6));
        ui->GCL7->setText(GC[a][b].section(",",5,5));
        ui->GCL6->setText(GC[a][b].section(",",4,4));
        ui->GCL5->setText(GC[a][b].section(",",3,3));
        ui->GCL4->setText(GC[a][b].section(",",2,2));
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }else if(AmountOfUsers == 6){
        ui->GCL7->setText(GC[a][b].section(",",5,5));
        ui->GCL6->setText(GC[a][b].section(",",4,4));
        ui->GCL5->setText(GC[a][b].section(",",3,3));
        ui->GCL4->setText(GC[a][b].section(",",2,2));
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }else if(AmountOfUsers == 5){
        ui->GCL6->setText(GC[a][b].section(",",4,4));
        ui->GCL5->setText(GC[a][b].section(",",3,3));
        ui->GCL4->setText(GC[a][b].section(",",2,2));
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }else if(AmountOfUsers == 4){
        ui->GCL5->setText(GC[a][b].section(",",3,3));
        ui->GCL4->setText(GC[a][b].section(",",2,2));
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }else if(AmountOfUsers == 3){
        ui->GCL4->setText(GC[a][b].section(",",2,2));
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }else if(AmountOfUsers == 2){
        ui->GCL3->setText(GC[a][b].section(",",1,1));
        ui->GCL2->setText(GC[a][b].section(",",0,0));
    }
}

void MainWindow::updateGC(int a){
    QString answer;
    int abc;
    answer = readAllFromFile(3,Username+"GC.txt");
    abc = answer.section(":",0,0).toInt();
    for (int i = 0;i<abc;i++) {
        GC[i*6][0] = answer.section(":",(i*6)+1,(i*6)+1);
        GC[i*6][1] = answer.section(":",(i*6)+2,(i*6)+2);
        GC[i*6][2] = answer.section(":",(i*6)+3,(i*6)+3);
        GC[i*6][3] = answer.section(":",(i*6)+4,(i*6)+4);
        GC[i*6][4] = answer.section(":",(i*6)+5,(i*6)+5);
        GC[i*6][5] = answer.section(":",(i*6)+6,(i*6)+6);
        ui->comboBox_2->addItem(GC[i*6][0]);
        if(a == 1){
            qDebug() << "subscribing to GC " << GC[i*6][0];
            Subscribe(GC[i*6][0]);
        }
    }
}

void MainWindow::Subscribe(QString sub){
    auto subscription = m_client->subscribe(sub);
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}
