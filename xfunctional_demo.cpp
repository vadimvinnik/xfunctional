#include "xfunctional.h"
#include <string>
#include <cassert>
#include <limits>
#include <functional>
#include <list>

using number_t = int;
constexpr auto number_nothing = std::numeric_limits<number_t>::min();

number_t decimal_to_number(std::string const& s)
{
    try
    {
        return std::stoi(s);
    }
    catch (std::invalid_argument)
    {
        return number_nothing;
    }
}

number_t english_numeral_to_number(std::string const& s)
{
    static std::string const numerals[] = {
        "zero",
        "one",
        "two",
        "three",
        "four",
        "five",
        "six",
        "seven",
        "eight",
        "nine",
        "ten",
        "eleven",
        "twelve",
        "thirteen",
        "fourteen",
        "fifteen",
        "sixteen",
        "seventeen",
        "eighteen",
        "nineteen",
        "twenty"
    };

    auto const i = std::find(
        std::cbegin(numerals),
        std::cend(numerals),
        s);

    return i == std::cend(numerals)
        ? number_nothing
        : i - std::cbegin(numerals);
}

number_t roman_to_number(std::string const& s)
{
    static std::string const numerals[] = {
        "I",
        "II",
        "III",
        "IV",
        "V",
        "VI",
        "VII",
        "VIII",
        "IX",
        "X",
        "XI",
        "XII",
        "XIII",
        "XIV",
        "XV",
        "XVI",
        "XVII",
        "XVIII",
        "XIX",
        "XX"
    };

    auto const i = std::find(
        std::cbegin(numerals),
        std::cend(numerals),
        s);

    return i == std::cend(numerals)
        ? number_nothing
        : 1 + (i - std::cbegin(numerals));
}

void test_make_string_to_number()
{
    using sum_t = xfunctional::fsum_t<number_t, std::string>;

    auto string_to_number = sum_t::make(
        number_nothing,
        decimal_to_number,
        english_numeral_to_number,
        roman_to_number);

    assert(string_to_number("2019")
        == 2019);
    assert(string_to_number("twelve")
        == 12);
    assert(string_to_number("XIV")
        == 14);
    assert(string_to_number("sieben")
        == number_nothing);
}

void test_exec_string_to_number()
{
    using func_t = number_t (*)(std::string const&);
    using sum_t = xfunctional::fsum_t<number_t, std::string>;

    func_t const string_to_number[] =
    {
        decimal_to_number,
        english_numeral_to_number,
        roman_to_number
    };

    assert(sum_t::exec(number_nothing, string_to_number, "2019")
        == 2019);
    assert(sum_t::exec(number_nothing, string_to_number, "twelve")
        == 12);
    assert(sum_t::exec(number_nothing, string_to_number, "XIV")
        == 14);
    assert(sum_t::exec(number_nothing, string_to_number, "sieben")
        == number_nothing);
}

void test_make_invocation_count()
{
    using sum_t = xfunctional::fsum_t<int>;

    int count = 0;
    auto chain = sum_t::make(
        0,
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 7; },
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 3; }
    );

    auto const value = chain();

    assert(value == 7);
    assert(count == 4);
}

void test_exec_invocation_count()
{
    using func_t = std::function<int()>;
    using sum_t = xfunctional::fsum_t<int>;

    int count = 0;

    auto const chain = std::list<func_t>
    {
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 7; },
        [&count]() { ++count; return 0; },
        [&count]() { ++count; return 3; }
    };

    auto const value = sum_t::exec(0, chain);

    assert(value == 7);
    assert(count == 4);
}

class single_match
{
    int const match_;
    std::string const value_if_match_;
    std::string const value_if_miss_;

public:
    single_match(
        int const match,
        std::string const& value_if_match,
        std::string const& value_if_miss) :
    match_(match),
    value_if_match_(value_if_match),
    value_if_miss_(value_if_miss)
    {}

    std::string operator() (int const x) const
    {
        return match_ == x
            ? value_if_match_
            : value_if_miss_;
    }
};

void test_make_first_match()
{
    using sum_t = xfunctional::fsum_t<std::string, int>;

    auto int_to_string = sum_t::make(
        {},
        single_match(0, "zero"  , ""),
        single_match(1, "one"   , ""),
        single_match(2, "two"   , ""),
        single_match(0, "zero2" , ""), // !
        single_match(3, "three" , "")
    );

    assert(int_to_string(0) == "zero");
    assert(int_to_string(2) == "two");
    assert(int_to_string(3) == "three");
    assert(int_to_string(4) == "");
}

void test_exec_first_match()
{
    using sum_t = xfunctional::fsum_t<std::string, int>;

    std::list<single_match> const int_to_string =
    {
        { 0, "zero"  , "" },
        { 1, "one"   , "" },
        { 2, "two"   , "" },
        { 0, "zero2" , "" }, // !
        { 3, "three" , "" }
    };

    assert(sum_t::exec({}, int_to_string, 0) == "zero");
    assert(sum_t::exec({}, int_to_string, 2) == "two");
    assert(sum_t::exec({}, int_to_string, 3) == "three");
    assert(sum_t::exec({}, int_to_string, 4) == "");
}

int main()
{
    test_make_string_to_number();
    test_exec_string_to_number();

    test_make_invocation_count();
    test_exec_invocation_count();

    test_make_first_match();
    test_exec_first_match();

    return 0;
}