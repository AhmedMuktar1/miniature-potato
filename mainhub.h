#ifndef MAINHUB_H
#define MAINHUB_H

#include <QDialog>

namespace Ui {
class mainhub;
}

class mainhub : public QDialog
{
    Q_OBJECT

public:
    explicit mainhub(QWidget *parent = nullptr);
    ~mainhub();

private slots:
    void on_pushButton_clicked();

private:
    Ui::mainhub *ui;
};

#endif // MAINHUB_H
