// No copyright
//
// This file declares utilities for serialization.
#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "mncore/app.h"
#include "mncore/conv.h"
#include "mncore/logger.h"


namespace mnian::core {

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
    using Item = std::variant<Any, const iSerializable*>;


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


    void Add(const std::string& key, Any&& value) {
      assert(serializer_);
      items_.emplace_back(key, std::move(value));
    }
    void Add(const std::string& key, const Any& value) {
      assert(serializer_);
      items_.emplace_back(key, value);
    }

    // The serializable object must be alive longer than `this`.
    void Add(const std::string& key, const iSerializable* serializable) {
      assert(serializer_);
      items_.emplace_back(key, serializable);
    }

   protected:
    void Serialize(iSerializer* serializer) const override;

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
  class ArrayGuard final : public iSerializable {
   public:
    using Item = std::variant<Any, const iSerializable*>;


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


    void Add(Any&& value) {
      items_.emplace_back(std::move(value));
    }
    void Add(const Any& value) {
      items_.emplace_back(value);
    }

    // The serializable object must be alive longer than `this`.
    void Add(const iSerializable* serializable) {
      items_.emplace_back(serializable);
    }

   protected:
    void Serialize(iSerializer* serializer) const override;

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
  virtual void SerializeValue(const Any& value) = 0;


  // Sugar syntax of key-value pair serialization.
  void SerializeKeyValue(const std::string& key, Any&& value) {
    SerializeKey(key);
    SerializeValue(std::move(value));
  }
};


// This is like a DI container for deserializing.
class Registry final {
 public:
  using Factory =
      std::function<std::unique_ptr<iPolymorphicSerializable>(iDeserializer*)>;

  using FactorySet = std::map<std::string, Factory>;


  Registry() = delete;
  explicit Registry(iApp* app) : app_(app) {
    assert(app_);
  }

  Registry(const Registry&) = delete;
  Registry(Registry&&) = delete;

  Registry& operator=(const Registry&) = delete;
  Registry& operator=(Registry&&) = delete;


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


  iApp& app() const {
    return *app_;
  }

 private:
  iApp* app_;

  std::map<size_t, FactorySet> items_;
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


  iDeserializer() = delete;
  explicit iDeserializer(iLogger* logger, const Registry* registry) :
      logger_(logger), registry_(registry) {
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


  iLogger& logger() const {
    return *logger_;
  }

  const Registry& registry() const {
    return *registry_;
  }
  iApp& app() const {
    return registry_->app();
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
  iLogger* logger_;

  const Registry* registry_;

  std::vector<Key> stack_;
  size_t           null_depth_ = 0;

  std::optional<Any>    value_;
  std::optional<size_t> size_;
};


template <typename I>
std::unique_ptr<I> Registry::DeserializeParam(
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
std::unique_ptr<I> Registry::Deserialize(iDeserializer* des) const {
  static_assert(std::is_base_of<iPolymorphicSerializable, I>::value);

  des->Enter(std::string("type"));
  const auto name = des->value<std::string>();
  des->Leave();
  if (!name) return nullptr;

  iDeserializer::ScopeGuard _(des, std::string("param"));
  return DeserializeParam<I>(des, *name);
}

}  // namespace mnian::core
