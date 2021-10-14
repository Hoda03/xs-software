#include "acceuil.h"
#include "ui_acceuil.h"
#include <QMessageBox>

acceuil::acceuil(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::acceuil)
{
    ui->setupUi(this);
    this->setWindowTitle("X&S");
    this->setStyleSheet("QMainWindow { background-color : #ffffff}");
    ui->label_2->setFrameStyle(QFrame::Box | QFrame::Sunken);
    ui-> label_2->setLineWidth(3);
    ui-> label_2->setMidLineWidth(2);

    ui->label->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    ui-> label->setLineWidth(3);
    ui-> label->setMidLineWidth(3);

    ui->label_3->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    ui-> label_3->setLineWidth(3);
    ui-> label_3->setMidLineWidth(3);

}

acceuil::~acceuil()
{
    delete ui;
}

void acceuil::on_acarsvdl2_clicked()
{
    if ((ui -> acars -> isChecked() & ui -> radioButton_5 -> isChecked()))
     {
         acceuil::close();
         mainwindow3.show();
     }
    else if ((ui -> acars -> isChecked() & ui -> radioButton_6 -> isChecked()))
     {
         acceuil::close();
         mainwindow.show();
     }
    else if ((ui -> vdl2 -> isChecked() & ui -> radioButton_5 -> isChecked()))
     {
         acceuil::close();
         mainwindow4.show();
     }
    else if ((ui -> vdl2 -> isChecked() & ui -> radioButton_6 -> isChecked()))
     {
         acceuil::close();
         mainwindow1.show();
     }
     else
     {
         QMessageBox::information(this, "Erreur", "Attention aucun port ni protocole sélectionnés !");

     }
}

