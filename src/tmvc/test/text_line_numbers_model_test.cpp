// Copyright (c) 2025, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

/// \file text_line_numbers_model_test.cpp
/// Contains unit tests for the text_line_numbers_model class.

#include "../simple_text_model.hpp"
#include "../text_line_numbers_model.hpp"
#include <boost/test/unit_test.hpp>


namespace tmvc::test {


struct text_line_numbers_model_test_fixture {
    using text_model_t = wsimple_text_model;
    using line_numbers_model_t = text_line_numbers_model<text_model_t>;

    text_model_t text;
    line_numbers_model_t line_numbers{text};

    static_assert(line_numbers_model<line_numbers_model_t>);
};


BOOST_FIXTURE_TEST_SUITE(text_line_numbers_model_test, text_line_numbers_model_test_fixture)


/// Tests returning max number of characters in line number
BOOST_AUTO_TEST_CASE(test_max_size) {
    text.reset(L"X \nX \nX \nX \nX \nX \nX \nX \nX");
    BOOST_CHECK_EQUAL(line_numbers.max_size(), 1);

    text.reset(L"X \nX \nX \nX \nX \nX \nX \nX \nX \nX \nX \n");
    BOOST_CHECK_EQUAL(line_numbers.max_size(), 2);
}


/// Tests returning line_number
BOOST_AUTO_TEST_CASE(test_line_number) {
    text.reset(L"X \nX \nX \nX \nX \nX \nX \nX \nX \nX \nX \n");
    BOOST_CHECK(line_numbers.line_number(1) == L"2");
    BOOST_CHECK(line_numbers.line_number(10) == L"11");
}


BOOST_AUTO_TEST_SUITE_END()


}
