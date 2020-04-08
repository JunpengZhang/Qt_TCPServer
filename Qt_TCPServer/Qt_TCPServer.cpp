#include "Qt_TCPServer.h"

Qt_TCPServer::Qt_TCPServer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    tcpServer = new QTcpServer(this);
    ui.IPEdit->setText(QNetworkInterface().allAddresses().at(1).toString());   //获取本地IP
    ui.connectBtn->setEnabled(true);
    ui.sendBtn->setEnabled(false);

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(NewConnectionSlot()));
}

void Qt_TCPServer::NewConnectionSlot()
{
    //通过nextPendingConnection()获得连接过来的客户端信息，
    //取到peerAddress和peerPort后显示在comboBox(cbxConnection)上，
    //并将客户端的readyRead()信号连接到服务器端自定义的读数据槽函数ReadData()上。
    //将客户端的disconnected()信号连接到服务器端自定义的槽函数disconnectedSlot()上。
    currentClient = tcpServer->nextPendingConnection();//下一个挂起的连接
    tcpClient.append(currentClient);
    ui.comboBox->addItem(tr("%1:%2").arg(currentClient->peerAddress().toString().split("::ffff:")[1]).arg(currentClient->peerPort()));
    
    connect(currentClient, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(currentClient, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
}

Qt_TCPServer::~Qt_TCPServer()
{
   
}

void Qt_TCPServer::disconnectedSlot()
{
    //由于disconnected信号并未提供SocketDescriptor，所以需要遍历寻找
    for (int i = 0; i < tcpClient.length(); i++)
    {
        if (tcpClient[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // 删除存储在combox中的客户端信息
            ui.comboBox->removeItem(ui.comboBox->findText(tr("%1:%2")\
                .arg(tcpClient[i]->peerAddress().toString().split("::ffff:")[1])\
                .arg(tcpClient[i]->peerPort())));
            // 删除存储在tcpClient列表中的客户端信息
            tcpClient[i]->destroyed();
            tcpClient.removeAt(i);
        }
    }
}

void Qt_TCPServer::readData()
{
    // 由于readyRead信号并未提供SocketDecriptor，所以需要遍历所有客户端
    for (int i = 0; i < tcpClient.length(); i++)
    {
        QByteArray buffer = tcpClient[i]->readAll();
        if (buffer.isEmpty())    continue;

        static QString IP_Port, IP_Port_Pre;
        IP_Port = tr("[%1:%2]:").arg(tcpClient[i]->peerAddress().toString().split("::ffff:")[1])\
            .arg(tcpClient[i]->peerPort());

        // 若此次消息的地址与上次不同，则需显示此次消息的客户端地址
        if (IP_Port != IP_Port_Pre)
            ui.recvEdit->append(IP_Port);
        ui.recvEdit->append(buffer);
        //更新ip_port
        IP_Port_Pre = IP_Port;
    }
}

void Qt_TCPServer::on_connectBtn_clicked()
{
    //a）建立TCP连接的函数：void connectToHost(const QHostAddress & address, quint16 port, OpenMode openMode = ReadWrite)是从QAbstractSocket继承下来的public function，同时它又是一个virtual function。作用为：Attempts to make a connection to address on port port。
    //b）等待TCP连接成功的函数：bool waitForConnected(int msecs = 30000)同样是从QAbstractSocket继承下来的public function，同时它又是一个virtual function。作用为：Waits until the socket is connected, up to msecs milliseconds.If the connection has been established, this function returns true; otherwise it returns false.In the case where it returns false, you can call error() to determine the cause of the error.
    //上述代码中，edtIP, edtPort是ui上的两个lineEditor，用来填写服务器IP和端口号。btnConnect是“连接 / 断开”复用按钮，btnSend是向服务器发送数据的按钮，只有连接建立之后，才将其setEnabled。
    if (ui.connectBtn->text() == "监听")
    {
        bool ok = tcpServer->listen(QHostAddress::Any, ui.portNumEdit->text().toInt());
        if (ok)
        {
            ui.connectBtn->setText("断开");
            ui.sendBtn->setEnabled(true);
        }
    }
    else
    {
        for (int i = 0; i < tcpClient.length(); i++)
        {
            tcpClient[i]->disconnectFromHost();
            bool ok = tcpClient[i]->waitForDisconnected(1000);
            if (!ok)
            {
                ui.connectBtn->setText("连接");
                ui.sendBtn->setEnabled(false);
            }
            tcpClient.removeAt(i);//从保存的客户端列表中去除   
        }
        tcpServer->close();//不再监听端口
        ui.connectBtn->setText("监听");
    }
    
   }

void Qt_TCPServer::on_clearBtn_clicked()
{
    ui.recvEdit->clear();
}

void Qt_TCPServer::on_sendBtn_clicked()
{
    QString data = ui.sendEdit->toPlainText();
    if (data == "") return;
    //全部连接
    if (ui.comboBox->currentIndex() == 0)
    {
        for (int i = 0; i < tcpClient.length(); i++)
            tcpClient[i]->write(data.toLatin1());  //qt5除去了.toAscii()
    }
    else
    {
        //指定连接
        QString clientIP = ui.comboBox->currentText().split(":")[0];
        int clientPort = ui.comboBox->currentText().split(":")[1].toInt();
        for (int i = 0; i < tcpClient.length(); i++)
        {
            if (tcpClient[i]->peerAddress().toString().split("::ffff:")[1] == clientIP && tcpClient[i]->peerPort() == clientPort)
            {
                tcpClient[i]->write(data.toLatin1());
                return; //ip:port唯一，无需继续检索
            }
        }

    }
    
}


