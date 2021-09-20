// No copyright
//
// This file declares utilities for serialization.
//
#pragma once

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>


namespace mnian::core {

class iSerializer;

// This is an interface of what can be serialized.
class iSerializable {
 public:
  iSerializable() = default;
  virtual ~iSerializable() = default;

  iSerializable(const iSerializable&) = default;
  iSerializable(iSerializable&&) = default;

  iSerializable& operator=(const iSerializable&) = default;
  iSerializable& operator=(iSerializable&&) = default;


  virtual void Serialize(iSerializer*) const = 0;
};


// This is an interface of serializer.
//
// ## Example: Serializing Map
// ```
// serializer.SerializeMap(3);  // number of key-value pairs
// serializer.SerializeKeyValue("key1", 0);
// serializer.SerializeKeyValue("key2", "helloworld");
// serializer.SerializeKeyValue("key3", 4.3);
// // output: {"key1":0,"key2":"helloworld","key3":4.3}
// ```
//
// ## Example: Serializing Array
// ```
// serializer.SerializeArray(3)
// serializer.SerializeValue(0);
// serializer.SerializeValue("helloworld");
// serializer.SerializeValue(4.3);
// // output: [0, "helloworld", 4.3]
// ```
class iSerializer {
 public:
  using Value = std::variant<int64_t, double, bool, std::string>;

  // MapGuard can provide a way as a scope guard to serialize map object safely.
  //
  // ## Example
  // ```
  // {
  //   MapGuard map(serializer1);
  //   map.Add("key1", 0);
  //   map.Add("key2", "helloworld");
  //   map.Add("key3", serializer2);  // {"subkey1":4.3}
  // }
  // // output: {"key1":0,"key2":"helloworld","key3":{"subkey1":4.3}}
  // ```
  struct MapGuard {
   public:
    using Item = std::variant<Value, const iSerializable*>;


    MapGuard() = delete;
    explicit MapGuard(iSerializer* serializer) : serializer_(serializer) {
    }
    MapGuard(iSerializer* serializer, size_t reserve) : MapGuard(serializer) {
      items_.reserve(reserve);
    }
    ~MapGuard() {
      serializer_->SerializeMap(items_.size());
      for (auto& item : items_) {
        serializer_->SerializeKey(item.first);
        if (std::holds_alternative<const iSerializable*>(item.second)) {
          std::get<const iSerializable*>(item.second)->Serialize(serializer_);
        } else {
          serializer_->SerializeValue(std::move(std::get<Value>(item.second)));
        }
      }
    }

    MapGuard(const MapGuard&) = delete;
    MapGuard(MapGuard&&) = delete;

    MapGuard& operator=(const MapGuard&) = delete;
    MapGuard& operator=(MapGuard&&) = delete;


    void Add(const std::string& key, Value&& value) {
      items_.emplace_back(key, std::move(value));
    }
    void Add(const std::string& key, const Value& value) {
      items_.emplace_back(key, value);
    }
    void Add(const std::string& key, const iSerializable* serializable) {
      items_.emplace_back(key, serializable);
    }

   private:
    iSerializer* serializer_;

    std::vector<std::pair<std::string, Item>> items_;
  };

  // ArrayGuard can provide a way as a scope guard to serialize array object
  // safely.
  //
  // ## Example
  // ```
  // {
  //   ArrayGuard array(serializer1);
  //   array.Add(0);
  //   array.Add("helloworld");
  //   array.Add(serializer2);  // {"subkey1":4.3}
  // }
  // // output: [0,"helloworld",{"subkey1":4.3}]
  // ```
  struct ArrayGuard {
   public:
    using Item = std::variant<Value, const iSerializable*>;


    ArrayGuard() = delete;
    explicit ArrayGuard(iSerializer* serial) : serializer_(serial) {
    }
    ArrayGuard(iSerializer* serial, size_t reserve) : ArrayGuard(serial) {
      items_.reserve(reserve);
    }
    ~ArrayGuard() {
      serializer_->SerializeArray(items_.size());
      for (auto& item : items_) {
        if (std::holds_alternative<const iSerializable*>(item)) {
          std::get<const iSerializable*>(item)->Serialize(serializer_);
        } else {
          serializer_->SerializeValue(std::get<Value>(item));
        }
      }
    }

    ArrayGuard(const ArrayGuard&) = delete;
    ArrayGuard(ArrayGuard&&) = delete;

    ArrayGuard& operator=(const ArrayGuard&) = delete;
    ArrayGuard& operator=(ArrayGuard&&) = delete;


    void Add(Value&& value) {
      items_.emplace_back(std::move(value));
    }
    void Add(const Value& value) {
      items_.emplace_back(value);
    }
    void Add(const iSerializable* serializable) {
      items_.emplace_back(serializable);
    }

   private:
    iSerializer* serializer_;

    std::vector<Item> items_;
  };


  iSerializer() = default;
  virtual ~iSerializer() = default;

  iSerializer(const iSerializer&) = default;
  iSerializer(iSerializer&&) = default;

  iSerializer& operator=(const iSerializer&) = default;
  iSerializer& operator=(iSerializer&&) = default;


  virtual void SerializeMap(size_t n) = 0;
  virtual void SerializeArray(size_t n) = 0;
  virtual void SerializeKey(const std::string& key) = 0;
  virtual void SerializeValue(const Value& value) = 0;


  // Sugar syntax of key-value pair serialization.
  void SerializeKeyValue(const std::string& key, Value&& value) {
    SerializeKey(key);
    SerializeValue(std::move(value));
  }
};


// This is an interface of deserializer.
//
// ## Example: Deserialize Map
// ```
// // input: {"key1":0,"key2":"helloworld","key3":4.3}
// assert(deserializer.size() == 3);
//
// deserializer.EnterByKey("key1");
// assert(deserializer.value() == 0);
// deserializer.Leave();
//
// deserializer.EnterByKey("key2");
// assert(deserializer.value() == "helloworld");
// deserializer.Leave();
//
// deserializer.EnterByKey("key3");
// assert(deserializer.value() == 4.3);
// deserializer.Leave();
// ```
//
// ## Example: Deserialize Array
// ```
// // input: [0,"helloworld",4.3]
// assert(deserializer.size() == 3);
//
// deserializer.EnterByIndex(0);
// assert(deserializer.GetValue() == 0);
// deserializer.Leave();
//
// deserializer.EnterByIndex(1);
// assert(deserializer.GetValue() == "helloworld");
// deserializer.Leave();
//
// deserializer.EnterByIndex(2);
// assert(deserializer.GetValue() == 4.3);
// deserializer.Leave();
// ```
class iDeserializer {
 public:
  using Value = iSerializer::Value;


  // Converts a value of `in` into `out`.
  template <typename T>
  static bool ConvertFromValue(T* out, const Value& in) {
    // Checks exact types firstly.
    if constexpr (std::is_same<T, bool>::value) {
      if (std::holds_alternative<std::string>(in)) {
        // Converts to bool from string.
        const std::string& x = std::get<std::string>(in);

        const bool t = (x == "true");
        if (t == (x == "false")) {
          return false;
        }
        *out = t;
        return true;
      }
      if (std::holds_alternative<bool>(in)) {
        // Converts to bool from bool.
        *out = std::get<bool>(in);
        return true;
      }
      return false;

    } else if constexpr (std::is_same<T, std::string>::value) {
      // Converts to string from any types.
      if (std::holds_alternative<std::string>(in)) {
        *out = std::get<std::string>(in);
        return true;
      }
      if (std::holds_alternative<bool>(in)) {
        *out = std::get<bool>(in)? "true": "false";
        return true;
      }
      if (std::holds_alternative<int64_t>(in)) {
        *out = std::to_string(std::get<int64_t>(in));
        return true;
      }
      if (std::holds_alternative<double>(in)) {
        *out = std::to_string(std::get<double>(in));
        return true;
      }
      return false;

    } else if constexpr (std::is_integral<T>::value) {
      using L = std::numeric_limits<T>;

      if (std::holds_alternative<std::string>(in)) {
        // Converts to integral from string.
        char* end;
        if constexpr (std::is_unsigned<T>::value) {
          const uintmax_t ret = std::strtoumax(
              std::get<std::string>(in).c_str(), &end, 0);
          if (L::max() < ret || *end != 0) {
            return false;
          }
          *out = static_cast<T>(ret);
          return true;
        } else {
          const intmax_t ret = std::strtoimax(
              std::get<std::string>(in).c_str(), &end, 0);
          if (ret < L::min() || L::max() < ret || *end != 0) {
            return false;
          }
          *out = static_cast<T>(ret);
          return true;
        }
        // UNREACHABLE
      }
      if (std::holds_alternative<int64_t>(in)) {
        // Converts to integral from integral.
        const int64_t x = std::get<int64_t>(in);
        if (x < L::min() || L::max() < x) {
          return false;
        }
        *out = static_cast<T>(x);
        return true;
      }
      if (std::holds_alternative<double>(in)) {
        // Converts to integral from floating.
        const double x = std::get<double>(in);
        if (!std::isfinite(x) || x < L::min() || L::max() < x) {
          return false;
        }
        *out = static_cast<T>(x);
        return true;
      }
      return false;

    } else if constexpr (std::is_floating_point<T>::value) {
      if (std::holds_alternative<std::string>(in)) {
        // Converts to floating from string.
        char* end;
        const T ret = static_cast<T>(
            std::strtod(std::get<std::string>(in).c_str(), &end));
        if (*end != 0 || !std::isfinite(ret)) {
          return false;
        }
        *out = ret;
        return true;
      }
      if (std::holds_alternative<int64_t>(in)) {
        // Converts to floating from integral.
        *out = static_cast<T>(std::get<int64_t>(in));
        return true;
      }
      if (std::holds_alternative<double>(in)) {
        // Converts to floating from floating.
        const T ret = static_cast<T>(std::get<double>(in));
        if (!std::isfinite(ret)) {
          return false;
        }
        *out = ret;
        return true;
      }
      return false;

    } else {
      []<bool f = false>() { static_assert(f, "unknown output type"); }();
    }
  }


  iDeserializer() = default;
  virtual ~iDeserializer() = default;

  iDeserializer(const iDeserializer&) = default;
  iDeserializer(iDeserializer&&) = default;

  iDeserializer& operator=(const iDeserializer&) = default;
  iDeserializer& operator=(iDeserializer&&) = default;


  // Makes the specified item as a current target. If no such item is found,
  // Enter*() returns false, and value() and size() return nothing. You can
  // enter more deeply by Enter*() and recover from this state by Leave().
  // EnterByKey() is available only when the current target is a map.
  // EnterByIndex() is available only when it's an array.
  virtual bool EnterByKey(const std::string& key) = 0;
  virtual bool EnterByIndex(size_t index) = 0;

  virtual void Leave() = 0;

  virtual std::optional<Value> value() const = 0;
  virtual std::optional<size_t> size() const = 0;


  std::optional<Value> FindValue(const std::string& key) {
    EnterByKey(key);
    auto v = value();
    Leave();
    return v;
  }
  std::optional<Value> FindValue(size_t index) {
    EnterByIndex(index);
    auto v = value();
    Leave();
    return v;
  }

  template <typename T>
  bool FindValue(const std::string& key, T* out) {
    const auto in = FindValue(key);
    if (!in) return false;
    return ConvertFromValue(out, in);
  }
  template <typename T>
  bool FindValue(size_t index, T* out) {
    const auto in = FindValue(index);
    if (!in) return false;
    return ConvertFromValue(out, in);
  }

  template <typename T>
  T FindValueOr(const std::string& key, T def) {
    const auto in = FindValue(key);
    T out = def;
    return in && ConvertFromValue(&out, in)? out: def;
  }
  template <typename T>
  T FindValueOr(size_t index, T def) {
    const auto in = FindValue(index);
    T out = def;
    return in && ConvertFromValue(&out, in)? out: def;
  }
};

}  // namespace mnian::core