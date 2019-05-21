#include "xfunctional.h"

#include <string>
#include <cassert>
#include <limits>
#include <functional>
#include <list>

using number_t = int;
using maybe_number_t = std::optional<number_t>;
using maybe_string_t = std::optional<std::string>;

maybe_number_t decimal_to_number(std::string const& s)
{
  try
  {
      return std::stoi(s);
  }
  catch (std::invalid_argument)
  {
      return std::nullopt;
  }
}

maybe_number_t english_numeral_to_number(std::string const& s)
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
    ? std::nullopt
    : maybe_number_t{ i - std::cbegin(numerals) };
}

maybe_number_t roman_to_number(std::string const& s)
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
    ? std::nullopt
    : maybe_number_t{ 1 + (i - std::cbegin(numerals)) };
}

void test_make_string_to_number()
{
  using sum_t = xfunctional::fsum<number_t, std::string>;

  auto string_to_number = sum_t::make(
    decimal_to_number,
    english_numeral_to_number,
    roman_to_number);

  assert(string_to_number("2019")   == 2019);
  assert(string_to_number("twelve") == 12);
  assert(string_to_number("XIV")    == 14);
  assert(string_to_number("sieben") == std::nullopt);
}

void test_exec_string_to_number()
{
  using func_t = maybe_number_t(*)(std::string const&);
  using sum_t = xfunctional::fsum<number_t, std::string>;

  func_t const string_to_number[] =
  {
    decimal_to_number,
    english_numeral_to_number,
    roman_to_number
  };

  assert(sum_t::exec(string_to_number, "2019")   == 2019);
  assert(sum_t::exec(string_to_number, "twelve") == 12);
  assert(sum_t::exec(string_to_number, "XIV")    == 14);
  assert(sum_t::exec(string_to_number, "sieben") == std::nullopt);
}

void test_make_invocation_count()
{
  using sum_t = xfunctional::fsum<int>;

  int count = 0;
  auto chain = sum_t::make(
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return 7;            },
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return 3;            }
  );

  auto const value = chain();

  assert(value == 7);
  assert(count == 4);
}

void test_exec_invocation_count()
{
  using func_t = std::function<maybe_number_t()>;
  using sum_t = xfunctional::fsum<int>;

  int count = 0;

  auto const chain = std::list<func_t>
  {
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return 7;            },
    [&count]() { ++count; return std::nullopt; },
    [&count]() { ++count; return 3;            }
  };

  auto const value = sum_t::exec(chain);

  assert(value == 7);
  assert(count == 4);
}

class single_match
{
  int const match_;
  std::string const value_if_match_;

public:
  single_match(
    int const match,
    std::string const& value_if_match) :
  match_(match),
  value_if_match_(value_if_match)
  {}

  maybe_string_t operator() (int const x) const
  {
    return match_ == x
      ? maybe_string_t {value_if_match_}
      : std::nullopt;
  }
};

void test_make_first_match()
{
  using sum_t = xfunctional::fsum<std::string, int>;

  auto int_to_string = sum_t::make(
    single_match(0, "zero" ),
    single_match(1, "one"  ),
    single_match(2, "two"  ),
    single_match(0, "zero2"), // !
    single_match(3, "three")
  );

  assert(int_to_string(0) == "zero");
  assert(int_to_string(2) == "two");
  assert(int_to_string(3) == "three");
  assert(int_to_string(4) == std::nullopt);
}

void test_exec_first_match()
{
  using sum_t = xfunctional::fsum<std::string, int>;

  std::list<single_match> const int_to_string =
  {
    { 0, "zero" },
    { 1, "one"  },
    { 2, "two"  },
    { 0, "zero2"}, // !
    { 3, "three"}
  };

  assert(sum_t::exec(int_to_string, 0) == "zero");
  assert(sum_t::exec(int_to_string, 2) == "two");
  assert(sum_t::exec(int_to_string, 3) == "three");
  assert(sum_t::exec(int_to_string, 4) == std::nullopt);
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
