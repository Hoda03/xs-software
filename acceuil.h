#ifndef ACCEUIL_H
#define ACCEUIL_H

#include <QMainWindow>
#include "mainwindow.h"
#include "mainwindow1.h"
#include "mainwindow3.h"
#include "mainwindow4.h"

namespace Ui {
class acceuil;
}

class acceuil : public QMainWindow
{
    Q_OBJECT

public:
    explicit acceuil(QWidget *parent = nullptr);
    ~acceuil();

private slots:
    void on_acarsvdl2_clicked();

private:
    Ui::acceuil *ui;
    MainWindow mainwindow;
    MainWindow1 mainwindow1;
    MainWindow3 mainwindow3;
    MainWindow4 mainwindow4;
};

#endif // ACCEUIL_H

