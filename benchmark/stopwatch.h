/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Tim Severeijns
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <chrono>
#include <type_traits>

namespace detail
{
template <typename Type> struct ChronoTypeName
{
    static constexpr auto value = "unknown units";
};

template <> struct ChronoTypeName<std::chrono::nanoseconds>
{
    static constexpr auto value = "nanoseconds";
};

template <> struct ChronoTypeName<std::chrono::microseconds>
{
    static constexpr auto value = "microseconds";
};

template <> struct ChronoTypeName<std::chrono::milliseconds>
{
    static constexpr auto value = "milliseconds";
};

template <> struct ChronoTypeName<std::chrono::seconds>
{
    static constexpr auto value = "seconds";
};

template <> struct ChronoTypeName<std::chrono::minutes>
{
    static constexpr auto value = "minutes";
};

template <> struct ChronoTypeName<std::chrono::hours>
{
    static constexpr auto value = "hours";
};
} // namespace detail

/**
 * @brief The Stopwatch class will wrap the callable object to be timed in a timing block, and then,
 * based on which constructor was called, pass the resulting timing information to either std::cout
 * or a user-defined output stream or function upon completion of timing.
 *
 * @tparam ChronoType               One of the following std::chrono time representations:
 *                                     @li std::chrono::nanoseconds
 *                                     @li std::chrono::microseconds
 *                                     @li std::chrono::milliseconds
 *                                     @li std::chrono::seconds
 *                                     @li std::chrono::minutes
 *                                     @li std::chrono::hours
 */
template <typename ChronoType> class Stopwatch
{
  public:
    /**
     * @brief This Stopwatch constructor will time the code encapsulated in the std::function object
     * and then save the result to a member variable.
     *
     * In order to retrieve the elapsed time, call GetElapsedTime(). @See GetElapsedTime().
     */
    template <typename CallableType>
    Stopwatch(CallableType&& callable) noexcept(std::is_nothrow_invocable_v<CallableType>)
    {
        ExecuteAndTime(std::forward<CallableType>(callable));
    }

    /**
     * @returns The elapsed time in ChronoType units.
     */
    ChronoType GetElapsedTime() const noexcept
    {
        return m_elapsedTime;
    }

    /**
     * @returns A character array containing the chrono resolution name.
     */
    constexpr const char* GetUnitsAsString() const noexcept
    {
        return detail::ChronoTypeName<ChronoType>::value;
    }

  private:
    template <typename CallableType>
    void ExecuteAndTime(CallableType&& callable) noexcept(std::is_nothrow_invocable_v<CallableType>)
    {
        const auto start = std::chrono::steady_clock::now();
        callable();
        const auto end = std::chrono::steady_clock::now();

        m_elapsedTime = std::chrono::duration_cast<ChronoType>(end - start);
    }

    ChronoType m_elapsedTime;
};