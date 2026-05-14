
#include <iostream>
#include <thread>
#include <utility>

#include <bits/types/sigset_t.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/socket_base.hpp>

using boost::asio::ip::tcp;
using namespace boost::asio;

static constexpr int SERVER_PORT = 9000;

// * socket → bind → listen → accept → read/write
// * resolver/acceptor/socket每个 I/O 对象都要绑定到某个事件循环, 才知道“完成时往哪里汇报”
// *boost::asio::io_context io; 在出现async_read时, 才需要io.run()开启异步.

bool read_socket(tcp::socket* socket)
{
    size_t length = 0;
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

        // * 创建 acceptor(监听器), 实际上是代替了原生的控制listen()/accept()的socket
        // * 原生的socket 在listen() 之后，这个 socket 就变成“监听socket”, 在boost里语义显式化了
        // * 内核层面：监听 socket 有一个 backlog 队列, 每个连接是一个独立的内核 socket 结构, accept() 只是把队列里的连接“取出来”
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), SERVER_PORT));
        std::cout << "Server listening on port 9000...\n";

        // 等待客户端连接(阻塞)
        while (true)
        {
            tcp::socket* socket = new tcp::socket(io);
            // * 相当于 accept()
            acceptor.accept(*socket);
            std::cout << "Client connected!\n";

            std::string message = "Hello from server!";
            socket->write_some(boost::asio::buffer(message));

            auto t1 = std::jthread(process_socket, socket);
            t1.detach();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
};

// awaitable<void> listener()
// {
//     auto executor = co_await std::this_coro::executor;
//     tcp::acceptor acceptor(executor, {tcp::v4(), 8888});

//     for (;;)
//     {
//         tcp::socket socket = co_await acceptor.async_accept(use_awaitable);

//         // 启动 session 协程
//         co_spawn(executor,
//             session(std::move(socket)),
//             detached);
//     }
// }

// struct Task
// {
//     struct promise_type
//     {
//         Task get_return_object()
//         {
//             return {};
//         }
//         std::suspend_never initial_suspend()
//         {
//             return {};
//         }
//         std::suspend_never final_suspend() noexcept
//         {
//             return {};
//         }
//         void return_void() {}
//         void unhandled_exception() {}
//     };
// };

// Task hello()
// {
//     std::cout << "1\n";
//     co_await std::suspend_always {};
//     std::cout << "2\n";
// }

// struct SimpleTask
// {
//     struct promise_type
//     {
//         SimpleTask get_return_object()
//         {
//             return SimpleTask {
//                 std::coroutine_handle<promise_type>::from_promise(*this)};
//         }

//         std::suspend_always initial_suspend()
//         {
//             return {};
//         }

//         std::suspend_always final_suspend() noexcept
//         {
//             return {};
//         }

//         void return_void() {}

//         void unhandled_exception()
//         {
//             std::terminate();
//         }
//     };

//     std::coroutine_handle<promise_type> handle;

//     explicit SimpleTask(std::coroutine_handle<promise_type> h)
//         : handle(h) {}

//     ~SimpleTask()
//     {
//         if (handle)
//             handle.destroy();
//     }

//     void resume()
//     {
//         if (!handle.done())
//             handle.resume();
//     }

//     bool done() const
//     {
//         return handle.done();
//     }
// };

// SimpleTask my_coroutine()
// {
//     std::cout << "Step 1\n";
//     co_await std::suspend_always {};

//     std::cout << "Step 2\n";
//     co_await std::suspend_always {};

//     std::cout << "Step 3\n";
// };

// int main()
// {
//     auto task = my_coroutine();

//     std::cout << "Resume 1\n";
//     task.resume();

//     std::cout << "Resume 2\n";
//     task.resume();

//     std::cout << "Resume 3\n";
//     task.resume();

//     std::cout << "Resume 4 (nothing happens)\n";
//     task.resume();
// }
