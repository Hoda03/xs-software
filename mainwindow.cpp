#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#define SIZE_SYNCHRO_VDL2     4
const uint8_t synchrovdl2[SIZE_SYNCHRO_VDL2] = {0xff,0x00,0xf0,0xca};
int counter =0;
int counter_sqp_ind=0;
int counter_unit_data_ind=0;
int counter_unit_data_conf=0;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socketTcp = NULL;
    socketTcp = new QTcpSocket(this);
    socketTcp1 = new QTcpSocket();
    socketTR = new QUdpSocket(this);
    socketTR->bind(QHostAddress::LocalHost,8585);

    this->setWindowTitle("ACARS&TCP");
    this->setStyleSheet("QMainWindow { background-color : #ffffff}");
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::start_timer);

    socketsqp1 = new QUdpSocket(this);
    connect (socketTcp, &QTcpSocket::readyRead, [&](){

        QByteArray buffer;
        buffer.resize( socketTcp->bytesAvailable() );
        socketTcp->read( buffer.data(), buffer.size() );
        QByteArray ba_as_hex_string = buffer.toHex();
        ui->plainTextEdit->setReadOnly( true );

        if ((buffer[5] == 80) && (buffer[8] >= 18)){

            int PARAM_CONF = ui-> frequence_8 -> value();
            ui-> plainTextEdit->appendPlainText(QString("PARAM_CONF = %1").arg(PARAM_CONF));
            ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);

        }
        if (((buffer[8] == 47) || (buffer[8] == 44) || (buffer[8] == 40)) && ( buffer[9] == 1)){

            int PWR_SET_REQ = ui -> puissance_8 -> value();
            ui->plainTextEdit->appendPlainText(QString("PWR_SET_CONF = %1").arg(PWR_SET_REQ));
            ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);

          }

        if( buffer[9] == 1 && buffer[11] == 93){

                ui->plainTextEdit->appendPlainText(QString("MODE_SET_CONF"));
                ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);


           }
        if( buffer[5] == 85 ){
            counter_unit_data_conf++;

              ui->plainTextEdit->appendPlainText(QString("[%1] : UNIT DATA CONF").arg(counter_unit_data_conf));
              ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);


          }
        if(buffer[5] == 83){


            ui->plainTextEdit->appendPlainText(QString(" VDR_ERROR IND"));
            ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);



        }

    });
    connect (socketTcp1, &QTcpSocket::readyRead, [&](){

        QByteArray buffer;
        buffer.resize( socketTcp1->bytesAvailable() );
        socketTcp1->read( buffer.data(), buffer.size() );
        ui->plainTextEdit->setReadOnly( true );
        QByteArray ba_as_hex_string = buffer.toHex();

        if(buffer[5] == 83){


            ui->plainTextEdit->appendPlainText(QString(" VDR_ERROR IND"));
            ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);



        }
        if ((buffer[5] == 80) && (buffer[8] >= 18)){

            int PARAM_CONF = ui-> frequence_8 -> value();
            ui-> plainTextEdit->appendPlainText(QString("PARAM_CONF = %1").arg(PARAM_CONF));
            ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);

        }
        if( buffer[5] == 81 ){


              counter_unit_data_ind++;
              ui->plainTextEdit->appendPlainText(QString("[%1] : UNIT DATA IND").arg(counter_unit_data_ind));
               ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);
               socketTR->writeDatagram(buffer,QHostAddress::LocalHost, 0505);




           }
        if( buffer[9] == 1 && buffer[11] == 93){

                ui->plainTextEdit->appendPlainText(QString("MODE_SET_CONF"));
                ui->  plainTextEdit_3->appendPlainText(ba_as_hex_string);


           }
        if( buffer[5] == 32  && buffer[4] == 95){
           counter_sqp_ind++;
           ui->plainTextEdit->appendPlainText(QString("[%1] : SQP IND").arg(counter_sqp_ind));
           ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);
          socketsqp1->writeDatagram(buffer,QHostAddress::LocalHost,1999);



        }
       });
}
uint16_t CRCV2(uint8_t *x, uint16_t len) {
    uint8_t *data = x;
    uint16_t crc=0;

    while (len--) {
        crc ^= *data++;
        for (uint8_t k = 0; k < 8; k++)
            if (crc & 1) crc =(crc >> 1) ^ 0x8408;
            else crc = crc >> 1;
    }

    return crc;
}
MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionPARAM_REQ_triggered()
{
    /*------------------------Se connecter à la radio---------------------------------------------------*/

    QString ip = ui->lineEditip_15->text();
    quint16 port = ui->spinBox_24->value();

    socketTcp->connectToHost(QHostAddress(ip),port);

    /*-----------------------------structure de la Pid20----------------------------------------------------*/


    struct tsPid20 {

        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t freq_lsb;
        uint8_t freq_msb;
        uint8_t mod;
        uint8_t prekey;
        uint16_t crc;
    };

    /*-----------------------------------------------------------------------------------*/
    // Algo de traitement de la fréquence
    //Il faut créer une liste qui prend la taille de la valeur de fréquence désiré
    //mettre le nombre saisie dans le tableu sous forme de chiffre

    /*-----------------------------------------------------------------------------------------*/


    int T[6];
    int c = 0;
    int msb[2] , lsb[2] ;
    int monnombre = ui->frequence_8->value();
    int mynumber = 0;
    int p =1 , e = 1 , o = 1;

    /*------------------mettre le nmbre saisie dans un tableau en forme de chiffre---------------------------------*/

    while(monnombre!=0) {
        int mod = monnombre%10;
        T[c] = mod;
        monnombre-=mod;
        monnombre/=10;
        c++;
    }

    /*------------------------Reconversion du tableau en chiffre-----------------------------------------------------*/
   for(int i = 1; i<5 ; i++){
        mynumber =  mynumber + T[i]*p;
        p=p*10;
    }

    msb[0] = T[4] ;
    msb[1] = T[3];

    lsb[0] = T[2];
    lsb[1] = T[1];

    uint8_t var = 0 ;
    for(int k=1;k>=0;k--) {
            var = var +msb[k]*e ;
            e = e*10 ;
    }
    uint8_t var2 = 0 ;
    for(int k=1;k>=0;k--) {
            var2 = var2 +lsb[k]*o ;
            o = o*10 ;
    }


    uint8_t var4;
    if (ui -> prekey_8 -> value() >= 0 && ui -> prekey_8 -> value() <= 85 ){
        var4 = ui -> prekey_8 -> value();
    }
    else{

        var4 = 37;
    }
    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/

    unsigned char myData[] = {0xff, 0x00, 0xf0, 0xca, 0xf2, 0x20, 0x00, 0x04,var,var2,0x00,var4};
    unsigned int myDataLen;
    unsigned short myCrc;
    myDataLen = sizeof(myData);
    myCrc = CRCV2(myData, myDataLen);
    myCrc = ((myCrc << 8) | (myCrc >> 8));

    /*-------------Définition des champs de la trame et son envoie dans un fil------------- */

    if  ( ui -> frequence_8 -> value() >= 118000  &  ui -> frequence_8 -> value() <= 136000 )
            {
                tsPid20 Pid20;
                Pid20.synchro [0] = synchrovdl2[0];
                Pid20.synchro [1] = synchrovdl2[1];
                Pid20.synchro [2] = synchrovdl2[2];
                Pid20.synchro [3] = synchrovdl2[3];
                Pid20.protocoleCode = 0xF2;
                Pid20.pid = 0x20;
                Pid20.lengthMsb =0x00;
                Pid20.lengthLsb = 0x04;
                Pid20.freq_msb = var2;
                Pid20.freq_lsb = var;
                Pid20.mod = 0x00;
                Pid20.prekey=var4;
                Pid20.crc=myCrc;

                QByteArray buffer;


                //  ecrire les données dans le flux d'envoie
                QDataStream s(&buffer, QIODevice::WriteOnly);
                if (false){

                    s.setByteOrder(QDataStream::LittleEndian);
                }

                s << (uint8_t)Pid20.synchro [0]<< (uint8_t)Pid20.synchro [1]<<(uint8_t)Pid20.synchro [2]<<(uint8_t)Pid20.synchro [3]
                << (uint8_t)Pid20.protocoleCode << (uint8_t)Pid20.pid << (uint8_t)Pid20.lengthMsb <<(uint8_t)Pid20.lengthLsb
               <<(uint8_t)Pid20.freq_lsb<<(uint8_t)Pid20.freq_msb<<(uint8_t)Pid20.mod<<(uint8_t)Pid20.prekey<<(uint16_t)Pid20.crc;
                /*--------------------------------Envoie des données--------------------------------------*/


                socketTcp->write(buffer);
                int PARAM_REQ = ui-> frequence_8 -> value();
                ui-> plainTextEdit_2 -> appendPlainText(QString("PARAM_REQ_TX = %1").arg(PARAM_REQ));

                QByteArray ba_as_hex_string = buffer.toHex();
                ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);

            }
}


void MainWindow::on_actionPARAM_REQ_2_triggered()
{/*------------------------Se connecter à la radio---------------------------------------------------*/

    QString ip = ui->lineEditip_16->text();
    quint16 port = ui->spinBox_22->value();

    socketTcp1->connectToHost(QHostAddress(ip),port);

    /*-----------------------------structure de la Pid20----------------------------------------------------*/


    struct tsPid20 {

        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t freq_lsb;
        uint8_t freq_msb;
        uint8_t mod;
        uint8_t prekey;
        uint16_t crc;
    };

    /*-----------------------------------------------------------------------------------*/
    // Algo de traitement de la fréquence
    //Il faut créer une liste qui prend la taille de la valeur de fréquence désiré
    //mettre le nombre saisie dans le tableu sous forme de chiffre

    /*-----------------------------------------------------------------------------------------*/


    int T[6];
    int c = 0;
    int msb[2] , lsb[2] ;
    int monnombre = ui->frequence_8->value();
    int mynumber = 0;
    int p =1 , e = 1 , o = 1;

    /*------------------mettre le nmbre saisie dans un tableau en forme de chiffre---------------------------------*/

    while(monnombre!=0) {
        int mod = monnombre%10;
        T[c] = mod;
        monnombre-=mod;
        monnombre/=10;
        c++;
    }

    /*------------------------Reconversion du tableau en chiffre-----------------------------------------------------*/
   for(int i = 1; i<5 ; i++){
        mynumber =  mynumber + T[i]*p;
        p=p*10;
    }

    msb[0] = T[4] ;
    msb[1] = T[3];

    lsb[0] = T[2];
    lsb[1] = T[1];

    uint8_t var = 0 ;
    for(int k=1;k>=0;k--) {
            var = var +msb[k]*e ;
            e = e*10 ;
    }
    uint8_t var2 = 0 ;
    for(int k=1;k>=0;k--) {
            var2 = var2 +lsb[k]*o ;
            o = o*10 ;
    }


    uint8_t var4;
    if (ui -> prekey_8 -> value() >= 0 && ui -> prekey_8 -> value() <= 85 ){
        var4 = ui -> prekey_8 -> value();
    }
    else{

        var4 = 37;
    }
    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/

    unsigned char myData[] = {0xff, 0x00, 0xf0, 0xca, 0xf2, 0x20, 0x00, 0x04,var,var2,0x00,var4};
    unsigned int myDataLen;
    unsigned short myCrc;
    myDataLen = sizeof(myData);
    myCrc = CRCV2(myData, myDataLen);
    myCrc = ((myCrc << 8) | (myCrc >> 8));

    /*-------------Définition des champs de la trame et son envoie dans un fil------------- */

    if  ( ui -> frequence_8 -> value() >= 118000  &  ui -> frequence_8 -> value() <= 136000 )
            {
                tsPid20 Pid20;
                Pid20.synchro [0] = synchrovdl2[0];
                Pid20.synchro [1] = synchrovdl2[1];
                Pid20.synchro [2] = synchrovdl2[2];
                Pid20.synchro [3] = synchrovdl2[3];
                Pid20.protocoleCode = 0xF2;
                Pid20.pid = 0x20;
                Pid20.lengthMsb =0x00;
                Pid20.lengthLsb = 0x04;
                Pid20.freq_msb = var2;
                Pid20.freq_lsb = var;
                Pid20.mod = 0x00;
                Pid20.prekey=var4;
                Pid20.crc=myCrc;

                QByteArray buffer;


                //  ecrire les données dans le flux d'envoie
                QDataStream s(&buffer, QIODevice::WriteOnly);
                if (false){

                    s.setByteOrder(QDataStream::LittleEndian);
                }

                s << (uint8_t)Pid20.synchro [0]<< (uint8_t)Pid20.synchro [1]<<(uint8_t)Pid20.synchro [2]<<(uint8_t)Pid20.synchro [3]
                << (uint8_t)Pid20.protocoleCode << (uint8_t)Pid20.pid << (uint8_t)Pid20.lengthMsb <<(uint8_t)Pid20.lengthLsb
               <<(uint8_t)Pid20.freq_lsb<<(uint8_t)Pid20.freq_msb<<(uint8_t)Pid20.mod<<(uint8_t)Pid20.prekey<<(uint16_t)Pid20.crc;
                /*--------------------------------Envoie des données--------------------------------------*/


                socketTcp1->write(buffer);
                int PARAM_REQ = ui-> frequence_8 -> value();
                ui-> plainTextEdit_2 -> appendPlainText(QString("PARAM_REQ_RX = %1").arg(PARAM_REQ));

                QByteArray ba_as_hex_string = buffer.toHex();
                ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);

            }

}


void MainWindow::on_actionUNIT_DATA_REQ_triggered()
{

    /*------------------------Se connecter à la radio---------------------------------------------------*/


    QString ip = ui->lineEditip_15->text();
    quint16 port = ui->spinBox_24->value();

    socketTcp->connectToHost(QHostAddress(ip),port);



    /*-----------------------------structure de la Pid21----------------------------------------------------*/
    struct tsPid21{
        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t donnees;
        uint8_t sufBcs;
        uint8_t soh;
        uint8_t mode;
        uint8_t address1;
        uint8_t address2;
        uint8_t address3;
        uint8_t address4;
        uint8_t address5;
        uint8_t address6;
        uint8_t address7;
        uint8_t ack;
        uint8_t label1;
        uint8_t label2;
        uint8_t bi;
        uint8_t stx;
        char str1[220];
        uint8_t suff;
        uint8_t bcs;
        uint8_t DE;
        uint8_t bcsSuff;
        uint16_t crc;
        uint8_t msn;
        uint8_t flightid;


    };
   // uint8_t varh = ui-> spinBox -> value() + 27;

    uint8_t var6 = ((ui-> spinBox_23 -> value())) +28;
    unsigned char str[] ={
    0x4c, 0x27, 0x20, 0x41, 0x43, 0x41, 0x52, 0x53 , 0x20, 0x41,
    0x69, 0x72, 0x63, 0x72, 0x61, 0x66, 0x74, 0x20, 0x43, 0x6f,
    0x6d, 0x6d, 0x75,0x6e,  0x69, 0x63, 0x61, 0x74 ,0x69, 0x6f,
    0x6e, 0x20, 0x41 ,0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x69,
    0x6e, 0x67, 0x20 ,0x61, 0x6e, 0x64, 0x20 ,0x52, 0x65, 0x70,
    0x6f, 0x72, 0x74, 0x69 ,0x6e, 0x67,  0x20, 0x53 ,0x79, 0x73,
    0x74, 0x65, 0x6d, 0x29, 0x20, 0x65 ,0x73 ,0x74 ,0x20,0x75,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6c, 0x69, 0x74 ,0x65, 0x20, 0x68,0x7f,0x64,0x61,0x61};
    unsigned char str2[ui-> spinBox_23->value()];
    memcpy(str2, str,sizeof(str2));
    QByteArray databuf = QByteArray((char*)str2, sizeof(str2));

    /*--------------------------- initialisation des valeurs de chaque paramètre de la structure du PARAM_DATA_REQ -------------------------------*/

    tsPid21 Pid21;
    Pid21.synchro [0] = synchrovdl2[0];
    Pid21.synchro [1] = synchrovdl2[1];
    Pid21.synchro [2] = synchrovdl2[2];
    Pid21.synchro [3] = synchrovdl2[3];
    Pid21.protocoleCode = 0xF2;
    Pid21.pid = 0x21;
    Pid21.lengthMsb=0x00;
    Pid21.lengthLsb=var6;
    Pid21.soh = 0x01;
    Pid21.mode = 0x02;

    Pid21.address1=0x2e;
    Pid21.address2=0x2e;
    Pid21.address3=0x6e;
    Pid21.address4=0x33;
    Pid21.address5=0x38;
    Pid21.address6=0x37;
    Pid21.address7=0x32;
    Pid21.ack = 0x01;
    Pid21.label1 = 0x01;
    Pid21.label2 = 0x00;
    Pid21.bi = 0x01;
    Pid21.stx = 0x01;
    Pid21.msn=0x31;
    Pid21.flightid=0x58;
    //strncpy ((char *)Pid21.str1,str, longeur );
    Pid21.suff = 0x03;


    /*---------------------------- Cette Fonction Permettre d'écrire les données dans un flus d'envoie ---------------------------------*/

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);

    if (false){

        s.setByteOrder(QDataStream::LittleEndian);
    }
    s << (uint8_t)Pid21.synchro [0]<< (uint8_t)Pid21.synchro [1]<<(uint8_t)Pid21.synchro [2]
      <<(uint8_t)Pid21.synchro [3]  << (uint8_t)Pid21.protocoleCode << (uint8_t)Pid21.pid
      << (uint8_t)Pid21.lengthMsb<< (uint8_t)Pid21.lengthLsb<<(uint8_t)Pid21.soh
      <<(uint8_t)Pid21.mode<<(uint8_t)Pid21.address1<<(uint8_t)Pid21.address2<<(uint8_t)Pid21.address3
      <<(uint8_t)Pid21.address4<<(uint8_t)Pid21.address5<<(uint8_t)Pid21.address6
      <<(uint8_t)Pid21.address7<<(uint8_t)Pid21.ack<<(uint8_t)Pid21.label1<<(uint8_t)Pid21.label2
      <<(uint8_t)Pid21.bi<<(uint8_t)Pid21.stx<<(uint8_t)Pid21.msn<<(uint8_t)Pid21.msn<<(uint8_t)Pid21.msn<<(uint8_t)Pid21.msn
      <<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid
      <<(uint8_t)Pid21.flightid;

    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/


    unsigned char myData[] ={
    0xff ,0x00, 0xf0, 0xca, 0xf2, 0x21, 0x00 ,var6 , 0x01,0x02,
    0x2e , 0x2e , 0x6e , 0x33 , 0x38 , 0x37,0x32 , 0x01 , 0x01,
    0x00 ,0x01, 0x01,0x31,0x31,0x31,0x31,0x58,0x58,0x58,0x58,
    0x58,0x58,    0x4c, 0x27, 0x20, 0x41, 0x43, 0x41, 0x52, 0x53 , 0x20, 0x41,
    0x69, 0x72, 0x63, 0x72, 0x61, 0x66, 0x74, 0x20, 0x43, 0x6f,
    0x6d, 0x6d, 0x75,0x6e,  0x69, 0x63, 0x61, 0x74 ,0x69, 0x6f,
    0x6e, 0x20, 0x41 ,0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x69,
    0x6e, 0x67, 0x20 ,0x61, 0x6e, 0x64, 0x20 ,0x52, 0x65, 0x70,
    0x6f, 0x72, 0x74, 0x69 ,0x6e, 0x67,  0x20, 0x53 ,0x79, 0x73,
    0x74, 0x65, 0x6d, 0x29, 0x20, 0x65 ,0x73 ,0x74 ,0x20,0x75,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6c, 0x69, 0x74 ,0x65, 0x20, 0x68,0x7f,0x64,0x61,0x61};


      //nouveau atbleau ou on ibnsère le nombre saisie dans l'interface graphique de caractère
   unsigned char myData1[ui-> spinBox_23->value()+32];

      // on copie les valeurs dans le nouveau tableau
   memcpy(myData1, myData,sizeof(myData1));

     // un tableau qui contien la valeur 0x03
   unsigned char myData3[] ={0x03,0x02,0x02,0x7f};
   QByteArray databuf3 = QByteArray((char*)myData3, sizeof(myData3));



     // on conbine entre les deux tableau pour l'intégrer deans la fonction qui calcule le crc

   unsigned char allstrings[sizeof(myData1)+sizeof(myData3)];
   memcpy(allstrings,myData1,sizeof(myData1));
   memcpy(allstrings+sizeof(myData1),myData3,sizeof(myData3));


   unsigned short myCrc;
   int myDataLen = sizeof(myData1)+ sizeof(myData3);
   myCrc = CRCV2(allstrings, myDataLen);
   myCrc = ((myCrc << 8) | (myCrc >> 8));

   Pid21.crc=myCrc;
   QByteArray bufferCrc;
   QDataStream g(&bufferCrc, QIODevice::WriteOnly);

   if (false){

       g.setByteOrder(QDataStream::LittleEndian);
   }
   g<< (uint16_t)Pid21.crc;

   /*--------------------------------Envoie des données--------------------------------------*/



   QByteArray buffer2 ;

   buffer2 = buffer;
   buffer2 = buffer2.append(databuf);
   buffer2 = buffer2.append(databuf3);

   buffer2 = buffer2 + bufferCrc;
   socketTcp->write(buffer2);
   socketTR->writeDatagram(buffer2,QHostAddress::LocalHost, 0505);
   counter++;
   ui-> plainTextEdit_2 -> appendPlainText(QString("[%1] : UNIT_DATA_REQ").arg(counter));

   QByteArray ba_as_hex_string = buffer2.toHex();
   ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);
}


void MainWindow::on_send_clicked()
{
    timer->start(1000);

}


void MainWindow::on_actionPWR_SET_REQ_triggered()
{
    /*------------------------Se connecter à la radio---------------------------------------------------*/

    QString ip = ui->lineEditip_15->text();
    quint16 port = ui->spinBox_24->value();

    socketTcp->connectToHost(QHostAddress(ip),port);

   /*-----------------------------structure de la PidF1----------------------------------------------------*/

    struct tsPidF1{
        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t pwr;
        uint8_t req;
        uint16_t crc;

};
    /*----------------instruction conditionnelle pour l'envoie de la puissance à la Radio------------------------*/


    int pwr = ui -> puissance_8 -> value();
    uint8_t var3;

   if(  pwr == 50 ) {
         var3 = 47;
    }
   if ( pwr == 25){
         var3 = 44;
    }
   if ( pwr == 10) {

         var3 = 40;
    }


   /*-------------Définition des champs de la trame ------------- */



    tsPidF1 PidF1;

    PidF1.synchro [0] = synchrovdl2[0];
    PidF1.synchro [1] = synchrovdl2[1];
    PidF1.synchro [2] = synchrovdl2[2];
    PidF1.synchro [3] = synchrovdl2[3];
    PidF1.protocoleCode = 0xF2;
    PidF1.pid = 0xF1;
    PidF1.lengthMsb =0x00;
    PidF1.lengthLsb = 0x02;
    PidF1.pwr = var3;
    PidF1.req = 0x00;

    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/

    unsigned char myData[] = {0xff, 0x00, 0xf0, 0xca, 0xf2, 0xf1, 0x00, 0x02,var3,0x00};
    unsigned int myDataLen;
    unsigned short myCrc;
    myDataLen = sizeof(myData);
    myCrc = CRCV2(myData, myDataLen);
    myCrc = ((myCrc << 8) | (myCrc >> 8));
    PidF1.crc = myCrc;


    /*-------------------Intégration des champs du buffer dans un fil d'envoie--------------------------*/

    QByteArray buffer;

    //  ecrire les données dans le flux d'envoie
    QDataStream s(&buffer, QIODevice::WriteOnly);

    if (false){

        s.setByteOrder(QDataStream::LittleEndian);
    }
     s << (uint8_t)PidF1.synchro [0]<< (uint8_t)PidF1.synchro [1]<<(uint8_t)PidF1.synchro [2]
     <<(uint8_t)PidF1.synchro [3]  << (uint8_t)PidF1.protocoleCode << (uint8_t)PidF1.pid
     << (uint8_t)PidF1.lengthMsb <<(uint8_t)PidF1.lengthLsb<<(uint8_t)PidF1.pwr<<(uint8_t)PidF1.req<<(uint16_t)PidF1.crc;

     /*--------------------------------Envoie des données--------------------------------------*/

     socketTcp->write(buffer);
     int PWR_SET_REQ = ui -> puissance_8 -> value();
     ui-> plainTextEdit_2 -> appendPlainText(QString("PWR_SET_REQ= %1").arg(PWR_SET_REQ));

     QByteArray ba_as_hex_string = buffer.toHex();
     ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);

}


void MainWindow::on_actionMODE_SET_REQ_triggered()
{
    /*------------------------Se connecter à la radio---------------------------------------------------*/

    QString ip = ui->lineEditip_15->text();
    quint16 port = ui->spinBox_24->value();

    socketTcp->connectToHost(QHostAddress(ip),port);


    /*-----------------------------structure de la PidF0----------------------------------------------------*/


    struct tsPidF0{
        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t mode;
        uint8_t req;
        uint16_t crc;

    };


    /*-------------Définition des champs de la trame ------------- */

    tsPidF0 PidF0;
    PidF0.synchro [0] = synchrovdl2[0];
    PidF0.synchro [1] = synchrovdl2[1];
    PidF0.synchro [2] = synchrovdl2[2];
    PidF0.synchro [3] = synchrovdl2[3];
    PidF0.protocoleCode = 0xF2;
    PidF0.pid = 0xF0;
    PidF0.lengthMsb =0x00;
    PidF0.lengthLsb = 0x02;
    PidF0.mode = 0xF1;
    PidF0.req = 0x00;


    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/

    unsigned char myData[] = {0xff, 0x00, 0xf0, 0xca, 0xf2, 0xf0, 0x00, 0x02,0xf1, 0x00};

    unsigned int myDataLen;
    unsigned short myCrc;
    myDataLen = sizeof(myData);
    myCrc = CRCV2(myData, myDataLen);
    myCrc = ((myCrc << 8) | (myCrc >> 8));
    PidF0.crc=myCrc;


    /*-------------------Intégration des champs du buffer dans un flux d'envoie--------------------------*/

    QByteArray buffer;
    //  ecrire les données dans le flux d'envoie
    QDataStream s(&buffer, QIODevice::WriteOnly);

    if (false){

        s.setByteOrder(QDataStream::LittleEndian);
    }
    s << (uint8_t)PidF0.synchro [0]<< (uint8_t)PidF0.synchro [1]<<(uint8_t)PidF0.synchro [2]<<(uint8_t)PidF0.synchro [3]  << (uint8_t)PidF0.protocoleCode << (uint8_t)PidF0.pid << (uint8_t)PidF0.lengthMsb <<(uint8_t)PidF0.lengthLsb<<(uint8_t)PidF0.mode<<(uint8_t)PidF0.req<<(uint16_t)PidF0.crc;


    /*--------------------------------Envoie des données--------------------------------------*/

    socketTcp->write(buffer);
    ui-> plainTextEdit_2 -> appendPlainText("MODE_SET_REQ");

    QByteArray ba_as_hex_string = buffer.toHex();
    ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);

}


void MainWindow::on_actionPURGE_RQ_triggered()
{

}


void MainWindow::on_actionMODE_SET_REQ_2_triggered()
{
    /*------------------------Se connecter à la radio---------------------------------------------------*/

        QString ip = ui->lineEditip_16->text();
        quint16 port = ui->spinBox_22->value();

        socketTcp1->connectToHost(QHostAddress(ip),port);



    /*-----------------------------structure de la PidF0----------------------------------------------------*/


    struct tsPidF0{
        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t mode;
        uint8_t req;
        uint16_t crc;

    };


    /*-------------Définition des champs de la trame ------------- */

    tsPidF0 PidF0;
    PidF0.synchro [0] = synchrovdl2[0];
    PidF0.synchro [1] = synchrovdl2[1];
    PidF0.synchro [2] = synchrovdl2[2];
    PidF0.synchro [3] = synchrovdl2[3];
    PidF0.protocoleCode = 0xF2;
    PidF0.pid = 0xF0;
    PidF0.lengthMsb =0x00;
    PidF0.lengthLsb = 0x02;
    PidF0.mode = 0xF1;
    PidF0.req = 0x00;


    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/

    unsigned char myData[] = {0xff, 0x00, 0xf0, 0xca, 0xf2, 0xf0, 0x00, 0x02,0xf1, 0x00};

    unsigned int myDataLen;
    unsigned short myCrc;
    myDataLen = sizeof(myData);
    myCrc = CRCV2(myData, myDataLen);
    myCrc = ((myCrc << 8) | (myCrc >> 8));
    PidF0.crc=myCrc;


    /*-------------------Intégration des champs du buffer dans un flux d'envoie--------------------------*/

    QByteArray buffer;
    //  ecrire les données dans le flux d'envoie
    QDataStream s(&buffer, QIODevice::WriteOnly);

    if (false){

        s.setByteOrder(QDataStream::LittleEndian);
    }
    s << (uint8_t)PidF0.synchro [0]<< (uint8_t)PidF0.synchro [1]<<(uint8_t)PidF0.synchro [2]<<(uint8_t)PidF0.synchro [3]  << (uint8_t)PidF0.protocoleCode << (uint8_t)PidF0.pid << (uint8_t)PidF0.lengthMsb <<(uint8_t)PidF0.lengthLsb<<(uint8_t)PidF0.mode<<(uint8_t)PidF0.req<<(uint16_t)PidF0.crc;


    /*--------------------------------Envoie des données--------------------------------------*/

    socketTcp1->write(buffer);
    ui-> plainTextEdit_2 -> appendPlainText("MODE_SET_REQ");

    QByteArray ba_as_hex_string = buffer.toHex();
    ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);

}


void MainWindow::on_STOP_clicked()
{
    timer->stop();

}
void MainWindow::start_timer(){
    /*------------------------Se connecter à la radio---------------------------------------------------*/


    QString ip = ui->lineEditip_15->text();
    quint16 port = ui->spinBox_24->value();

    socketTcp->connectToHost(QHostAddress(ip),port);



    /*-----------------------------structure de la Pid21----------------------------------------------------*/
    struct tsPid21{
        uint8_t synchro[SIZE_SYNCHRO_VDL2];
        uint8_t protocoleCode;
        uint8_t pid;
        uint8_t lengthMsb;
        uint8_t lengthLsb;
        uint8_t donnees;
        uint8_t sufBcs;
        uint8_t soh;
        uint8_t mode;
        uint8_t address1;
        uint8_t address2;
        uint8_t address3;
        uint8_t address4;
        uint8_t address5;
        uint8_t address6;
        uint8_t address7;
        uint8_t ack;
        uint8_t label1;
        uint8_t label2;
        uint8_t bi;
        uint8_t stx;
        char str1[220];
        uint8_t suff;
        uint8_t bcs;
        uint8_t DE;
        uint8_t bcsSuff;
        uint16_t crc;
        uint8_t msn;
        uint8_t flightid;


    };
   // uint8_t varh = ui-> spinBox -> value() + 27;

    uint8_t var6 = ((ui-> spinBox_23 -> value())) +28;
    unsigned char str[] ={
    0x4c, 0x27, 0x20, 0x41, 0x43, 0x41, 0x52, 0x53 , 0x20, 0x41,
    0x69, 0x72, 0x63, 0x72, 0x61, 0x66, 0x74, 0x20, 0x43, 0x6f,
    0x6d, 0x6d, 0x75,0x6e,  0x69, 0x63, 0x61, 0x74 ,0x69, 0x6f,
    0x6e, 0x20, 0x41 ,0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x69,
    0x6e, 0x67, 0x20 ,0x61, 0x6e, 0x64, 0x20 ,0x52, 0x65, 0x70,
    0x6f, 0x72, 0x74, 0x69 ,0x6e, 0x67,  0x20, 0x53 ,0x79, 0x73,
    0x74, 0x65, 0x6d, 0x29, 0x20, 0x65 ,0x73 ,0x74 ,0x20,0x75,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6c, 0x69, 0x74 ,0x65, 0x20, 0x68,0x7f,0x64,0x61,0x61};
    unsigned char str2[ui-> spinBox_23->value()];
    memcpy(str2, str,sizeof(str2));
    QByteArray databuf = QByteArray((char*)str2, sizeof(str2));

    /*--------------------------- initialisation des valeurs de chaque paramètre de la structure du PARAM_DATA_REQ -------------------------------*/

    tsPid21 Pid21;
    Pid21.synchro [0] = synchrovdl2[0];
    Pid21.synchro [1] = synchrovdl2[1];
    Pid21.synchro [2] = synchrovdl2[2];
    Pid21.synchro [3] = synchrovdl2[3];
    Pid21.protocoleCode = 0xF2;
    Pid21.pid = 0x21;
    Pid21.lengthMsb=0x00;
    Pid21.lengthLsb=var6;
    Pid21.soh = 0x01;
    Pid21.mode = 0x02;

    Pid21.address1=0x2e;
    Pid21.address2=0x2e;
    Pid21.address3=0x6e;
    Pid21.address4=0x33;
    Pid21.address5=0x38;
    Pid21.address6=0x37;
    Pid21.address7=0x32;
    Pid21.ack = 0x01;
    Pid21.label1 = 0x01;
    Pid21.label2 = 0x00;
    Pid21.bi = 0x01;
    Pid21.stx = 0x01;
    Pid21.msn=0x31;
    Pid21.flightid=0x58;
    //strncpy ((char *)Pid21.str1,str, longeur );
    Pid21.suff = 0x03;


    /*---------------------------- Cette Fonction Permettre d'écrire les données dans un flus d'envoie ---------------------------------*/

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);

    if (false){

        s.setByteOrder(QDataStream::LittleEndian);
    }
    s << (uint8_t)Pid21.synchro [0]<< (uint8_t)Pid21.synchro [1]<<(uint8_t)Pid21.synchro [2]
      <<(uint8_t)Pid21.synchro [3]  << (uint8_t)Pid21.protocoleCode << (uint8_t)Pid21.pid
      << (uint8_t)Pid21.lengthMsb<< (uint8_t)Pid21.lengthLsb<<(uint8_t)Pid21.soh
      <<(uint8_t)Pid21.mode<<(uint8_t)Pid21.address1<<(uint8_t)Pid21.address2<<(uint8_t)Pid21.address3
      <<(uint8_t)Pid21.address4<<(uint8_t)Pid21.address5<<(uint8_t)Pid21.address6
      <<(uint8_t)Pid21.address7<<(uint8_t)Pid21.ack<<(uint8_t)Pid21.label1<<(uint8_t)Pid21.label2
      <<(uint8_t)Pid21.bi<<(uint8_t)Pid21.stx<<(uint8_t)Pid21.msn<<(uint8_t)Pid21.msn<<(uint8_t)Pid21.msn<<(uint8_t)Pid21.msn
      <<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid<<(uint8_t)Pid21.flightid
      <<(uint8_t)Pid21.flightid;

    /*-------------------------------Calcul du Crc16 Kermit-------------------------------------*/


    unsigned char myData[] ={
    0xff ,0x00, 0xf0, 0xca, 0xf2, 0x21, 0x00 ,var6 , 0x01,0x02,
    0x2e , 0x2e , 0x6e , 0x33 , 0x38 , 0x37,0x32 , 0x01 , 0x01,
    0x00 ,0x01, 0x01,0x31,0x31,0x31,0x31,0x58,0x58,0x58,0x58,
    0x58,0x58,    0x4c, 0x27, 0x20, 0x41, 0x43, 0x41, 0x52, 0x53 , 0x20, 0x41,
    0x69, 0x72, 0x63, 0x72, 0x61, 0x66, 0x74, 0x20, 0x43, 0x6f,
    0x6d, 0x6d, 0x75,0x6e,  0x69, 0x63, 0x61, 0x74 ,0x69, 0x6f,
    0x6e, 0x20, 0x41 ,0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x69,
    0x6e, 0x67, 0x20 ,0x61, 0x6e, 0x64, 0x20 ,0x52, 0x65, 0x70,
    0x6f, 0x72, 0x74, 0x69 ,0x6e, 0x67,  0x20, 0x53 ,0x79, 0x73,
    0x74, 0x65, 0x6d, 0x29, 0x20, 0x65 ,0x73 ,0x74 ,0x20,0x75,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6e, 0x20,  0x73, 0x79, 0x73, 0x74, 0x74, 0x74, 0x74 ,0x74,
    0x6c, 0x69, 0x74 ,0x65, 0x20, 0x68,0x7f,0x64,0x61,0x61};


      //nouveau atbleau ou on ibnsère le nombre saisie dans l'interface graphique de caractère
   unsigned char myData1[ui-> spinBox_23->value()+32];

      // on copie les valeurs dans le nouveau tableau
   memcpy(myData1, myData,sizeof(myData1));

     // un tableau qui contien la valeur 0x03
   unsigned char myData3[] ={0x03,0x02,0x02,0x7f};
   QByteArray databuf3 = QByteArray((char*)myData3, sizeof(myData3));



     // on conbine entre les deux tableau pour l'intégrer deans la fonction qui calcule le crc

   unsigned char allstrings[sizeof(myData1)+sizeof(myData3)];
   memcpy(allstrings,myData1,sizeof(myData1));
   memcpy(allstrings+sizeof(myData1),myData3,sizeof(myData3));


   unsigned short myCrc;
   int myDataLen = sizeof(myData1)+ sizeof(myData3);
   myCrc = CRCV2(allstrings, myDataLen);
   myCrc = ((myCrc << 8) | (myCrc >> 8));

   Pid21.crc=myCrc;
   QByteArray bufferCrc;
   QDataStream g(&bufferCrc, QIODevice::WriteOnly);

   if (false){

       g.setByteOrder(QDataStream::LittleEndian);
   }
   g<< (uint16_t)Pid21.crc;

   /*--------------------------------Envoie des données--------------------------------------*/



   QByteArray buffer2 ;

   buffer2 = buffer;
   buffer2 = buffer2.append(databuf);
   buffer2 = buffer2.append(databuf3);

   buffer2 = buffer2 + bufferCrc;
   socketTcp->write(buffer2);
   socketTR->writeDatagram(buffer2,QHostAddress::LocalHost, 0505);
   counter++;
   ui-> plainTextEdit_2 -> appendPlainText(QString("[%1] : UNIT_DATA_REQ").arg(counter));

   QByteArray ba_as_hex_string = buffer2.toHex();
   ui->  plainTextEdit_4->appendPlainText(ba_as_hex_string);
}


void MainWindow::on_sqp_clicked()
{
    sqp.show();
}


void MainWindow::on_TRANSMISSION_RATIO_clicked()
{
    transmission_ratio.show();
}


void MainWindow::on_actionVersion_triggered()
{
    version.show();
}

