#include <QCoreApplication>

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

class client
{

private:
    io_service m_io;
    buffer_type m_buf;
    ip::tcp::endpoint m_ep;
    int m_count;
public:
    client(): m_buf(100, 0),  m_ep(ip::address::from_string("127.0.0.1"), 6688), m_count(0)
    { start(); }

    void run()
    { m_io.run();}

    void start()
    {
        sock_ptr sock(new ip::tcp::socket(m_io));
        sock->async_connect(m_ep, boost::bind(&client::conn_handler, this, boost::asio::placeholders::error, sock));
    }

    void conn_handler(const boost::system::error_code&ec, sock_ptr sock)
    {
        if (ec)
        {return;}
        cout<<"Receive from "<<sock->remote_endpoint().address()<<": "<<endl;
        sock->async_read_some(buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
    }

    void read_handler(const boost::system::error_code&ec, sock_ptr sock)
    {
        if (ec)
        {return;}

        sock->async_write_some(buffer("hello asio server"), boost::bind(&client::write_handler, this, boost::asio::placeholders::error));

        sock->async_read_some(buffer(m_buf), boost::bind(&client::read_handler, this, boost::asio::placeholders::error, sock));
        cout<<&m_buf[0]<<endl;
    }

    void write_handler(const boost::system::error_code&ec)
    {
        cout<<"send to server msg complete"<<endl;
        m_count++;
        cout << m_count << endl;
    }
};

int main(int argc, char *argv[])
{
    try
        {
            cout<<"Client start."<<endl;
            client cli;
            cli.run();
        }
        catch (std::exception &e)
        {
            cout<<e.what()<<endl;
        }

    getchar();
    return 0;
}
