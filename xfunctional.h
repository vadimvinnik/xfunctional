#pragma once

#include <iterator>
#include <type_traits>
#include <optional>

namespace xfunctional
{
  template <typename T>
  T id(T x) { return x; }

  template <typename R, typename ...Args>
  struct constf
  {
    template <R Value>
    static R make(Args...) { return Value; }
  };

  template <typename R, typename ...Args>
  class constf_t
  {
  public:
    explicit constf_t(R const& value) : value_(value) {}

    R operator()(Args const&...) const { return value_; }

  private:
    R const value_;
  };

  template <typename R, typename ...Args>
  struct fsum
  {
    using maybe_t = std::optional<R>;

    static auto make()
    {
      return &constf<maybe_t, Args...>::make<std::nullopt>;
    }

    template <typename F, typename ...Fs>
    static auto make(F f, Fs ...fs)
    {
      return [f, fs...](Args ...args) -> maybe_t
      {
        maybe_t const r = f(args...);
        return r.has_value()
          ? r
          : make(fs...)(args...);
      };
    }

    template <typename I>
    static maybe_t exec(
      I from,
      I to,
      Args ...args)
    {
      while (from != to)
      {
        maybe_t const r = (*from)(args...);

        if (r.has_value())
          return r;

        ++from;
      }

      return std::nullopt;
    }

    template <typename C>
    static maybe_t exec(
      C& container,
      Args ...args)
    {
      return exec(
        std::begin(container),
        std::end(container),
        args...);
    }

    template <typename F, std::size_t N>
    static maybe_t exec(
      F (&array)[N],
      Args ...args)
    {
      return exec(
        std::begin(array),
        std::end(array),
        args...);
    }
  };
}
