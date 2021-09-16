#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt Serial Plot");

    connect(ui->pushButtonMultiSend_1,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_2,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_3,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_4,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_5,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_6,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_7,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_8,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_9,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));
    connect(ui->pushButtonMultiSend_10,SIGNAL(clicked(bool)),this,SLOT(slot_pushButtonMultiSend_n_clicked()));

    // 单次发送选项卡-定时发送-定时器
    timerSingleSend = new QTimer;
    timerSingleSend->setInterval(1000);// 设置默认定时时长1000ms
    connect(timerSingleSend, &QTimer::timeout, this, [=](){
        on_pushButtonSingleSend_clicked();
    });

    // 多次发送选项卡-定时发送-定时器
    timerMultiSend = new QTimer;
    timerMultiSend->setInterval(1000);// 设置默认定时时长1000ms
    connect(timerMultiSend, &QTimer::timeout, this, [=](){
        slot_timerMultiSend_timeout();
    });

    // Label数据更新-定时器
    timerUpdateLabel = new QTimer;
    timerUpdateLabel->start(1000);
    connect(timerUpdateLabel, &QTimer::timeout, this, [=](){
        slot_timerUpdateLabel_timeout();
    });

    // 新建串口对象
    serialPort = new QSerialPort(this);
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(slot_serialPort_readyRead()));

    qDebug()<<"start..."<<endl;

}

MainWindow::~MainWindow()
{
    delete ui;
}

//配置、打开串口
void MainWindow::on_pushButtonOpenPort_clicked()
{
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity   checkBits;

    // 获取串口波特率
    baudRate = (QSerialPort::BaudRate)ui->comboBoxBaudRate->currentText().toUInt();
    // 获取串口数据位
    dataBits = (QSerialPort::DataBits)ui->comboBoxDataBits->currentText().toUInt();
    // 获取串口停止位
    if(ui->comboBoxStopBits->currentText() == "1"){
        stopBits = QSerialPort::OneStop;
    }else if(ui->comboBoxStopBits->currentText() == "1.5"){
        stopBits = QSerialPort::OneAndHalfStop;
    }else if(ui->comboBoxStopBits->currentText() == "2"){
        stopBits = QSerialPort::TwoStop;
    }else{
        stopBits = QSerialPort::OneStop;
    }
    // 获取串口奇偶校验位
    if(ui->comboBoxParity->currentText() == "无"){
        checkBits = QSerialPort::NoParity;
    }else if(ui->comboBoxParity->currentText() == "奇校验"){
        checkBits = QSerialPort::OddParity;
    }else if(ui->comboBoxParity->currentText() == "偶校验"){
        checkBits = QSerialPort::EvenParity;
    }else{
        checkBits = QSerialPort::NoParity;
    }
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setStopBits(stopBits);
    serialPort->setParity(checkBits);
    // 匹配带有串口设备信息的文本
    QString serialPortName = ui->comboBoxSerialPort->currentText();
    serialPortName = serialPortName.section(':', 0, 0);//spTxt.mid(0, spTxt.indexOf(":"));
    serialPort->setPortName(serialPortName);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if(ui->pushButtonOpenPort->text() == "打开串口"){
        if(serialPort->open(QIODevice::ReadWrite) == true){
            ui->pushButtonOpenPort->setText("关闭串口");
            // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
            ui->comboBoxSerialPort->setEnabled(false);
            ui->comboBoxBaudRate->setEnabled(false);
            ui->comboBoxStopBits->setEnabled(false);
            ui->comboBoxDataBits->setEnabled(false);
            ui->comboBoxParity->setEnabled(false);
        }else{
            QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n\r\n该串口可能被占用，请选择正确的串口\r\n或者波特率过高，超出硬件支持范围");
        }
    }else{
        serialPort->close();
        ui->pushButtonOpenPort->setText("打开串口");
        // 端口号下拉框恢复可选
        ui->comboBoxSerialPort->setEnabled(true);
        ui->comboBoxBaudRate->setEnabled(true);
        ui->comboBoxStopBits->setEnabled(true);
        ui->comboBoxDataBits->setEnabled(true);
        ui->comboBoxParity->setEnabled(true);
    }
}

//清除接收窗口
void MainWindow::on_pushButtonClearRec_clicked()
{
    //(1)清空plainTextEditRec内容
    ui->plainTextEditRec->clear();
    //(2)清空字节计数
    curSendNum = 0;
    curRecvNum = 0;
    lastSendNum = 0;
    lastRecvNum = 0;
    curRecvFrameNum = 0;
    lastRecvFrameNum = 0;
    recvFrameCrcErrNum = 0;
}

//清除发送窗口
void MainWindow::on_pushButtonClearSend_clicked()
{
    //(1)清空plainTextEditSend内容
    ui->plainTextEditSend->clear();
    //(2)清空发送字节计数
    curSendNum = 0;
    lastSendNum = 0;
}

//定时器槽函数timeout，1s 数据更新
void MainWindow::slot_timerUpdateLabel_timeout(void)
{
    //当前总计数-上次总结存暂存
    sendRate = curSendNum - lastSendNum;
    recvRate = curRecvNum - lastRecvNum;
    recvFrameRate = curRecvFrameNum - lastRecvFrameNum;
    //设置label
    ui->labelSendRate->setText(tr("Byte/s: %1").arg(sendRate));
    ui->labelRecvRate->setText(tr("Byte/s: %1").arg(recvRate));
    ui->labelRecvFrameRate->setText(tr("FPS: %1").arg(recvFrameRate));
    //暂存当前总计数
    lastSendNum = curSendNum;
    lastRecvNum = curRecvNum;
    lastRecvFrameNum = curRecvFrameNum;

    //更新label
    ui->labelSendNum->setText(tr("S: %1").arg(curSendNum));
    ui->labelRecvNum->setText(tr("R: %1").arg(curRecvNum));
    ui->labelRecvFrameNum->setText(tr("FNum: %1").arg(curRecvFrameNum));
    ui->labelRecvFrameErrNum->setText(tr("FErr: %1").arg(recvFrameCrcErrNum));
    ui->labelRecvFrameMissNum->setText(tr("FMiss: %1").arg(recvFrameMissNum));
}

//tab1 单次发送按钮槽函数
void MainWindow::on_pushButtonSingleSend_clicked()
{
    QString strSendData = ui->plainTextEditSend->toPlainText();
    QByteArray baSendData;

    if(ui->checkBoxHexSend->checkState() == false){
        // 字符串形式发送
        //baSendData = strSendData.toUtf8();                            // Unicode编码输出
        baSendData = strSendData.toLocal8Bit();                         // GB2312编码输出,用以兼容大多数单片机
    }else{
        // 16进制发送
        //baSendData = QByteArray::fromHex(strSendData.toUtf8());       // Unicode编码输出
        baSendData = QByteArray::fromHex(strSendData.toLocal8Bit());    // GB2312编码输出
    }

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int ret = serialPort->write(baSendData);

    if(ret > 0)
        curSendNum += ret;
}

//tab1 定时发送选框槽函数(开启单发定时器)
void MainWindow::on_checkBoxSingleSend_stateChanged(int arg1)
{
    int TimerInterval = ui->lineEditSingleSend->text().toInt();

    // 获取复选框状态，未选为0，选中为2
    if(arg1 == 0){
        timerSingleSend->stop();
        ui->lineEditSingleSend->setEnabled(true);
    }else{
        // 对输入的值做限幅，小于20ms会弹出对话框提示
        if(TimerInterval >= 20){
            timerSingleSend->start(TimerInterval);    // 设置定时时长
            ui->lineEditSingleSend->setEnabled(false);
        }else{
            ui->checkBoxSingleSend->setCheckState(Qt::Unchecked);
            QMessageBox::critical(this, "错误提示", "定时发送的最小间隔为 20ms\r\n请确保输入的值 >=20");
        }
    }
}

//tab2 数字标号单次发送按钮槽函数
void MainWindow::slot_pushButtonMultiSend_n_clicked()
{
    //获取信号发送者
    QPushButton *pushButton = qobject_cast<QPushButton *>(sender());
    QString strSendData;
    QByteArray baSendData;

    if(pushButton == ui->pushButtonMultiSend_1)
        strSendData = ui->lineEditMultiSend_1->text();
    if(pushButton == ui->pushButtonMultiSend_2)
        strSendData = ui->lineEditMultiSend_2->text();
    if(pushButton == ui->pushButtonMultiSend_3)
        strSendData = ui->lineEditMultiSend_3->text();
    if(pushButton == ui->pushButtonMultiSend_4)
        strSendData = ui->lineEditMultiSend_4->text();
    if(pushButton == ui->pushButtonMultiSend_5)
        strSendData = ui->lineEditMultiSend_5->text();
    if(pushButton == ui->pushButtonMultiSend_6)
        strSendData = ui->lineEditMultiSend_6->text();
    if(pushButton == ui->pushButtonMultiSend_7)
        strSendData = ui->lineEditMultiSend_7->text();
    if(pushButton == ui->pushButtonMultiSend_8)
        strSendData = ui->lineEditMultiSend_8->text();
    if(pushButton == ui->pushButtonMultiSend_9)
        strSendData = ui->lineEditMultiSend_9->text();
    if(pushButton == ui->pushButtonMultiSend_10)
        strSendData = ui->lineEditMultiSend_10->text();

    // 只16进制发送
    baSendData = QByteArray::fromHex(strSendData.toLocal8Bit());    // GB2312编码输出

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int ret = serialPort->write(baSendData);

    if(ret > 0)
        curSendNum += ret;
}

//tab2 定时发送选框槽函数(开启多发定时器)
void MainWindow::on_checkBoxMultiSend_stateChanged(int arg1)
{
    int TimerInterval = ui->lineEditMultiSend->text().toInt();

    // 获取复选框状态，未选为0，选中为2
    if(arg1 == 0){
        timerMultiSend->stop();
        ui->lineEditMultiSend->setEnabled(true);
    }else{
        // 对输入的值做限幅，小于20ms会弹出对话框提示
        if(TimerInterval >= 20){
            timerMultiSend->start(TimerInterval);    // 设置定时时长
            ui->lineEditMultiSend->setEnabled(false);
        }else{
            ui->checkBoxSingleSend->setCheckState(Qt::Unchecked);
            QMessageBox::critical(this, "错误提示", "定时发送的最小间隔为 20ms\r\n请确保输入的值 >=20");
        }
    }
}


void MainWindow::slot_timerMultiSend_timeout()
{
    QString strSendData;
    QByteArray baSendData;

    if(ui->checkBoxMultiSend_1->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_1->text();
    if(ui->checkBoxMultiSend_2->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_2->text();
    if(ui->checkBoxMultiSend_3->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_3->text();
    if(ui->checkBoxMultiSend_4->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_4->text();
    if(ui->checkBoxMultiSend_5->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_5->text();
    if(ui->checkBoxMultiSend_6->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_6->text();
    if(ui->checkBoxMultiSend_7->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_7->text();
    if(ui->checkBoxMultiSend_8->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_8->text();
    if(ui->checkBoxMultiSend_9->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_9->text();
    if(ui->checkBoxMultiSend_10->checkState()==Qt::Checked)
        strSendData += ui->lineEditMultiSend_10->text();

    // 只16进制发送
    baSendData = QByteArray::fromHex(strSendData.toLocal8Bit());    // GB2312编码输出

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int ret = serialPort->write(baSendData);

    if(ret > 0)
        curSendNum += ret;
}


void MainWindow::on_checkBoxHexSend_stateChanged(int arg1)
{
    changeEncodeStrAndHex(ui->plainTextEditSend, arg1);
}

void MainWindow::on_checkBoxHexRec_stateChanged(int arg1)
{
    changeEncodeStrAndHex(ui->plainTextEditRec, arg1);
}

void MainWindow::changeEncodeStrAndHex(QPlainTextEdit *plainTextEdit, int arg1)
{
    QString strRecvData1 = plainTextEdit->toPlainText();
    QByteArray baRecvData;
    QString strRecvData2;

    // 获取多选框状态，未选为0，选中为2
    if(arg1 == 0){
        // 为0时，多选框未勾选，接收区先前接收的16进制数据转换为asc2字符串格式
        //ba = QByteArray::fromHex(strRecvData1.toUtf8());               //Unicode(UTF8)编码，
        //str = QString(ba);//QString::fromUtf8(ba);                     //这里bytearray转为string时也可直接赋值，或者QString构造，因为QString存储的UTF-16编码
        baRecvData = QByteArray::fromHex(strRecvData1.toLocal8Bit());    //ANSI(GB2132)编码
        strRecvData2 =QString::fromLocal8Bit(baRecvData);                //这里bytearray转为string一定要指定编码
    }
    else{
        // 不为0时，多选框勾选，接收区先前接收asc2字符串转换为16进制显示
        //QByteArray ba = strRecvData1.toUtf8().toHex(' ').toUpper();     // Unicode(UTF8)编码(中国：E4 B8 AD E5 9B BD)
        baRecvData = strRecvData1.toLocal8Bit().toHex(' ').toUpper();     // ANSI(GB2132)编码(中国:D6 D0 B9 FA)
        strRecvData2 = QString(baRecvData);                               //这里由16进制的bytearray转为string所有编码统一，所以可以直接构造，并没有歧义
    }

    // 文本控件清屏，显示新文本
    plainTextEdit->clear();
    plainTextEdit->insertPlainText(strRecvData2);
    // 移动光标到文本结尾
    plainTextEdit->moveCursor(QTextCursor::End);
}

void MainWindow::slot_serialPort_readyRead(void)
{
    QByteArray baRecvData = serialPort->readAll();
    QString strRecvData;

    curRecvNum += baRecvData.size();

    if(ui->checkBoxHexRec->checkState() == false){
        // 字符编码显示
        //QString str = QString::fromUtf8(baRecvData);
        strRecvData = QString::fromLocal8Bit(baRecvData);   // GB2312编码输入
    }else{
        // 16进制显示，用空格分隔，转换为大写
        QByteArray ba = baRecvData.toHex(' ').toUpper();
        strRecvData = QString(ba);
    }

    // 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
    ui->plainTextEditRec->insertPlainText(strRecvData);
    // 移动光标到文本结尾
    ui->plainTextEditRec->moveCursor(QTextCursor::End);
}
