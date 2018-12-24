#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化
    ui->comboBox_baudRate->setCurrentIndex(7);

    QRegExp ipRegexp = QRegExp("((2[0-4]]\\d|25[0-5]|[01]?\\d\\d?)\\.){4}");
    QRegExpValidator *ipRegExpValidator = new QRegExpValidator(ipRegexp,this);
    ui->lineEdit_localIP->setValidator(ipRegExpValidator);
    ui->lineEdit_localIP->setInputMask("000.000.000.000");
    ui->lineEdit_localIP->setText("127.0.0.1");

    QIntValidator *portValidator = new QIntValidator(0,65535,this);
    ui->lineEdit_localPort->setValidator(portValidator);
    ui->lineEdit_localPort->setText(QString::number(localPort));

    ui->lineEdit_remoteIP->setValidator(ipRegExpValidator);
    ui->lineEdit_remoteIP->setInputMask("000.000.000.000");
    ui->lineEdit_remoteIP->setText("127.0.0.1");

    ui->lineEdit_remotePort->setValidator(portValidator);
    ui->lineEdit_remotePort->setText(QString::number(remotePort));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//打开串口按钮，并设置参数
void MainWindow::on_pushButton_SerialPortOpen_clicked()
{
    if(ui->pushButton_SerialPortOpen->text() == "打开")
    {
        serialPort = new QSerialPort();
        serialPortName = ui->lineEdit_PortName->text();
        QString strBaudRate = ui->comboBox_baudRate->currentText();
        baudRate = strBaudRate.toInt();

        if(serialPort->isOpen())
        {
            serialPort->close();
        }

        serialPort->setPortName(serialPortName);
        bool bOK = serialPort->open(QIODevice::ReadWrite);
        if(!bOK)
        {
            QMessageBox::critical(this,"提示","无法打开串口，请检查是否被占用。",QMessageBox::Yes,QMessageBox::Yes);
            return;
        }
        serialPort->setBaudRate(baudRate);

        int index_parity = 0;
        index_parity = ui->comboBox_parity->currentIndex();
        int index_dataBit = 0;
        index_dataBit = ui->comboBox_dataBit->currentIndex();
        int index_stopBit = 0;
        index_stopBit = ui->comboBox_stopBit->currentIndex();

        switch(index_parity)
        {
            case 0:
                serialPort->setParity(QSerialPort::NoParity);
                break;
            case 1:
                serialPort->setParity(QSerialPort::OddParity);
                break;
            case 2:
                serialPort->setParity(QSerialPort::EvenParity);
                break;
            case 3:
                serialPort->setParity(QSerialPort::MarkParity);
                break;
            case 4:
                serialPort->setParity(QSerialPort::SpaceParity);
                break;
            default:
                serialPort->setParity(QSerialPort::NoParity);
                break;
        }

        switch(index_dataBit)
        {
            case 0:
                serialPort->setDataBits(QSerialPort::Data8);
                break;
            case 1:
                serialPort->setDataBits(QSerialPort::Data7);
                break;
            case 2:
                serialPort->setDataBits(QSerialPort::Data6);
                break;
            case 3:
                serialPort->setDataBits(QSerialPort::Data5);
                break;
            default:
                serialPort->setDataBits(QSerialPort::Data8);
                break;
        }

        switch(index_stopBit)
        {
            case 0:
                serialPort->setStopBits(QSerialPort::OneStop);
                break;
            case 1:
                serialPort->setStopBits(QSerialPort::OneAndHalfStop);
                break;
            case 2:
                serialPort->setStopBits(QSerialPort::TwoStop);
                break;
            default:
                serialPort->setStopBits(QSerialPort::OneStop);
                break;
        }

        serialPort->setFlowControl(QSerialPort::NoFlowControl);

        connect(serialPort,SIGNAL(readyRead()),this,SLOT(serialPort_rev()));
        ui->pushButton_SerialPortOpen->setText("关闭");
    }
    else
    {
        if(serialPort->isOpen())
        {
            serialPort->close();
        }
        delete serialPort;
        serialPort = NULL;

        ui->pushButton_SerialPortOpen->setText("打开");
    }

}

//串口接收数据槽函数
void MainWindow::serialPort_rev()
{
    int byteLen = serialPort->bytesAvailable();

    if(byteLen < 0)
    {
        return;
    }

    buffer_serialPort += serialPort->readAll();


    //接收是否转向文件
    if(ui->checkBox_serialPortRevToFile->isChecked())
    {
        //接收转向文件
        //16进制
        if(ui->checkBox_serialPortRevHex->isChecked())
        {
            QByteArray bufferHex = buffer_serialPort.toHex();
            QString str_buffer = QString(bufferHex.toUpper());
            for(int i = str_buffer.count(); i > 0; i = i-2)
            {
                str_buffer.insert(i," ");
            }

            QFile fs(serialPortRevDesFile);
            if(fs.open(QFile::Append))
            {
                QTextStream ts(&fs);

                ts<<str_buffer;

                ts.flush();
            }
            fs.close();

        }
        //普通处理
        else
        {
            QString str_buffer = QString(buffer_serialPort);
            QFile fs(serialPortRevDesFile);
            if(fs.open(QFile::Append))
            {
                QTextStream ts(&fs);

                ts<<str_buffer;

                ts.flush();
            }
            fs.close();
        }

        //写完文件后直接返回
        int allCount = ui->label_serialPortRevCount->text().toInt();
        allCount = allCount + buffer_serialPort.count();
        ui->label_serialPortRevCount->setText(QString::number(allCount));
        buffer_serialPort.clear();
        return;
    }

    //接收不转向文件时，显示在界面上
    //16进制
    if(ui->checkBox_serialPortRevHex->isChecked())
    {
        QByteArray bufferHex = buffer_serialPort.toHex();

        QString str_buffer = QString(bufferHex.toUpper());
        for(int i = str_buffer.count(); i > 0; i = i-2)
        {
            str_buffer.insert(i," ");
        }
        ui->textEdit_SerialPortRev->moveCursor(QTextCursor::End);
        ui->textEdit_SerialPortRev->insertPlainText(str_buffer);
        ui->textEdit_SerialPortRev->moveCursor(QTextCursor::End);

    }
    //普通处理
    else
    {
        QString str_buffer = QString(buffer_serialPort);
        ui->textEdit_SerialPortRev->moveCursor(QTextCursor::End);
        ui->textEdit_SerialPortRev->insertPlainText(str_buffer);
        ui->textEdit_SerialPortRev->moveCursor(QTextCursor::End);


    }




    int allCount = ui->label_serialPortRevCount->text().toInt();
    allCount = allCount + buffer_serialPort.count();
    ui->label_serialPortRevCount->setText(QString::number(allCount));
    buffer_serialPort.clear();

}

//串口发送按钮
void MainWindow::on_pushButton_clicked()
{
    if(ui->pushButton_SerialPortOpen->text() == "打开")
    {
        QMessageBox::critical(this,"提示","串口未打开。",QMessageBox::Yes,QMessageBox::Yes);
        return;
    }
    QString command = ui->lineEdit_serialPortSend->text();
    if(ui->checkBox_endLine->isChecked())
    {
        command += endLine;
    }

    if(ui->checkBox_serialPortSendHex->isChecked())
    {
        QStringList commadList = command.split(' ');
        QByteArray byteArray;
        byteArray.resize(commadList.count());
        bool ok = false;
        for(int i = 0; i < commadList.count(); i++)
        {
            byteArray[i] = commadList.at(i).toInt(&ok,16);
        }

        serialPort->write(byteArray);
        QThread::msleep(30);
    }
    else
    {
        serialPort->write(command.toUtf8().data());
        QThread::msleep(30);
    }


    int allCount = ui->label_serialPortSendCount->text().toInt();
    allCount = allCount + command.toUtf8().count();
    ui->label_serialPortSendCount->setText(QString::number(allCount));
}

//串口接收转向文件
void MainWindow::on_checkBox_serialPortRevToFile_clicked()
{
    if(ui->checkBox_serialPortRevToFile->isChecked())
    {
        QFileDialog *fileDialog = new QFileDialog(this);
        fileDialog->setWindowTitle("选择一个文件");
        fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog->setFileMode(QFileDialog::AnyFile);
        fileDialog->setViewMode(QFileDialog::Detail);
        fileDialog->setDirectory("/home/");

        QString desFile = "";
        if(fileDialog->exec() == QDialog::Accepted)
        {
            desFile = fileDialog->selectedFiles()[0];

        }
        if(desFile == "")
        {
            ui->checkBox_serialPortRevToFile->setChecked(false);
            return;
        }
        serialPortRevDesFile = desFile;

        ui->textEdit_SerialPortRev->append("接收转向文件");
        ui->textEdit_SerialPortRev->append(desFile);
        ui->textEdit_SerialPortRev->setEnabled(false);
    }
    else
    {
        ui->textEdit_SerialPortRev->clear();
        ui->textEdit_SerialPortRev->setEnabled(true);
        serialPortRevDesFile.clear();
    }
}

//清除串口接受区
void MainWindow::on_pushButton_serialPortRevClear_clicked()
{
    ui->textEdit_SerialPortRev->clear();
}

//串口计数复位
void MainWindow::on_pushButton_serialPortResetCount_clicked()
{
    ui->label_serialPortRevCount->setText("0");
    ui->label_serialPortSendCount->setText("0");
}


//网口开始监听按钮
void MainWindow::on_pushButton_netStartListen_clicked()
{
    if(ui->pushButton_netStartListen->text() == "开始监听")
    {
        strLocalIP = ui->lineEdit_localIP->text();
        localPort = ui->lineEdit_localPort->text().toInt();

        QHostAddress ipAddress;
        ipAddress.setAddress(strLocalIP);

        udpSocket = new QUdpSocket(this);
        bool b = udpSocket->bind(ipAddress,localPort);

        connect(udpSocket,SIGNAL(readyRead()),this,SLOT(netPort_Rev()));
        ui->pushButton_netStartListen->setText("停止监听");
    }
    else
    {
        delete udpSocket;
        udpSocket = NULL;

        ui->pushButton_netStartListen->setText("开始监听");
    }
}

void MainWindow::netPort_Rev()
{
    while(udpSocket->hasPendingDatagrams())
    {
        buffer_netPort.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(buffer_netPort.data(), buffer_netPort.size());

        //接收是否转向文件
        if(ui->checkBox_netPortRevToFile->isChecked())
        {
            //16进制
            if(ui->checkBox_netPortRevHex->isChecked())
            {
                QByteArray bufferHex = buffer_netPort.toHex();
                QString str_buffer = QString(bufferHex.toUpper());
                for(int i = str_buffer.count(); i > 0; i = i-2)
                {
                    str_buffer.insert(i," ");
                }

                QFile fs(netPortRevDesFile);
                if(fs.open(QFile::Append))
                {
                    QTextStream ts(&fs);

                    ts<<str_buffer;

                    ts.flush();
                }
                fs.close();
            }
            //普通处理
            else
            {
                QString str_buffer = QString(buffer_netPort);
                QFile fs(netPortRevDesFile);
                if(fs.open(QFile::Append))
                {
                    QTextStream ts(&fs);

                    ts<<str_buffer;

                    ts.flush();
                }
                fs.close();
            }
        }
        else
        {
            //16进制
            if(ui->checkBox_netPortRevHex->isChecked())
            {
                QByteArray bufferHex = buffer_netPort.toHex();
                QString str_buffer = QString(bufferHex.toUpper());
                for(int i = str_buffer.count(); i > 0; i = i-2)
                {
                    str_buffer.insert(i," ");
                }

                ui->textEdit_netRev->moveCursor(QTextCursor::End);
                ui->textEdit_netRev->insertPlainText(str_buffer);
                ui->textEdit_netRev->moveCursor(QTextCursor::End);
            }
            //普通处理
            else
            {
                QString str_buffer = QString(buffer_netPort);
                ui->textEdit_netRev->moveCursor(QTextCursor::End);
                ui->textEdit_netRev->insertPlainText(str_buffer);
                ui->textEdit_netRev->moveCursor(QTextCursor::End);
            }
        }


        int allCount = ui->label_netPortRevCount->text().toInt();
        allCount = allCount + buffer_netPort.count();
        ui->label_netPortRevCount->setText(QString::number(allCount));
        buffer_netPort.clear();
    }


}

//清除网口接受区
void MainWindow::on_pushButton_netRevClear_clicked()
{
    ui->textEdit_netRev->clear();
}

//网口接收转向文件
void MainWindow::on_checkBox_netPortRevToFile_clicked()
{
    if(ui->checkBox_netPortRevToFile->isChecked())
    {
        QFileDialog *fileDialog = new QFileDialog(this);
        fileDialog->setWindowTitle("选择一个文件");
        fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog->setFileMode(QFileDialog::AnyFile);
        fileDialog->setViewMode(QFileDialog::Detail);
        fileDialog->setDirectory("/home/");

        QString desFile = "";
        if(fileDialog->exec() == QDialog::Accepted)
        {
            desFile = fileDialog->selectedFiles()[0];

        }
        netPortRevDesFile = desFile;

        if(desFile == "")
        {
            ui->checkBox_netPortRevToFile->setChecked(false);
            return;
        }

        ui->textEdit_netRev->append("接收转向文件");
        ui->textEdit_netRev->append(desFile);
        ui->textEdit_netRev->setEnabled(false);
    }
    else
    {
        ui->textEdit_netRev->clear();
        ui->textEdit_netRev->setEnabled(true);
        netPortRevDesFile.clear();
    }
}

//网口计数复位
void MainWindow::on_pushButton_netPortResetCount_clicked()
{
    ui->label_netPortRevCount->setText("0");
    ui->label_netPortSendCount->setText("0");
}

//串口发送按钮
void MainWindow::on_pushButton_netPortSend_clicked()
{
    if(ui->pushButton_netStartListen->text() == "开始监听")
    {
        QMessageBox::critical(this,"提示","网络未连接。",QMessageBox::Yes,QMessageBox::Yes);
        return;
    }

    QString command = ui->lineEdit_netPortSend->text();
    if(ui->checkBox_netEndLine->isChecked())
    {
        command += endLine;
    }

    strRemoteIP = ui->lineEdit_remoteIP->text();
    remotePort = ui->lineEdit_remotePort->text().toInt();

    int ilen = 0;

    if(ui->checkBox_netPortSendHex->isChecked())
    {
        QStringList commadList = command.split(' ');
        QByteArray byteArray;
        byteArray.resize(commadList.count());
        bool ok = false;
        for(int i = 0; i < commadList.count(); i++)
        {
            byteArray[i] = commadList.at(i).toInt(&ok,16);
        }

        ilen = udpSocket->writeDatagram(byteArray,QHostAddress(strRemoteIP),remotePort);
        QThread::msleep(30);
    }
    else
    {
        ilen = udpSocket->writeDatagram(command.toUtf8(),QHostAddress(strRemoteIP),remotePort);
        QThread::msleep(30);
    }


    int allCount = ui->label_netPortSendCount->text().toInt();
    allCount = allCount + ilen;
    ui->label_netPortSendCount->setText(QString::number(allCount));
}
