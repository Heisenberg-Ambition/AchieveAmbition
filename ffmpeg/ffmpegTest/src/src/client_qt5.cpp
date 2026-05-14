
#include <QCoreApplication>
#include <QObject>
#include <QTcpSocket>
#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTcpSocket socket;

    QObject::connect(&socket, &QTcpSocket::connected, [&]() {
        std::cout << "Connected!\n";
        socket.write("Hello from client!\n");
    });

    QObject::connect(&socket, &QTcpSocket::readyRead, [&socket]() {
        auto current_time = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(current_time);
        auto time_str = std::make_shared<std::string>(std::ctime(&t));
        std::cout << std::string(*time_str) + socket.readAll().toStdString();
    });

    socket.connectToHost("127.0.0.1", 12345);

    return app.exec(); // 事件循环
}
