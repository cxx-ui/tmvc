// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file range_test.cpp
/// Contains unit tests for range utility functions.

#include "../range.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


BOOST_AUTO_TEST_SUITE(range_test)


BOOST_AUTO_TEST_CASE(adjust_pos_after_insert_before_pos_same_line) {
    position pos{2, 10};
    range ins{{2, 4}, {2, 7}};
    auto res = adjust_pos_after_insert(pos, ins);
    BOOST_CHECK_EQUAL(res.line, 2);
    BOOST_CHECK_EQUAL(res.column, 13);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_insert_multiline) {
    position pos{2, 10};
    range ins{{2, 4}, {4, 3}};
    auto res = adjust_pos_after_insert(pos, ins);
    BOOST_CHECK_EQUAL(res.line, 4);
    BOOST_CHECK_EQUAL(res.column, 9);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_insert_equal_start_flag_off) {
    position pos{1, 2};
    range ins{{1, 2}, {1, 5}};
    auto res = adjust_pos_after_insert(pos, ins, false);
    BOOST_CHECK_EQUAL(res.line, 1);
    BOOST_CHECK_EQUAL(res.column, 2);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_insert_equal_start_flag_on) {
    position pos{1, 2};
    range ins{{1, 2}, {1, 5}};
    auto res = adjust_pos_after_insert(pos, ins, true);
    BOOST_CHECK_EQUAL(res.line, 1);
    BOOST_CHECK_EQUAL(res.column, 5);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_erase_before_range) {
    position pos{1, 2};
    range er{{1, 3}, {1, 6}};
    auto res = adjust_pos_after_erase(pos, er);
    BOOST_CHECK_EQUAL(res.line, 1);
    BOOST_CHECK_EQUAL(res.column, 2);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_erase_inside_range) {
    position pos{1, 5};
    range er{{1, 3}, {1, 6}};
    auto res = adjust_pos_after_erase(pos, er);
    BOOST_CHECK_EQUAL(res.line, 1);
    BOOST_CHECK_EQUAL(res.column, 3);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_erase_after_range_same_line) {
    position pos{1, 8};
    range er{{1, 3}, {1, 6}};
    auto res = adjust_pos_after_erase(pos, er);
    BOOST_CHECK_EQUAL(res.line, 1);
    BOOST_CHECK_EQUAL(res.column, 5);
}


BOOST_AUTO_TEST_CASE(adjust_pos_after_erase_after_range_multiline) {
    position pos{5, 7};
    range er{{2, 4}, {4, 1}};
    auto res = adjust_pos_after_erase(pos, er);
    BOOST_CHECK_EQUAL(res.line, 3);
    BOOST_CHECK_EQUAL(res.column, 7);
}

BOOST_AUTO_TEST_CASE(adjust_range_after_insert_before_range) {
    range rng{{0, 5}, {0, 15}};
    range ins{{0, 3}, {0, 5}};
    auto res = adjust_range_after_insert(rng, ins);
    BOOST_CHECK_EQUAL(res.start.line, 0);
    BOOST_CHECK_EQUAL(res.start.column, 7);
    BOOST_CHECK_EQUAL(res.end.line, 0);
    BOOST_CHECK_EQUAL(res.end.column, 17);
}

BOOST_AUTO_TEST_CASE(adjust_range_after_insert_inside_range) {
    range rng{{0, 5}, {0, 15}};
    range ins{{0, 7}, {0, 10}};
    auto res = adjust_range_after_insert(rng, ins);
    BOOST_CHECK_EQUAL(res.start.line, 0);
    BOOST_CHECK_EQUAL(res.start.column, 5);
    BOOST_CHECK_EQUAL(res.end.line, 0);
    BOOST_CHECK_EQUAL(res.end.column, 18);
}

BOOST_AUTO_TEST_CASE(adjust_range_after_erase_partial_overlap) {
    range rng{{0, 5}, {0, 15}};
    range del{{0, 8}, {0, 10}};
    auto res = adjust_range_after_erase(rng, del);
    BOOST_CHECK_EQUAL(res.start.line, 0);
    BOOST_CHECK_EQUAL(res.start.column, 5);
    BOOST_CHECK_EQUAL(res.end.line, 0);
    BOOST_CHECK_EQUAL(res.end.column, 13);
}

BOOST_AUTO_TEST_CASE(adjust_range_after_erase_entire_range) {
    range rng{{0, 5}, {0, 15}};
    range del{{0, 4}, {0, 20}};
    auto res = adjust_range_after_erase(rng, del);
    BOOST_CHECK(res.empty());
}


BOOST_AUTO_TEST_SUITE_END()


}
