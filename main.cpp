/**
 * 2020-8-18
 * zhangyu
 * 模拟采集线程的缓冲队列
 * 都是采用了mutex + condition_variable 的形式达到线程安全
 * mutex防止竞争修改字节内容
 * condition_variable减少cpu消耗，达到消息通知作用
 * fun1 将队列中取出来的内容进行拷贝，
 * 达到不阻塞生产者线程，同时不篡改消费者线程正在使用的数据
 *
 * fun2 重写了队列中元素类型，
 * 添加bool标志位判断此元素是否正在消费者线程中使用，
 * 使得生产者线程避开此元素，从而达到不篡改消费者线程数据的目的
 **/


#include <QCoreApplication>

#include <queue>
#include <QDebug>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>


struct Item {
    bool bUse;
    char *value;

    Item() {
        bUse = false;
        value = new char[10] {0};
    }
};

std::queue<char *> g_qu;
//std::queue<Item *> g_qu;
char *g_test1 = new char(10);
char *g_test2 = new char(10);

std::thread g_thread1;
std::thread g_thread2;
std::thread g_thread3;

std::mutex g_mutex;
std::condition_variable g_con;


Item *g_item1 = new Item();
Item *g_item2 = new Item();

//fun1
static void initQueue() {

    strcpy(g_test1, "test1");
    strcpy(g_test2, "test2");

    g_qu.push(g_test1);
    g_qu.push(g_test2);
}


//fun1
static char * popItem() {
    if (!g_qu.empty()) {
        char *tmp = g_qu.front();
        g_qu.pop();
        return tmp;
    }

    return NULL;
}

static void pushItem(char *tmp) {
    g_qu.push(tmp);
}


//fun1
static void publishData() {
    int num = 100;
    char tmp[10] = {0};
    while (num > 0) {
        std::unique_lock<std::mutex> lock(g_mutex);
        char *tmp_test = popItem();
//        qDebug() << tmp_test << endl;

        sprintf(tmp, "test%d", num);

        strcpy(tmp_test, tmp);
        qDebug() << tmp_test << endl;

        pushItem(tmp_test);

        lock.unlock();
        g_con.notify_one();

//        if (tmp_test == g_test1 || tmp_test == g_test2) {
//            qDebug() << "point equal" << endl;
//        }

        sleep(1);
        num--;
    }
}


//fun1
static void handleData1() {
    while(1) {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_con.wait(lock);

        char *test = g_qu.front();
        if (test != NULL) {
            char str[10] = {0};
            strcpy(str, test);
            lock.unlock();
            qDebug() << "handleData1: " <<  str << endl;
        }
    }
}


static void handleData2() {
    while(1) {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_con.wait(lock);

        char * test = popItem();
        if (test != NULL)
            qDebug() << "handleData2: " <<  test << endl;

        pushItem(test);
        lock.unlock();
    }
}


////////////fun2 //////////////////////
////fun2
//static void initQueue() {

//    strcpy(g_item1->value, "test1");
//    strcpy(g_item2->value, "test2");

//    g_qu.push(g_item1);
//    g_qu.push(g_item1);
//}

////fun2
//static Item * popItem() {
//    if (!g_qu.empty()) {
//        Item *tmp = g_qu.front();
//        g_qu.pop();
//        return tmp;
//    }

//    return NULL;
//}

////fun2
//static void pushItem(Item *item) {
//    g_qu.push(item);
//}

////fun2
//static void publishData() {
//    int num = 10000;
//    char tmp[10] = {0};
//    while (num > 0) {
//        std::unique_lock<std::mutex> lock(g_mutex);
//        Item *tmp_test = popItem();

//        if (tmp_test->bUse == false) {
//            sprintf(tmp, "test%d", num);
//            strcpy(tmp_test->value, tmp);
//            qDebug() << "test false" <<"num: " << num << " value: " << tmp_test->value << endl;
//        } else {
//            qDebug() << "test true" <<"num: " << num << " value: " << tmp_test->value << endl;
//        }


//        pushItem(tmp_test);

//        lock.unlock();
//        g_con.notify_one();

//        sleep(1);

////        if (tmp_test == g_test1 || tmp_test == g_test2) {
////            qDebug() << "point equal" << endl;
////        }

//        num--;
//    }
//}

////fun2
//static void handleData1() {
//    while(1) {
//        std::unique_lock<std::mutex> lock(g_mutex);
//        g_con.wait(lock);

//        Item *test = g_qu.front();
//        test->bUse = true;
//        lock.unlock();

//        sleep(2);

//        if (test != NULL) {
//            qDebug() << "handleData1: " << test->value << endl;
//            test->bUse = false;
//        }
//    }
//}


int main(int argc, char *argv[])
{
    initQueue();

    g_thread1 = std::thread(publishData);
    g_thread2 = std::thread(handleData1);
//    g_thread3 = std::thread(handleData2);

    g_thread1.join();
    g_thread2.join();
//    g_thread3.join();

    getchar();
    return 0;
}
