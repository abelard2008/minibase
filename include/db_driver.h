// -*- C++ -*-
#ifndef _DB_DRIVER_H_
#define _DB_DRIVER_H_

#include "test_driver.h"

class DBDriver : public TestDriver
{
public:
    DBDriver();
    ~DBDriver();

    Status runTests();

private:
    PageId runStart;

    int test1();
    int test2();
    int test3();
    int test4();
    int test5();
    int test6();
    const char* testName();
};


#endif
