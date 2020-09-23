#include <QCoreApplication>

#include <thread>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::asio;
using namespace std;

typedef boost::shared_ptr<ip::tcp::socket> sock_ptr;
typedef vector<char> buffer_type;

class server
{
private:
    io_service m_io;
    ip::tcp::acceptor m_acceptor;
    buffer_type m_buf;

public:
    server() : m_buf(100, 0), m_acceptor(m_io, ip::tcp::endpoint(ip::tcp::v4(), 6688))
    { accept(); }

    void run(){ m_io.run();}

    void accept()
    {
//        sock_ptr sock(new ip::tcp::socket(m_io));
        int  i = 10;
        while(i > 0) {
            sock_ptr sock(new ip::tcp::socket(m_io));
            m_acceptor.async_accept(*sock, boost::bind(&server::accept_handler, this, boost::asio::placeholders::error, sock));
            i--;
        }
    }

    void accept_handler(const boost::system::error_code& ec, sock_ptr sock)
    {
//        std::thread *thread = new std::thread(std::bind(&server::handleThread, this, std::placeholders::_1, sock));
        std::thread *thread = new std::thread(std::bind(&server::handleThread, this, sock));
//        thread->join();
//        if (ec)
//        {
//            return;
//        }
//        cout<<"Client:";
//        cout<<sock->remote_endpoint().address()<<endl;
//        sock->async_write_some(buffer("hello asio client"), boost::bind(&server::write_handler, this, boost::asio::placeholders::error));
//        // 发送完毕后继续监听，否则io_service将认为没有事件处理而结束运行

//        sock->async_read_some(buffer(m_buf), boost::bind(&server::read_handler, this, boost::asio::placeholders::error, sock));

//        accept();
    }

    void handleThread(sock_ptr sock)
    {
        int i = 10;
        while(i > 0) {
//            cout<<"Client:";
//            cout<<sock->remote_endpoint().address()<<endl;
            cout << sock->remote_endpoint().address() << sock->remote_endpoint().port() << endl;
            sock->async_write_some(buffer("hello asio client"), boost::bind(&server::write_handler, this, boost::asio::placeholders::error));
            // 发送完毕后继续监听，否则io_service将认为没有事件处理而结束运行

            sock->async_read_some(buffer(m_buf), boost::bind(&server::read_handler, this, boost::asio::placeholders::error, sock));
            sleep(1);
            i--;
            cout << i << endl;
        }
    }

    void read_handler(const boost::system::error_code&ec, sock_ptr sock)
    {
        if (ec)
        {return;}
        sock->async_read_some(buffer(m_buf), boost::bind(&server::read_handler, this, boost::asio::placeholders::error, sock));
        cout<<&m_buf[0]<<endl;
    }

    void write_handler(const boost::system::error_code&ec)
    {
        cout<<"send msg complete"<<endl;
    }
};

int main(int argc, char *argv[])
{
    try
        {
            cout<<"Server start."<<endl;
            server srv;
            srv.run();
        }
        catch (std::exception &e)
        {
            cout<<e.what()<<endl;
        }

    getchar();
    return 0;
}
