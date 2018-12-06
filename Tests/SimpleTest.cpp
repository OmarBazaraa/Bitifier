#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include "doctest/doctest.h"

using namespace std;

TEST_CASE ("Test 1") {
    CHECK(1 + 1 == 2);
}