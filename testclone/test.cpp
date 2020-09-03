#include "test.h"

#include <stdio.h>
#include <memory>

test::test()
{

}

void test::setNumber(int number)
{
    mNumber = number;
}

void test::print()
{
    printf("the number is: %d\n", mNumber);
}
