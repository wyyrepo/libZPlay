#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QTcpSocket>
#include <QAudioOutput>
#include <QByteArray>
#include <QTimer>
#include "libzplay.h"

using namespace libZPlay;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connectBtn_clicked();

private:
    Ui::MainWindow *ui;

private slots:
    void connectedSlot();
    void disconnectedSlot();
    void readyReadSlot();
    void errorSlot(QAbstractSocket::SocketError);

    void on_sendBtn_clicked();

    void on_clearBtn_clicked();

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void on_refreshBtn_clicked();

    void on_playBtn_clicked();

private:
    bool isConnected;
    bool isStart;
    bool isRefresh;

    int packageHeadLen;

    // Package Head struct
    typedef struct _PACKAGE_HEAD_
    {
        char MsgFlag[8];
        char nCodec;                // 0 -- H264(video) / 1 -- G711(audio)
        unsigned int nFrameId;
        int  nFrameLen;
        int  nFrameType;
        char szReserv[20];
    } PACKAGEHEAD;

    void setPackageHeadLen() {packageHeadLen=sizeof(PACKAGEHEAD); }
    int  getPackageHeadLen() {return packageHeadLen;}

    QTcpSocket *mySocket;

    int decode(QByteArray message);

// 音频解码
private:
    QAudioOutput* audioOutput;
    QIODevice * streamOut;
    QByteArray tempBuffer;
    QTimer *timer;

    char *fileBuffer;
    int fileLength;

    QTimer *myTimer;
    ZPlay *player;

    int  pnHarmonicNumber[512];
    int  pnHarmonicFreq[257];
    int  pnLeftAmplitude[257];
    int  pnRightAmplitude[257];
    int  pnLeftPhase[257];
    int  pnRightPhase[257];

    void musicPlay(QByteArray &buffer);

private slots:
    void slotTimeout();
    void myTimerout();
    void on_openMusicFileBtn_clicked();

    void on_pushButton_clicked();

    void on_openFileBtn_clicked();

    void on_playMusicBtn_clicked();

private:
    QVector<float>spect;
};

#endif // MAINWINDOW_H
