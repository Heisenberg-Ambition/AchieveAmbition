
#include <array>
#include <chrono>
#include <cstddef>
#include <deque>
#include <iostream>
#include <memory>
#include <utility>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace boost::asio;

// * socket → bind → listen → accept → read/write

using boost::asio::ip::tcp;

static constexpr int SERVER_PORT = 9000;

// * resolver/acceptor/socket每个 I/O 对象都要绑定到某个事件循环, 才知道“完成时往哪里汇报”
// * boost::asio::io_context io; 在出现async_read时, 才需要io.run()开启异步.

class Session : public std::enable_shared_from_this<Session>
{
  public:
    Session(tcp::socket socket)
        : socket_(std::move(socket)),
          timer_(socket_.get_executor())
    {}

    void start()
    {
        do_read();
        start_periodic_send();
    }

  private:
    // ======== 读链 ========
    void do_read()
    {
        auto self = shared_from_this();

        socket_.async_read_some(
            boost::asio::buffer(read_buffer_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                {
                    std::cout << "Server received: "
                              << std::string(read_buffer_.data(), length)
                              << "\n";

                    do_read();
                }
                else
                {
                    std::cout << "Client disconnected\n";
                }
            });
    }

    // ======== 写链 ========
    void send(const std::string& msg)
    {
        bool write_in_progress = !write_queue_.empty();
        write_queue_.push_back(msg);

        if (!write_in_progress)
        {
            do_write();
        }
    }

    void do_write()
    {
        auto self = shared_from_this();

        boost::asio::async_write(
            socket_,
            boost::asio::buffer(write_queue_.front()),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec)
                {
                    write_queue_.pop_front();

                    if (!write_queue_.empty())
                    {
                        do_write();
                    }
                }
            });
    }

    // ======== 定时主动发送 ========
    void start_periodic_send()
    {
        auto self = shared_from_this();

        timer_.expires_after(std::chrono::milliseconds(500));
        timer_.async_wait(
            [this, self](boost::system::error_code ec) {
                if (!ec)
                {
                    auto current_time = std::chrono::system_clock::now();
                    std::time_t t = std::chrono::system_clock::to_time_t(current_time);
                    auto time_str = std::make_shared<std::string>(std::ctime(&t));
                    send("Server says hello\n" + *time_str);
                    start_periodic_send();
                }
            });
    }

    tcp::socket socket_;
    boost::asio::steady_timer timer_;
    std::array<char, 1024> read_buffer_;
    std::deque<std::string> write_queue_;
};

int main()
{
    boost::asio::io_context io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), SERVER_PORT));

    acceptor.async_accept(
        [&](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
            {
                std::cout << "Client connected!\n";
                std::make_shared<Session>(std::move(socket))->start();
            }
        });

    std::cout << "Server started\n";
    io.run();
}

// int main()
// {
//     boost::asio::io_context io;

//     tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 9000));

//     tcp::socket socket(io);

//     // * 异步链接客户端
//     acceptor.async_accept(socket,
//         [&](boost::system::error_code ec) {
//             if (!ec)
//             {
//                 std::cout << "Client connected!\n";
//                 auto buffer = std::make_shared<std::array<char, 1024>>();

//                 socket.async_read_some(
//                     boost::asio::buffer(*buffer),
//                     [&, buffer](boost::system::error_code ec, std::size_t length) {
//                         if (!ec)
//                         {
//                             std::cout << "Received: "
//                                       << std::string(buffer->data(), length)
//                                       << "\n";

//                             std::string reply = "Hello from async server!";

//                             boost::asio::async_write(
//                                 socket,
//                                 boost::asio::buffer(reply),
//                                 [](boost::system::error_code, std::size_t) {
//                                     std::cout << "Reply sent.\n";
//                                 });
//                         }
//                     });
//             }
//             else
//             {
//                 std::cerr << "Accept error: " << ec.message() << "\n";
//             }
//         });

//     std::cout << "Server started.\n";
//     io.run();
//     std::cout << "Server finished.\n";
// }
