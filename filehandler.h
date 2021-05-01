#ifndef FILEHANDLER_H
#define FILEHANDLER_H
#include <QString>
#include <QFile>
#include <QTextStream>

class filehandler
{
public:
    filehandler();
    QString writeToFile(QString chatname, QString username,QString messages);
    QString readAllFromFile(int command, QString fileName);

};

inline QString writeToFile(QString chatname, QString username,QString messages){
    QString filepath;
    filepath = "//home//ntu-user//SDIChatApplication//";
    QFile file(filepath+chatname);
    file.open(QIODevice::Append | QIODevice::Text);
    QTextStream out(&file);
    if(username == "updateFriendsList"){
        out << messages + "\n";
    }else if(username == "Signup"){
        QString a = messages.section(":",0,0);
        QString b = messages.section(":",1,1);
        out << a << +"\n";
        out << b << +"\n";
    } else if(username == "newGC"){
        out<< messages +"\n";
    } else if (username == ""){
        //used to just create a new file.
    }else{
        out << username + "\n";
        out << messages + "\n";
        out << " \n";
    }
    file.close();
    return "done";
}

inline QString readAllFromFile(int command, QString fileName){
    QString filepath;
    int Command = command;

    if(Command == 1 || command == 3){
        filepath = ("//home//ntu-user//SDIChatApplication//"+fileName);
    } else if(Command == 2){
        filepath = ("//home//ntu-user//SDIChatApplication//"+fileName+".txt");
    }

    QFile file(filepath);

    if(!file.open(QFile::ReadOnly | QFile::Text)){ return "no file";
    } else{
        QTextStream in(&file);
        if(command == 1 || command == 3)
        {
            QString message;
            int num = 0;

            while(!in.atEnd()){
                QString line = in.readLine();
                if(command == 1){
                    message = message +line+":";
                } else if(command == 3){
                    message = message +line;
                }
                num++;
            }

            file.close();
            return QString::number(num)+":"+message;

        } else if (command == 2)
        {
            QString allText = in.readAll();
            file.close();
            return allText;
        }
    }
    return "0";
}

#endif // FILEHANDLER_H
