#include "Qt_TCPServer.h"

Qt_TCPServer::Qt_TCPServer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    tcpServer = new QTcpServer(this);
    ui.IPEdit->setText(QNetworkInterface().allAddresses().at(1).toString());   //��ȡ����IP
    ui.connectBtn->setEnabled(true);
    ui.sendBtn->setEnabled(false);

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(NewConnectionSlot()));
}

void Qt_TCPServer::NewConnectionSlot()
{
    //ͨ��nextPendingConnection()������ӹ����Ŀͻ�����Ϣ��
    //ȡ��peerAddress��peerPort����ʾ��comboBox(cbxConnection)�ϣ�
    //�����ͻ��˵�readyRead()�ź����ӵ����������Զ���Ķ����ݲۺ���ReadData()�ϡ�
    //���ͻ��˵�disconnected()�ź����ӵ����������Զ���Ĳۺ���disconnectedSlot()�ϡ�
    currentClient = tcpServer->nextPendingConnection();//��һ�����������
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
    //����disconnected�źŲ�δ�ṩSocketDescriptor��������Ҫ����Ѱ��
    for (int i = 0; i < tcpClient.length(); i++)
    {
        if (tcpClient[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // ɾ���洢��combox�еĿͻ�����Ϣ
            ui.comboBox->removeItem(ui.comboBox->findText(tr("%1:%2")\
                .arg(tcpClient[i]->peerAddress().toString().split("::ffff:")[1])\
                .arg(tcpClient[i]->peerPort())));
            // ɾ���洢��tcpClient�б��еĿͻ�����Ϣ
            tcpClient[i]->destroyed();
            tcpClient.removeAt(i);
        }
    }
}

void Qt_TCPServer::readData()
{
    // ����readyRead�źŲ�δ�ṩSocketDecriptor��������Ҫ�������пͻ���
    for (int i = 0; i < tcpClient.length(); i++)
    {
        QByteArray buffer = tcpClient[i]->readAll();
        if (buffer.isEmpty())    continue;

        static QString IP_Port, IP_Port_Pre;
        IP_Port = tr("[%1:%2]:").arg(tcpClient[i]->peerAddress().toString().split("::ffff:")[1])\
            .arg(tcpClient[i]->peerPort());

        // ���˴���Ϣ�ĵ�ַ���ϴβ�ͬ��������ʾ�˴���Ϣ�Ŀͻ��˵�ַ
        if (IP_Port != IP_Port_Pre)
            ui.recvEdit->append(IP_Port);
        ui.recvEdit->append(buffer);
        //����ip_port
        IP_Port_Pre = IP_Port;
    }
}

void Qt_TCPServer::on_connectBtn_clicked()
{
    //a������TCP���ӵĺ�����void connectToHost(const QHostAddress & address, quint16 port, OpenMode openMode = ReadWrite)�Ǵ�QAbstractSocket�̳�������public function��ͬʱ������һ��virtual function������Ϊ��Attempts to make a connection to address on port port��
    //b���ȴ�TCP���ӳɹ��ĺ�����bool waitForConnected(int msecs = 30000)ͬ���Ǵ�QAbstractSocket�̳�������public function��ͬʱ������һ��virtual function������Ϊ��Waits until the socket is connected, up to msecs milliseconds.If the connection has been established, this function returns true; otherwise it returns false.In the case where it returns false, you can call error() to determine the cause of the error.
    //���������У�edtIP, edtPort��ui�ϵ�����lineEditor��������д������IP�Ͷ˿ںš�btnConnect�ǡ����� / �Ͽ������ð�ť��btnSend����������������ݵİ�ť��ֻ�����ӽ���֮�󣬲Ž���setEnabled��
    if (ui.connectBtn->text() == "����")
    {
        bool ok = tcpServer->listen(QHostAddress::Any, ui.portNumEdit->text().toInt());
        if (ok)
        {
            ui.connectBtn->setText("�Ͽ�");
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
                ui.connectBtn->setText("����");
                ui.sendBtn->setEnabled(false);
            }
            tcpClient.removeAt(i);//�ӱ���Ŀͻ����б���ȥ��   
        }
        tcpServer->close();//���ټ����˿�
        ui.connectBtn->setText("����");
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
    //ȫ������
    if (ui.comboBox->currentIndex() == 0)
    {
        for (int i = 0; i < tcpClient.length(); i++)
            tcpClient[i]->write(data.toLatin1());  //qt5��ȥ��.toAscii()
    }
    else
    {
        //ָ������
        QString clientIP = ui.comboBox->currentText().split(":")[0];
        int clientPort = ui.comboBox->currentText().split(":")[1].toInt();
        for (int i = 0; i < tcpClient.length(); i++)
        {
            if (tcpClient[i]->peerAddress().toString().split("::ffff:")[1] == clientIP && tcpClient[i]->peerPort() == clientPort)
            {
                tcpClient[i]->write(data.toLatin1());
                return; //ip:portΨһ�������������
            }
        }

    }
    
}


