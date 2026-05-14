
#include <QCoreApplication>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTcpServer server {};

    QObject::connect(&server, &QTcpServer::newConnection, [&]() {
        while (server.hasPendingConnections())
        {
            QTcpSocket* socket = server.nextPendingConnection();

            std::cout << "Client connected\n";

            QObject::connect(socket, &QTcpSocket::readyRead, [socket]() {
                auto data = socket->readAll();
                std::cout << "Received: " << data.toStdString() << "\n";

                socket->write("Hello from server!\n");
                qDebug() << "bytesToWrite:" << socket->bytesToWrite();
            });

            QObject::connect(socket, &QTcpSocket::disconnected, [socket]() {
                std::cout << "Client disconnected\n";
                socket->deleteLater();
            });
        }
    });

    if (!server.listen(QHostAddress::AnyIPv4, 12345))
    {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server started.\n";

    // QT主事件循环
    return app.exec();
}
