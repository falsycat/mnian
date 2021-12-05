// No copyright
//
// Provides utility to convert between different types safely. Maybe useful when
// deserializing.
#pragma once

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>


namespace mnian::core {

// Variant type used for serialization
using Any = std::variant<
    int64_t,
    double,
    bool,
    std::string>;

// Variant type used for Lambda I/O
using SharedAny = std::variant<
    int64_t,
    double,
    bool,
    std::shared_ptr<std::string>>;


template <typename R, typename T>
std::optional<R> ToInt(T in) {
  static_assert(std::is_integral<R>::value);

  if constexpr (std::is_integral<T>::value) {
    if constexpr (std::is_signed<R>::value && std::is_signed<T>::value) {
      using L = std::numeric_limits<R>;
      if (in < L::min() || L::max() < in) {
        return std::nullopt;
      }
      return static_cast<R>(in);

    } else if constexpr (std::is_unsigned<T>::value) {
      using L = std::numeric_limits<R>;
      if (static_cast<uintmax_t>(L::max()) < static_cast<uintmax_t>(in)) {
        return std::nullopt;
      }
      return static_cast<R>(in);

    } else {
      using L = std::numeric_limits<R>;
      if (in < T{0} ||
          static_cast<uintmax_t>(L::max()) < static_cast<uintmax_t>(in)) {
        return std::nullopt;
      }
      return static_cast<R>(in);
    }
    // UNREACHABLE

  } else {
    (void) in;
    return std::nullopt;
  }
}
template <typename R>
std::optional<R> ToInt(const std::string& in) {
  static_assert(std::is_integral<R>::value);

  char* end;
  if constexpr (std::is_unsigned<R>::value) {
    const uintmax_t ret = std::strtoumax(in.c_str(), &end, 0);
    if (*end) return std::nullopt;
    return ToInt<R>(ret);
  } else {
    const intmax_t ret = std::strtoimax(in.c_str(), &end, 0);
    if (*end) return std::nullopt;
    return ToInt<R>(ret);
  }
}

template <typename R, typename T>
std::optional<R> ToFloat(T in) {
  static_assert(std::is_floating_point<R>::value);

  if constexpr (std::is_floating_point<T>::value) {
    if (!std::isfinite(in)) return std::nullopt;
    return static_cast<R>(in);
  } else if constexpr (std::is_integral<T>::value) {
    return static_cast<R>(in);
  } else {
    (void) in;
    return std::nullopt;
  }
}
template <typename R>
std::optional<R> ToFloat(const std::string& in) {
  static_assert(std::is_floating_point<R>::value);

  char* end;
  const double ret = std::strtod(in.c_str(), &end);
  if (*end) return std::nullopt;
  return ToFloat<R>(ret);
}

template <typename T>
std::optional<std::string> ToStr(T in) {
  if constexpr (std::is_integral<T>::value ||
                std::is_floating_point<T>::value) {
    return std::to_string(in);
  } else {
    (void) in;
    return std::nullopt;
  }
}
static inline std::optional<std::string> ToStr(const std::string& in) {
  return in;
}
static inline std::optional<std::string> ToStr(bool in) {
  return in? "true": "false";
}

template <typename T>
std::optional<bool> ToBool(T) {
  return std::nullopt;
}
static inline std::optional<bool> ToBool(const std::string& in) {
  const bool ret = (in == "true");
  if (ret == (in == "false")) {
    return std::nullopt;
  }
  return ret;
}
static inline std::optional<bool> ToBool(bool in) {
  return in;
}


static inline Any FromSharedAny(const SharedAny& in) {
  if (std::holds_alternative<int64_t>(in)) {
    return std::get<int64_t>(in);
  }
  if (std::holds_alternative<double>(in)) {
    return std::get<double>(in);
  }
  if (std::holds_alternative<bool>(in)) {
    return std::get<bool>(in);
  }
  if (std::holds_alternative<std::shared_ptr<std::string>>(in)) {
    return *std::get<std::shared_ptr<std::string>>(in);
  }
  assert(false);
  return {};
}

static inline SharedAny ToSharedAny(const Any& in) {
  if (std::holds_alternative<int64_t>(in)) {
    return std::get<int64_t>(in);
  }
  if (std::holds_alternative<double>(in)) {
    return std::get<double>(in);
  }
  if (std::holds_alternative<bool>(in)) {
    return std::get<bool>(in);
  }
  if (std::holds_alternative<std::string>(in)) {
    return std::make_shared<std::string>(std::get<std::string>(in));
  }
  assert(false);
  return {};
}


template <typename R>
std::optional<R> FromAny(const Any& in) {
  // Checks exact types firstly.
  if constexpr (std::is_same<R, Any>::value) {
    return in;

  } else if constexpr (std::is_same<R, SharedAny>::value) {
    return ToSharedAny(in);

  } else if constexpr (std::is_same<R, bool>::value) {
    return std::visit([&](auto x) { return ToBool(x); }, in);

  } else if constexpr (std::is_same<R, std::string>::value) {
    return std::visit([&](auto x) { return ToStr(x); }, in);

  } else if constexpr (std::is_integral<R>::value) {
    return std::visit([&](auto x) { return ToInt<R>(x); }, in);

  } else if constexpr (std::is_floating_point<R>::value) {
    return std::visit([&](auto x) { return ToFloat<R>(x); }, in);

  } else {
    []<bool f = false>() { static_assert(f, "unknown return type"); }();
  }
}

}  // namespace mnian::core
