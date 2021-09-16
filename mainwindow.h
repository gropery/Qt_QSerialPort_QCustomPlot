#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QPainter>
#include <QPlainTextEdit>
#include "plot.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonOpenPort_clicked();           //打开串口
    void on_pushButtonClearRec_clicked();           //清除接收
    void on_pushButtonClearSend_clicked();          //清除发送
    void slot_timerUpdateLabel_timeout();           //数据更新定时器槽函数

    void on_pushButtonSingleSend_clicked();                 //tab1 单次发送按钮槽函数
    void on_checkBoxSingleSend_stateChanged(int arg1);      //tab1 定时发送选框槽函数(开启单发定时器)
    void slot_pushButtonMultiSend_n_clicked();              //tab2 数字标号单次发送按钮槽函数
    void on_checkBoxMultiSend_stateChanged(int arg1);       //tab2 定时发送选框槽函数(开启多发定时器)
    void slot_timerMultiSend_timeout();                     //多发定时器槽函数

    void on_checkBoxHexRec_stateChanged(int arg1);
    void on_checkBoxHexSend_stateChanged(int arg1);
    void slot_serialPort_readyRead();






    void on_actionPlotShow_triggered();

private:
    void changeEncodeStrAndHex(QPlainTextEdit *plainTextEdit,int arg1);

private:
    Ui::MainWindow *ui;

    // 波形绘图窗口
    Plot *plot = NULL;// 必须初始化为空，否则后面NEW判断的时候会异常结束

    QSerialPort *serialPort;

    //发送接收数量，速率计算
    long curSendNum=0;
    long curRecvNum=0;
    long lastSendNum=0;
    long lastRecvNum=0;
    long sendRate=0;
    long recvRate=0;
    //接收帧数量、速率、CRC错误
    long curRecvFrameNum=0;
    long lastRecvFrameNum=0;
    long recvFrameRate=0;
    long recvFrameCrcErrNum=0;
    long recvFrameMissNum=0;

    // 定时发送-定时器
    QTimer *timerSingleSend;
    QTimer *timerMultiSend;
    // Label数据更新-定时器
    QTimer *timerUpdateLabel;

};
#endif // MAINWINDOW_H
