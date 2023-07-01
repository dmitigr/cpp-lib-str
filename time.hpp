// -*- C++ -*-
//
// Copyright 2023 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_STR_TIME_HPP
#define DMITIGR_STR_TIME_HPP

#include "../base/assert.hpp"
#include "exceptions.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <string_view>

namespace dmitigr::str {

namespace detail {
constexpr const std::size_t time_buf_size{128};
} // namespace detail

/// @returns The formatted string representation of the given timepoint.
template<class Clock, class Duration>
std::string_view
to_string_view(const std::chrono::time_point<Clock, Duration> tp,
  const char* const format) noexcept
{
  DMITIGR_ASSERT(format);
  const auto tp_time_t = Clock::to_time_t(tp);
  static thread_local tm tm_val;
  static thread_local char buf[detail::time_buf_size];
  tzset();
  if (const auto len = std::strftime(buf,
      sizeof(buf), format, localtime_r(&tp_time_t, &tm_val)))
    return {buf, len};
  buf[0] = '\0';
  return {buf, 0};
}

/// @returns The ISO 8601 extended format string representation of the given timepoint.
template<class Clock, class Duration>
std::string_view
to_string_view_iso8601(const std::chrono::time_point<Clock, Duration> tp) noexcept
{
  auto result = to_string_view(tp, "%Y-%m-%dT%H:%M:%S%z:");
  char* const buf = const_cast<char*>(result.data());
  std::rotate(buf + result.size() - 3, buf + result.size() - 1, buf + result.size());
  return result;
}

/**
 * @returns The human-readable string representation of the given timepoint
 * with microseconds or empty string_view on error.
 */
template<class Clock, class Duration>
std::string_view
to_string_view_us(const std::chrono::time_point<Clock, Duration> tp) noexcept
{
  namespace chrono = std::chrono;
  const auto tse = tp.time_since_epoch();
  const auto sec = chrono::duration_cast<chrono::seconds>(tse);
  const auto us = chrono::duration_cast<chrono::microseconds>(tse - sec);

  auto result = to_string_view(tp, "%Y-%m-%dT%H:%M:%S");
  if (const auto dt_length = result.size()) {
    const auto max_rest_length = detail::time_buf_size - dt_length;
    const auto rest_length = std::snprintf(const_cast<char*>(result.data()) + dt_length,
      max_rest_length, "%c%ld", '.', us.count());
    DMITIGR_ASSERT(rest_length > 0 && static_cast<std::size_t>(rest_length) < max_rest_length);
    return {result.data(), dt_length + rest_length};
  } else
    return result;
}

/// @returns `to_string_view(Clock::now())`.
template<class Clock = std::chrono::system_clock>
inline std::string_view now(const char* const format = "%Y-%m-%dT%H:%M:%S%z") noexcept
{
  return to_string_view(Clock::now(), format);
}

/// @returns `to_string_view_iso8601(Clock::now())`.
template<class Clock = std::chrono::system_clock>
inline std::string_view now_iso8601() noexcept
{
  return to_string_view_iso8601(Clock::now());
}

/// @returns `to_string_view_us(Clock::now())`.
template<class Clock = std::chrono::system_clock>
inline std::string_view now_us() noexcept
{
  return to_string_view_us(Clock::now());
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_TIME_HPP
