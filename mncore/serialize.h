// No copyright
//
// This file declares utilities for serialization.
#pragma once

#include <cassert>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mncore/conv.h"
#include "mncore/logger.h"


namespace mnian::core {

class iApp;
class iSerializer;
class iDeserializer;

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


class iPolymorphicSerializable : public iSerializable {
 public:
  iPolymorphicSerializable() = delete;
  explicit iPolymorphicSerializable(const char* type) : type_(type) {
  }

  iPolymorphicSerializable(const iPolymorphicSerializable&) = default;
  iPolymorphicSerializable(iPolymorphicSerializable&&) = default;

  iPolymorphicSerializable& operator=(
      const iPolymorphicSerializable&) = default;
  iPolymorphicSerializable& operator=(iPolymorphicSerializable&&) = default;


  void Serialize(iSerializer*) const override;


  std::string_view type() const {
    return type_;
  }

 protected:
  virtual void SerializeParam(iSerializer*) const = 0;

 private:
  const char* type_;
};


// This is an interface of serializer.
class iSerializer {
 public:
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
  class MapGuard final : public iSerializable {
   public:
    MapGuard() = delete;
    explicit MapGuard(iSerializer* serializer) : serializer_(serializer) {
      assert(serializer_);
    }
    MapGuard(iSerializer* serializer, size_t reserve) : MapGuard(serializer) {
      items_.reserve(reserve);
    }
    ~MapGuard() {
      if (!serializer_) return;
      Serialize(serializer_);
    }

    MapGuard(const MapGuard&) = delete;
    MapGuard(MapGuard&&) = delete;

    MapGuard& operator=(const MapGuard&) = delete;
    MapGuard& operator=(MapGuard&&) = delete;


    void Add(const std::string& key, std::function<void()>&& func) {
      assert(serializer_);
      items_.emplace_back(key, std::move(func));
    }

    void Add(const std::string& key, Any&& v) {
      Add(key,
          [this, v = std::move(v)]() {
            serializer_->SerializeValue(std::move(v));
          });
    }
    void Add(const std::string& key, const Any& v) {
      Add(key, [this, v]() { serializer_->SerializeValue(v); });
    }
    void Add(const std::string& key, const iSerializable* s) {
      Add(key, [this, s]() { s->Serialize(serializer_); });
    }

   protected:
    void Serialize(iSerializer* serializer) const override;

   private:
    iSerializer* serializer_;

    std::vector<std::pair<std::string, std::function<void()>>> items_;
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
  class ArrayGuard final : public iSerializable {
   public:
    ArrayGuard() = delete;
    explicit ArrayGuard(iSerializer* serial) : serializer_(serial) {
      assert(serializer_);
    }
    ArrayGuard(iSerializer* serial, size_t reserve) : ArrayGuard(serial) {
      items_.reserve(reserve);
    }
    ~ArrayGuard() {
      if (!serializer_) return;
      Serialize(serializer_);
    }

    ArrayGuard(const ArrayGuard&) = delete;
    ArrayGuard(ArrayGuard&&) = delete;

    ArrayGuard& operator=(const ArrayGuard&) = delete;
    ArrayGuard& operator=(ArrayGuard&&) = delete;


    void Add(std::function<void()>&& func) {
      assert(serializer_);
      items_.emplace_back(std::move(func));
    }

    void Add(Any&& v) {
      Add([this, v = std::move(v)]() {
            serializer_->SerializeValue(std::move(v));
          });
    }
    void Add(const Any& v) {
      Add([this, v]() { serializer_->SerializeValue(v); });
    }
    void Add(const iSerializable* s) {
      Add([this, s]() { s->Serialize(serializer_); });
    }

   protected:
    void Serialize(iSerializer* serializer) const override;

   private:
    iSerializer* serializer_;

    std::vector<std::function<void()>> items_;
  };


  static std::unique_ptr<iSerializer> CreateJson(std::ostream* out);

  static std::unique_ptr<iSerializer> CreatePrettyJson(std::ostream* out);


  iSerializer() = default;
  virtual ~iSerializer() = default;

  iSerializer(const iSerializer&) = default;
  iSerializer(iSerializer&&) = default;

  iSerializer& operator=(const iSerializer&) = default;
  iSerializer& operator=(iSerializer&&) = default;


  virtual void SerializeMap(size_t n) = 0;
  virtual void SerializeArray(size_t n) = 0;
  virtual void SerializeKey(const std::string& key) = 0;
  virtual void SerializeValue(const Any& value) = 0;


  // Sugar syntax of key-value pair serialization.
  void SerializeKeyValue(const std::string& key, Any&& value) {
    SerializeKey(key);
    SerializeValue(std::move(value));
  }
};


// This is like a DI container for deserializing.
class DeserializerRegistry final {
 public:
  using Factory =
      std::function<std::unique_ptr<iPolymorphicSerializable>(iDeserializer*)>;

  using FactorySet = std::unordered_map<std::string, Factory>;


  DeserializerRegistry() = default;

  DeserializerRegistry(const DeserializerRegistry&) = delete;
  DeserializerRegistry(DeserializerRegistry&&) = delete;

  DeserializerRegistry& operator=(const DeserializerRegistry&) = delete;
  DeserializerRegistry& operator=(DeserializerRegistry&&) = delete;


  template <typename I>
  void RegisterFactory(const std::string& name, Factory&& factory) {
    static_assert(std::is_base_of<iPolymorphicSerializable, I>::value);

    auto& set = items_[typeid(I).hash_code()];
    assert(!set.contains(name));
    set[name] = std::move(factory);
  }

  template <typename I, typename T>
  void RegisterType() {
    static_assert(std::is_base_of<iPolymorphicSerializable, I>::value);
    static_assert(std::is_base_of<I, T>::value);

    RegisterFactory<I>(
        T::kType,
        [](iDeserializer* des) {
          return T::DeserializeParam(des);
        });
  }


  // Implementation is on the bottom of this file.
  template <typename I>
  std::unique_ptr<I> DeserializeParam(
      iDeserializer* des, const std::string& type) const;

  template <typename I>
  std::unique_ptr<I> Deserialize(iDeserializer* des) const;

 private:
  std::unordered_map<size_t, FactorySet> items_;
};


// This is an interface of deserializer.
class iDeserializer {
 public:
  using Key = std::variant<size_t, std::string>;


  class ScopeGuard final {
   public:
    ScopeGuard() = delete;
    ScopeGuard(iDeserializer* target, const Key& key) : target_(target) {
      target_->Enter(key);
    }
    ~ScopeGuard() {
      target_->Leave();
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;

    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;

   private:
    iDeserializer* target_;
  };


  static std::unique_ptr<iDeserializer> CreateJson(
      iApp*                       app,
      iLogger*                    logger,
      const DeserializerRegistry* reg,
      std::istream*               in);


  iDeserializer() = delete;
  explicit iDeserializer(iApp*                       app,
                         iLogger*                    logger,
                         const DeserializerRegistry* registry) :
      app_(app), logger_(logger), registry_(registry) {
    assert(app_);
    assert(logger_);
    assert(registry_);
  }
  virtual ~iDeserializer() = default;

  iDeserializer(const iDeserializer&) = default;
  iDeserializer(iDeserializer&&) = default;

  iDeserializer& operator=(const iDeserializer&) = default;
  iDeserializer& operator=(iDeserializer&&) = default;


  // Makes the specified item as a current target. If no such item is found,
  // value() and size() become to return nothing besides undefined() does true.
  // You can enter more deeply and recover from this state.
  void Enter(const Key& key) {
    if (undefined()) {
      ++null_depth_;
    }
    if (null_depth_ == 0) {
      stack_.push_back(DoEnter(key));
    } else {
      stack_.push_back(key);
    }
  }
  void Leave() {
    assert(!stack_.empty());

    stack_.pop_back();
    if (null_depth_) {
      --null_depth_;
    } else {
      DoLeave();
    }
  }


  // Generates a string expressing location of the current target.
  // e.g.) 'foo.bar[0].baz'
  std::string GenerateLocation() const;

  // Writes location of the current target to the logger.
  void LogLocation() const {
    logger_->MNCORE_LOGGER_WRITE(
        iLogger::kAddition, std::string("location: ")+GenerateLocation());
  }


  template <typename I>
  std::unique_ptr<I> DeserializeObject() {
    return registry_->Deserialize<I>(this);
  }


  template <typename T>
  std::optional<std::vector<T>> values() const {
    const auto n = size();
    if (!n) return std::nullopt;

    std::vector<T> ret;
    for (size_t i = 0; i < *n; ++i) {
      ScopeGuard _(const_cast<iDeserializer*>(this), i);

      auto v = value<T>();
      if (!v) return std::nullopt;
      ret.push_back(std::move(*v));
    }
    return ret;
  }
  template <typename T>
  std::vector<T> values(std::vector<T>&& def) const {
    auto ret = values<T>();
    return ret? std::move(*ret): std::move(def);
  }


  iLogger& logger() const {
    return *logger_;
  }

  const DeserializerRegistry& registry() const {
    return *registry_;
  }
  iApp& app() const {
    return *app_;
  }


  const std::vector<Key> stack() const {
    return stack_;
  }


  template <typename T = std::string>
  std::optional<T> key() const {
    if (stack_.empty() || !std::holds_alternative<T>(stack_.back())) {
      return std::nullopt;
    }
    return std::get<T>(stack_.back());
  }

  template <typename T>
  std::optional<T> value() const {
    if (!value_) return std::nullopt;
    return FromAny<T>(*value_);
  }
  template <typename T>
  T value(T def) const {
    if (!value_) return def;
    auto ret = FromAny<T>(*value_);
    return ret? *ret: def;
  }

  const std::optional<size_t>& size() const {
    return size_;
  }
  bool undefined() const {
    return !value_ && !size_;
  }

 protected:
  virtual Key DoEnter(const Key&) = 0;
  virtual void DoLeave() = 0;

  // Subclass should call Set*() methods to update current state from
  // constructor, DoEnter(), and DoLeave().
  void SetUndefined() {
    value_ = std::nullopt;
    size_  = std::nullopt;
  }
  void SetField(const Any& value) {
    value_ = value;
    size_  = std::nullopt;
  }
  void SetMapOrArray(size_t size) {
    value_ = std::nullopt;
    size_  = size;
  }

 private:
  iApp*    app_;
  iLogger* logger_;

  const DeserializerRegistry* registry_;

  std::vector<Key> stack_;
  size_t           null_depth_ = 0;

  std::optional<Any>    value_;
  std::optional<size_t> size_;
};


template <typename I>
std::unique_ptr<I> DeserializerRegistry::DeserializeParam(
    iDeserializer* des, const std::string& type) const {
  static_assert(std::is_base_of<iPolymorphicSerializable, I>::value);

  auto set = items_.find(typeid(I).hash_code());
  if (set == items_.end()) {
    const std::string iname = typeid(I).name();
    des->logger().MNCORE_LOGGER_WARN(
        "deserializer requested unknown interface: "+iname);
    des->LogLocation();
    return nullptr;
  }

  auto factory = set->second.find(type);
  if (factory == set->second.end()) {
    des->logger().MNCORE_LOGGER_WARN(
        "deserializer requested unknown object: "+type);
    des->LogLocation();
    return nullptr;
  }

  auto product = factory->second(des);
  if (!product) {
    des->logger().MNCORE_LOGGER_WARN(
        "failed to deserialize object: "+type);
    des->LogLocation();
    return nullptr;
  }

  auto ret = dynamic_cast<I*>(product.release());
  assert(ret);
  return std::unique_ptr<I>(ret);
}

template <typename I>
std::unique_ptr<I> DeserializerRegistry::Deserialize(iDeserializer* des) const {
  static_assert(std::is_base_of<iPolymorphicSerializable, I>::value);

  des->Enter(std::string("type"));
  const auto name = des->value<std::string>();
  des->Leave();
  if (!name) return nullptr;

  iDeserializer::ScopeGuard _(des, std::string("param"));
  return DeserializeParam<I>(des, *name);
}

}  // namespace mnian::core
