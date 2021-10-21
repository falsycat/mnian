// No copyright
//
// utilities for i18n support
#pragma once

#include <cstring>
#include <string>
#include <unordered_map>

#include "mncore/serialize.h"


namespace mnian::core {

// A set of translated text.
class Lang final {
 public:
  using Id = uint64_t;

  struct Text {
   public:
    Text() = delete;
    Text(const Lang* lang, Id id) : lang_(lang), id_(id), hash_(lang_->hash()) {
    }

    Text(const Text&) = default;
    Text(Text&&) = default;

    Text& operator=(const Text&) = default;
    Text& operator=(Text&&) = default;

    const std::string& operator*() {
      return s();
    }


    const std::string& s() {
      if (hash_ != lang_->hash() || !cache_) {
        hash_ = lang_->hash();

        // update cache
        cache_ = lang_->Translate(id_);
        if (!cache_) {
          static const std::string kMissing = "**TRANSLATION FAILRUE**";
          cache_ = &kMissing;
        }
      }
      return *cache_;
    }
    const char* c() {
      return s().c_str();
    }

   private:
    const Lang* lang_;

    Id id_;

    size_t hash_;

    const std::string* cache_ = nullptr;
  };


  static constexpr Id Hash(const char* str, size_t n) {
    Id ret = 0;
    for (size_t i = 0; i < n; ++i) {
      const auto c = str[i];

      ret *= 37;
      ret += static_cast<Id>(
          'A' <= c && c <= 'Z'? c-'A':
          'a' <= c && c <= 'z'? c-'a':
          '0' <= c && c <= '9'? c-'0'+26: 36);
    }
    return ret;
  }
  static constexpr Id Hash(const char* str) {
    size_t n = 0;
    while (*(str+n)) ++n;
    return Hash(str, n);
  }
  static Id Hash(const std::string& str) {
    return Hash(str.data(), str.size());
  }


  explicit Lang(const Lang* fallback = nullptr) : fallback_(fallback) {
  }
  explicit Lang(iDeserializer* des, const Lang* fallback = nullptr) :
      Lang(fallback) {
    assert(des);
    Deserialize(des);
  }

  Lang(const Lang&) = default;
  Lang(Lang&&) = default;

  Lang& operator=(const Lang&) = default;
  Lang& operator=(Lang&&) = default;


  bool Add(Id id, const std::string& text) {
    if (items_.find(id) != items_.end()) return false;
    items_[id] = text;

    hash_ ^= id;
    hash_ ^= std::hash<std::string>{} (text);
    return true;
  }
  bool Add(const std::string& id, const std::string& text) {
    return Add(Hash(id.c_str()), text);
  }

  void Clear() {
    items_.clear();
    hash_ = 0;
  }


  const std::string* Translate(Id id) const {
    auto itr = items_.find(id);
    if (itr == items_.end()) {
      return fallback_? fallback_->Translate(id): nullptr;
    }
    return &itr->second;
  }
  const std::string& Translate(
      const std::string& id, const std::string& def) const {
    const auto ret = Translate(Hash(id));
    return ret? *ret: def;
  }
  const std::string& Translate(const std::string& id) const {
    return Translate(id, id);
  }


  size_t hash() const {
    return (fallback_? fallback_->hash_: 0) + hash_;
  }

 private:
  void Deserialize(iDeserializer*);


  const Lang* fallback_;

  std::unordered_map<Id, std::string> items_;

  uint64_t hash_ = 0;
};

}  // namespace mnian::core
