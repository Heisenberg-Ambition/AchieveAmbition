
#include <iostream>
#include <thread>
#include <utility>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

static constexpr int SERVER_PORT = 9000;

bool read_socket(tcp::socket* socket)
{
    size_t length;
    try
    {
        while (true)
        {
            // 读取数据（阻塞）
            char data[1024];
            length = socket->read_some(boost::asio::buffer(data));
            std::cout << "Received: " << std::string(data, length) << "\n";
        }
        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
    return true;
};

bool write_socket(tcp::socket* socket)
{
    try
    {
        while (true)
        {
            // 发送数据（阻塞）
            // std::string reply = "Hello from server!";
            std::string data {};
            std::cin >> data;
            socket->write_some(boost::asio::buffer(data));
        }
        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return false;
    }
    return true;
};

void process_socket(tcp::socket* socket)
{
    auto t1 = std::jthread(read_socket, socket);
    auto t2 = std::jthread(write_socket, socket);
    t1.join();
    t2.join();
    delete socket;
    socket = nullptr;
};

int main()
{
    try
    {
        boost::asio::io_context io;

        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(SERVER_PORT));

        tcp::socket* socket = new tcp::socket(io);

        // 连接服务器（阻塞）
        boost::asio::connect(*socket, endpoints);

        std::cout << "Connected to server.\n";

        // 发送数据
        std::string message = "Hello from client!";
        socket->write_some(boost::asio::buffer(message));

        auto t1 = std::jthread(process_socket, socket);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
};
