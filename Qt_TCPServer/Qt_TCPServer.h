#pragma once

#include <QtWidgets/QWidget>
#include "ui_Qt_TCPServer.h"
#include <qtcpsocket.h>
#include <qtcpserver.h>
#include <qnetworkinterface.h>
#include <qmessagebox.h>

class Qt_TCPServer : public QWidget
{
	Q_OBJECT

public:
	Qt_TCPServer(QWidget *parent = Q_NULLPTR);
	~Qt_TCPServer();

private slots:
	void NewConnectionSlot();
	void disconnectedSlot();
	void readData();

	void on_connectBtn_clicked();
	void on_sendBtn_clicked();
	void on_clearBtn_clicked();

private:
	Ui::Qt_TCPServerClass ui;
	QTcpServer* tcpServer;
	QList<QTcpSocket*> tcpClient;//连接到服务器的客户端
	QTcpSocket* currentClient;// 当前待连接的客户端
};
