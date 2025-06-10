// ==========================================================================
// Copyright (C) 2025 by Genetec, Inc.
// 
// This file is under the MIT License (MIT).
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ==========================================================================

#pragma once

#include <ctime>
#include <chrono>
#include <iomanip>

typedef std::chrono::time_point<std::chrono::system_clock> SomeTime;
typedef std::chrono::system_clock::duration SomeDuration;
inline SomeTime Now() {return SomeTime::clock::now();}

// --------------------------------------------------------------------------
inline std::string Iso8601WithMs(SomeTime timePoint)
{
    auto tp_time_t = std::chrono::system_clock::to_time_t(timePoint);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()) % 1000;

    std::tm tm_point;
    localtime_r(&tp_time_t, &tm_point);

    std::ostringstream oss;
    oss << std::put_time(&tm_point, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

// --------------------------------------------------------------------------
inline std::string FormatDuration(std::chrono::system_clock::duration dur)
{
    using namespace std::chrono;
    auto us = duration_cast<microseconds>(dur);
    auto h = duration_cast<hours>(us);
    us -= h;
    auto m = duration_cast<minutes>(us);
    us -= m;
    auto s = duration_cast<seconds>(us);
    us -= s;
    auto ms = duration_cast<milliseconds>(us);
    us -= ms;

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << h.count() << ":"
        << std::setw(2) << m.count() << ":"
        << std::setw(2) << s.count() << "."
        << std::setw(3) << ms.count()
        << std::setw(4) << us.count(); // microseconds, 4 digits for up to 9999

    return oss.str();
}