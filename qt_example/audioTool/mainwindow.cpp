#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // network init
    isConnected=false;
    isStart=false;
    isRefresh=true;
    setPackageHeadLen();
    fileLength=0;

    mySocket = new QTcpSocket(this);
    connect(mySocket, SIGNAL(connected()), this, SLOT(connectedSlot()));
    connect(mySocket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
    connect(mySocket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connect(mySocket, SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(errorSlot(QAbstractSocket::SocketError)));

    // qt audio player init
    QAudioFormat audioFormat; //设置采样格式
    audioFormat.setSampleRate(44100);     // 设置采样率
    audioFormat.setChannelCount(2);       // 设置通道数
    audioFormat.setSampleSize(16);        // 设置采样大小，一般为8位或16位
    audioFormat.setCodec("audio/pcm");    // 设置编码方式
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);      //设置字节序
    audioFormat.setSampleType(QAudioFormat::UnSignedInt);      //设置样本数据类型

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice(); //音频设备信息
    if (!info.isFormatSupported(audioFormat))
    {
        qDebug()<<"default format not supported try to use nearest";
        audioFormat = info.nearestFormat(audioFormat);
    }
    audioOutput = new QAudioOutput(audioFormat, this);
    streamOut = audioOutput->start();

    timer=new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(slotTimeout()));

    // zplay init
    player = CreateZPlay();
    myTimer=new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), SLOT(myTimerout()));

    // Music spect rum
    spect.resize(300);

    ui->statusBar->showMessage(tr("欢迎使用音频测试工具!"));
}

MainWindow::~MainWindow()
{
    delete mySocket;
    delete audioOutput;
    delete timer;
    delete myTimer;

    player->Release();

    delete ui;
}

void MainWindow::connectedSlot()
{
    isConnected = true;
    ui->statusBar->showMessage(tr("已连接!"));
    ui->connectBtn->setText("断开");
}

void MainWindow::disconnectedSlot()
{
    isConnected = false;
    mySocket->close();
    ui->statusBar->showMessage(tr("断开连接!"));
    ui->connectBtn->setText("连接");

    isStart=false;
}

void MainWindow::readyReadSlot()
{
    if(!isRefresh) return;

    QByteArray message=mySocket->readAll();

    decode(message);    // 解码

    ui->statusBar->showMessage(tr("成功接收%1字节数据.").arg(message.size()));
}

void MainWindow::errorSlot(QAbstractSocket::SocketError)
{
    QMessageBox::information(this, "show", mySocket->errorString());
    disconnectedSlot();
}

void MainWindow::on_connectBtn_clicked()
{
    QString ip=ui->IPLineEdit->text();
    QString port=ui->portLineEdit->text();

    qDebug()<<"IP:"<<ip;
    qDebug()<<"Port:"<<port;

    if(!isConnected) mySocket->connectToHost(ip, port.toInt());
    else disconnectedSlot();
}

void MainWindow::on_sendBtn_clicked()
{
    if(isConnected){
        QString str = ui->sendEdit->text();
        mySocket->write(str.toLatin1().data(), str.length());
    }
    else{
        QMessageBox::information(this, "show", "未连连.");
    }
}

void MainWindow::on_clearBtn_clicked()
{
    ui->recvEdit->clear();
}

void MainWindow::on_startBtn_clicked()
{
    if(isConnected)
    {
        if(!isStart)
        {
            isStart=true;
            QString str = "GETFRAME RUNS 0";
            mySocket->write(str.toLatin1().data(), str.length());
        }
    }
}

void MainWindow::on_stopBtn_clicked()
{
    if(isConnected)
    {
        if(isStart)
        {
            isStart=false;
            QString str = "GETFRAME STOP 0";
            mySocket->write(str.toLatin1().data(), str.length());
        }
    }
}

void MainWindow::on_refreshBtn_clicked()
{
    if(isRefresh)
    {
        isRefresh=false;
        ui->refreshBtn->setText("刷新数据");
    }
    else
    {
        isRefresh=true;
        ui->refreshBtn->setText("暂停刷新");
    }
}

int  MainWindow::decode(QByteArray message)
{
    ui->recvEdit->append(message);

    void *qMessage= message.data();
    PACKAGEHEAD headPacket;

    memcpy(&headPacket,qMessage,getPackageHeadLen());

    QString qsMsgFlag=QString(QLatin1String(headPacket.MsgFlag));

    if(qsMsgFlag != "IPCAMVR")
    {
        ui->recvEdit->append("数据校验出错！");
        return 1;
    }

    QString qsReserv=QString(QLatin1String(headPacket.szReserv));

    int nCodec=(int)headPacket.nCodec;
    unsigned int nFrameId=headPacket.nFrameId;
    int  nFrameLen=headPacket.nFrameLen;
    int  nFrameType=headPacket.nFrameType;

    qDebug()<<"MsgFlag:"<<qsMsgFlag;
    qDebug()<<"nCodec:"<<nCodec;
    qDebug()<<"nFrameId:"<<nFrameId;
    qDebug()<<"nFrameLen:"<<nFrameLen;
    qDebug()<<"nFrameType:"<<nFrameType;
    qDebug()<<"szReserv:"<<qsReserv;
    qDebug()<<" ";
}

void MainWindow::on_openMusicFileBtn_clicked()
{
    ui->statusBar->clearMessage();

    QString filename = QFileDialog::getOpenFileName(this,"Open Document",
       QDir::currentPath(),"Music fils (*.pcm)");

    if(filename.isNull()) return;

    qDebug()<<"Name:"<<filename;

    QByteArray qbFilename = filename.toLocal8Bit();
    char *qFilename = qbFilename.data();
    FILE *file=fopen(qFilename,"rb");
    if(!file)
    {
        ui->statusBar->showMessage(tr("文件打开失败!"));
        return;
    }

    fseek(file,0,SEEK_END);
    fileLength=ftell(file);
    fileBuffer=new char[fileLength+1];
    rewind(file);
    fread(fileBuffer,1,fileLength,file);
    fileBuffer[fileLength]=0;
    fclose(file);
    qDebug()<<fileLength;

    ui->playBtn->setText("暂  停");
}

void MainWindow::on_playBtn_clicked()
{
    if(!fileLength) return;

    static bool flag=true;
    if(flag)
    {
        timer->start(10);
        ui->playBtn->setText("暂  停");
    }
    else
    {
        timer->stop();
        ui->playBtn->setText("播  放");
    }
    flag=!flag;
}

/******************************************************************/
/*  PCM裸流1秒的数据量是44100×16×2/8=176400字节                      */
/*  那么10毫秒的数据量就是1764字节，所以定时器定时10毫秒                 */
/*  每隔10毫秒从数组中顺序读取1746个字节                               */
/******************************************************************/
void MainWindow::slotTimeout()
{
    static int i=0;
    if(i<fileLength/1764)
    {
        tempBuffer.append(fileBuffer+i*1764,1764);

        musicPlay(tempBuffer);
    }
    else on_playBtn_clicked();

    i++;
}

// 写入到扬声器
void MainWindow::musicPlay(QByteArray &buffer)
{
    if(buffer.length())
    {
        if(audioOutput&&audioOutput->state()!=QAudio::StoppedState&&
                audioOutput->state()!=QAudio::SuspendedState)
        {
            int chunks = audioOutput->bytesFree()/audioOutput->periodSize();
            while (chunks)
            {
                if (buffer.length() >= audioOutput->periodSize())
                {
                    //写入到扬声器
                    streamOut->write(buffer.data(),audioOutput->periodSize());
                    buffer = buffer.mid(audioOutput->periodSize());
                }
                else
                {
                    //写入到扬声器
                    streamOut->write(buffer);
                    buffer.clear();
                    break;
                }
                --chunks;
            }
        }
    }
}

void MainWindow::on_openFileBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,"Open Document",
       QDir::currentPath(),"Music fils (*.pcm *.mp3 *.ogg *.wav *.flac)");

    if(filename.isNull()) return;

    qDebug()<<"fileName:"<<filename;

    ui->nameLab->setText(filename.split('/').last());

    QString name=filename.split('.').last();
    TStreamFormat fileType=sfAutodetect;

    if(name == "mp3") fileType=sfMp3;
    else if(name == "ogg") fileType=sfOgg;
    else if(name == "wav") fileType=sfWav;
    else if(name == "pcm") fileType=sfPCM;
    else if(name == "flac") fileType=sfFLAC;

    QByteArray qbFilename = filename.toLocal8Bit();
    char *qFilename = qbFilename.data();

    // open file
    int result = player->OpenFile(qFilename, fileType);
    if(result == 0)
    {
        qDebug()<<"Error:"<<player->GetError();
        player->Release();
        return;
    }

    TStreamInfo info;
    player->GetStreamInfo(&info);

    QString strTime = QString().sprintf("/ %02d:%02d:%02d",
                                        info.Length.hms.hour,
                                        info.Length.hms.minute,
                                        info.Length.hms.second);
    ui->tolTimeLab->setText(strTime);

    // set graph type to AREA, left channel on top
    player->SetFFTGraphParam(gpGraphType, gtAreaLeftOnTop);
    // set linear scale
    player->SetFFTGraphParam(gpHorizontalScale, gsLinear);

    player->Play();

    ui->playMusicBtn->setText("暂  停");

    myTimer->start(200);
}

void MainWindow::on_playMusicBtn_clicked()
{
    static bool flag=false;
    if(flag)
    {
        myTimer->start(200);
        ui->playMusicBtn->setText("暂  停");
        player->Play();
    }
    else
    {
        myTimer->stop();
        ui->playMusicBtn->setText("播  放");
        player->Pause();
    }
    flag=!flag;
}

void MainWindow::myTimerout()
{
    TStreamStatus status;
    player->GetStatus(&status);
    if(status.fPlay == 0)
    {
        myTimer->stop();
        return;
    }

    TStreamTime pos;
    player->GetPosition(&pos);

    QString strTime = QString().sprintf("%02d:%02d:%02d",pos.hms.hour,pos.hms.minute,pos.hms.second);
    ui->curTimeLab->setText(strTime);


    int FFTPoints = player->GetFFTGraphParam(gpFFTPoints);

    player->GetFFTData(FFTPoints,fwTriangular,
                           pnHarmonicNumber,
                           pnHarmonicFreq,
                           pnLeftAmplitude,
                           pnRightAmplitude,
                           pnLeftPhase,
                           pnRightPhase);

    for(int i = 0; i < 257; i++)
    {
        spect[i] = pnLeftAmplitude[i];
    }

    ui->spectPlot->outPutSpect(spect);
}

void MainWindow::on_pushButton_clicked()
{

}
