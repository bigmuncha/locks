#include "doctest/doctest/doctest.h"
#include "lctree.hpp"
#include <thread>
#include <random>


uint8_t ip[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
TEST_CASE("testaddLookup")
{
    lcTree trie(100);
    uint8_t ip[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    trie.add(ip, 64, 100);

    auto val = trie.lookup(ip);
    REQUIRE(val == 100);

    ip[13] = 10;
    val = trie.lookup(ip);
    REQUIRE(val == 100);
    ip[5] = 10;
    val = trie.lookup(ip);
    REQUIRE(val == 0);
}

TEST_CASE("testaddMoreCommon")
{
    lcTree trie(100);
    std::array<uint8_t, 16> ipArr = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, 16};
    decltype(ipArr) most = ipArr;
    most[10] = 100;
    trie.add(ipArr, 64, 100);
    REQUIRE(trie.lookup(ipArr) == 100);
    REQUIRE(trie.lookup(most) == 100);
    trie.add(most, 85, 200);
    REQUIRE(trie.lookup(ipArr) == 100);
    REQUIRE(trie.lookup(most) == 200);
    most[10] = 20;
    REQUIRE(trie.lookup(most) == 100);
}
