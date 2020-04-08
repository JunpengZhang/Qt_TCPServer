#include "Qt_TCPServer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Qt_TCPServer w;
	w.show();
	return a.exec();
}
