#include <array>
#include <iostream>
#include <memory>
#include <utility>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

static constexpr int SERVER_PORT = 9000;

class Client : public std::enable_shared_from_this<Client>
{
  public:
    Client(boost::asio::io_context& io)
        : socket_(io)
    {}

    void start(const std::string& host, unsigned short port)
    {
        auto self = shared_from_this();

        tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, std::to_string(port));

        boost::asio::async_connect(socket_, endpoints,
            [this, self](boost::system::error_code ec, tcp::endpoint) {
                if (!ec)
                {
                    std::cout << "Connected to server!\n";
                    do_read();
                }
            });
    }

    void send(const std::string& msg)
    {
        auto self = shared_from_this();

        boost::asio::async_write(socket_,
            boost::asio::buffer(msg),
            [this, self, &msg](boost::system::error_code ec, std::size_t) {
                if (!ec)
                {
                    std::cout << "Sent: " << msg;
                }
            });
    }

  private:
    void do_read()
    {
        auto self = shared_from_this();

        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                {
                    std::cout << "Received: "
                              << std::string(buffer_.data(), length);

                    do_read(); // 继续监听
                }
                else
                {
                    std::cout << "Server closed connection.\n";
                }
            });
    }

  private:
    tcp::socket socket_;
    std::array<char, 1024> buffer_;
};

int main()
{
    boost::asio::io_context io;

    auto client = std::make_shared<Client>(io);

    client->start("127.0.0.1", SERVER_PORT);

    std::thread t([&io]() {
        io.run();
    });

    // 模拟主动发送
    std::string line;
    while (std::getline(std::cin, line))
    {
        client->send(line + "\n");
    }

    t.join();
}

// int main()
// {
//     boost::asio::io_context io;

//     auto resolver = std::make_shared<tcp::resolver>(io);
//     auto socket = std::make_shared<tcp::socket>(io);

//     resolver->async_resolve("127.0.0.1", "9000",
//         [resolver, socket](boost::system::error_code ec,
//             tcp::resolver::results_type results) {
//             if (!ec)
//             {
//                 boost::asio::async_connect(*socket, results,
//                     [socket](boost::system::error_code ec,
//                         const tcp::endpoint&) {
//                         if (!ec)
//                         {
//                             std::cout << "Connected to server.\n";

//                             auto message =
//                                 std::make_shared<std::string>("Hello from async client!");

//                             boost::asio::async_write(*socket,
//                                 boost::asio::buffer(*message),
//                                 [socket, message](boost::system::error_code ec,
//                                     std::size_t) {
//                                     if (!ec)
//                                     {
//                                         auto buffer =
//                                             std::make_shared<std::array<char, 1024>>();

//                                         socket->async_read_some(
//                                             boost::asio::buffer(*buffer),
//                                             [socket, buffer](boost::system::error_code ec,
//                                                 std::size_t length) {
//                                                 if (!ec)
//                                                 {
//                                                     std::cout << "Server says: "
//                                                               << std::string(buffer->data(), length)
//                                                               << "\n";
//                                                 }
//                                             });
//                                     }
//                                 });
//                         }
//                     });
//             }
//         });
//     std::cout << "Client started.\n";
//     io.run();
//     std::cout << "Client finished.\n";
// }
