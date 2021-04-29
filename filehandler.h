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
    QString readAllFromFile(QString fileName);

};

inline QString writeToFile(QString chatname, QString username,QString messages){
    QString Chatname = chatname.split(" ").at(0);
    QString Username = username.split(" ").at(0);
    QFile file("//home//ntu-user//Application//chatroom"+Chatname+".txt");
    if(!file.open(QIODevice::Append | QIODevice::Text)){
        return "no file";
    }else{
        QTextStream out(&file);
        out << Username + "\n";
        out << messages + "\n";
        out << " \n";
        file.close();
        return "file updated";
    }
}

inline QString readAllFromFile(QString fileName){

    QFile file("//home//ntu-user//Application//chatroom"+fileName+".txt");
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        return "no file";
    } else{
        QTextStream in(&file);
        QString allText = in.readAll();
        file.close();
        return allText;
    }
}

#endif // FILEHANDLER_H
