#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QMessageBox>
#include <QIcon>
#include <QThread>
#include <QUdpSocket>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString endLine = "\r\n";

    //串口对象
    QSerialPort *serialPort = NULL;
    QString serialPortName;
    //波特率
    int baudRate = 0;

    //缓存区
    QByteArray buffer_serialPort;
    //接收转向的文件
    QString serialPortRevDesFile = "";

    //UDP
    QUdpSocket *udpSocket;

    QString strLocalIP;
    qint16 localPort = 8080;

    QString strRemoteIP = "127.0.0.1";
    qint16 remotePort = 8080;

    //缓存区
    QByteArray buffer_netPort;
    //接收转向的文件
    QString netPortRevDesFile = "";

private slots:

    void serialPort_rev();

    void on_pushButton_clicked();

    void on_pushButton_SerialPortOpen_clicked();

    void on_pushButton_serialPortRevClear_clicked();

    void on_pushButton_netStartListen_clicked();

    void on_pushButton_serialPortResetCount_clicked();

    void on_checkBox_serialPortRevToFile_clicked();

    void netPort_Rev();

    void on_pushButton_netRevClear_clicked();

    void on_checkBox_netPortRevToFile_clicked();

    void on_pushButton_netPortResetCount_clicked();

    void on_pushButton_netPortSend_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
