#ifndef OPENSERIALPORT_H
#define OPENSERIALPORT_H

#include <QComboBox>
#include <QMouseEvent>
#include <QSerialPort>
#include <QSerialPortInfo>

class openSerialPort : public QComboBox
{
    Q_OBJECT
public:
    openSerialPort(QWidget *parent);

    // 重写鼠标点击事件
    void mousePressEvent(QMouseEvent *event);

signals:

private:
    void scanActivePort();

};

#endif // OPENSERIALPORT_H
