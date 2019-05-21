#pragma once

#include <iterator>
#include <type_traits>

namespace xfunctional
{
    template <typename T>
    T id(T x) { return x; }

    template <typename R, typename ...Args>
    struct constf
    {
        template <R Value>
        static R make(Args const&...) { return Value; }
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
    struct fsum_t
    {
        static auto make(R const& bottom)
        {
            return [&bottom](Args ...args)->R { return bottom; };
        }

        template <typename F, typename ...Fs>
        static auto make(R const& bottom, F f, Fs ...fs)
        {
            return [bottom, f, fs...](Args ...args)->R
            {
                R const r = f(args...);
                return (bottom != r)
                    ? r
                    : make(bottom, fs...)(args...);
            };
        }

        template <typename I>
        static R exec(
            R const& bottom,
            I from,
            I to,
            Args ...args)
        {
            while (from != to)
            {
                R const r = (*from)(args...);

                if (bottom != r)
                    return r;

                ++from;
            }

            return bottom;
        }

        template <typename C>
        static R exec(
            R const& bottom,
            C& container,
            Args ...args)
        {
            return exec(
                bottom,
                std::begin(container),
                std::end(container),
                args...);
        }

        template <typename F, std::size_t N>
        static R exec(
            R const& bottom,
            F (&array)[N],
            Args ...args)
        {
            return exec(
                bottom,
                std::begin(array),
                std::end(array),
                args...);
        }
    };
}
