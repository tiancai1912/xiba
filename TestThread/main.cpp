#include <QCoreApplication>

#include <QDebug>
#include <thread>

#include "test.h"

//fun1
//static void test() {
//    qDebug() << "test thread";
//}


//func2
//class ThreadImpl
//{
//public:
//    ThreadImpl() {}

//    void start() {
////        std::thread t(test);
//        std::thread t(&ThreadImpl::test, this);
//        t.detach();
//    }

//    void stop() {

//    }

//    virtual void test() {
//        qDebug() << "hello threadimpl";
//    }

//private:

//};

//class MyThread : public ThreadImpl
//{
//public:
//    MyThread() {}

//    void test() {
//        qDebug() << "hello my thread";
//    }
//};


int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

//    return a.exec();

      //fun1
//    std::thread t(test);
//    t.join();

      //fun2
//    MyThread thread;
//    thread.start();

//    qDebug() << "hello world";

    //fun3
    Test test;
    test.start();

    qDebug() << "wait thread\n";


    test.wait();

    qDebug() << "thread stop\n";
    return 0;
}
