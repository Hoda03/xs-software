#include "sqp1.h"
#include "ui_sqp1.h"

SQP1::SQP1(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SQP1)
{
    ui->setupUi(this);
    this->setWindowTitle("Signal Quality");

    mSockett = new QUdpSocket(this);
    mSockett->bind(QHostAddress::LocalHost,6969);
    connect(mSockett,&QUdpSocket::readyRead,[&](){
          while (mSockett ->hasPendingDatagrams())
          {
              QByteArray data;
              QHostAddress sender;
              quint16 senderPort;
              data.resize(mSockett->pendingDatagramSize());
              mSockett->readDatagram(data.data(),data.size(),&sender, &senderPort);
              int hd = (int)data[8];
              if (counter > 10 ){
                  for (int i=0; i<9; i++){
                      numbers[i]= numbers[i+1];
                  }
                  numbers[9] = hd;
              }
              else {
                  numbers[counter] = hd;
              }
              counter++;

              QBarSet *set0 = new QBarSet("Signal Quality");

              for(int i=0;i<10; i++){
             // *set0 << numbers[0]<< numbers[1]<< numbers[2]<< numbers[3]<< numbers[4]<< numbers[5]<< numbers[6]<< numbers[7]<< numbers[8]<< numbers[9]<< numbers[10] ;
               *set0 << numbers[i];
              }
              QBarSeries *series = new QBarSeries();
              series->append(set0);

              graphe = new QChart();
              graphe->setTitle("Signal Quality graph");
              graphe->addSeries(series);


              QBarCategoryAxis *axisX = new QBarCategoryAxis();
              graphe->addAxis(axisX, Qt::AlignBottom);
              series->attachAxis(axisX);


              QValueAxis *axisY = new QValueAxis();
              axisY->setRange(0,511);
              graphe->addAxis(axisY, Qt::AlignLeft);
              series->attachAxis(axisY);



                graphique = new QChartView;
                graphique->setRenderHint(QPainter::Antialiasing);
                graphique->setChart(graphe);
                setCentralWidget(graphique);
                resize(640, 480);


  }

  });
}

SQP1::~SQP1()
{
    delete ui;
}
