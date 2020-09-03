#include <QCoreApplication>
#include "test.h"

/**
 * c++ object only copy not reference
 */

int main(int argc, char *argv[])
{

    test a;
    test b = a;
    a.setNumber(123);
    a.print();
    b.print();

    b.setNumber(456);
    b.print();
    a.print();

    getchar();
    return 0;
}
