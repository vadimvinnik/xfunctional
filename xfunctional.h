#pragma once

#include <iterator>
#include <tuple>
#include <optional>

namespace xfunctional
{
  template <typename T>
  T id(T x) { return x; }

  template <typename R, typename ...Args>
  struct default_constf
  {
    using value_t = R;

    static R make(Args...) { return {}; }
  };

  template <typename R, typename ...Args>
  struct constf
  {
    using value_t = R;

    template <R Value>
    static R make(Args...) { return Value; }
  };

  template <typename R, typename ...Args>
  class constf_t
  {
  public:
    using value_t = R;

    explicit constf_t(R const& value) : value_(value) {}

    R operator()(Args const&...) const { return value_; }

  private:
    R const value_;
  };

  template <typename R, typename ...Args>
  class single_point_t
  {
    R const value_;
    std::tuple<Args...> const args_;

  public:
    using value_t = R;
    using maybe_t = std::optional<R>;

    single_point_t(
      R const& value,
      Args const&... args) :
    value_(value),
    args_(args...)
    {}

    maybe_t operator() (Args ...args) const
    {
      return args_ == std::make_tuple(args...)
        ? maybe_t {value_}
        : std::nullopt;
    }
  };

  template <typename R, typename ...Args>
  struct fsum
  {
    using maybe_t = std::optional<R>;
    using funcptr_t = maybe_t(*)(Args...);

    static funcptr_t make()
    {
      return &(default_constf<maybe_t, Args...>::make);
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
