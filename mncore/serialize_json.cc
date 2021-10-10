// No copyright
#include "mncore/serialize.h"

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

#include <stack>


namespace mnian::core {

template <typename W>
class JsonSerializer : public iSerializer {
 public:
  struct State {
    enum {
      kArray,
      kObject,
    } type;
    size_t n;
  };


  JsonSerializer() = delete;
  explicit JsonSerializer(std::ostream* out) : stream_(*out), writer_(stream_) {
  }
  ~JsonSerializer() {
    writer_.Flush();
  }

  JsonSerializer(const JsonSerializer&) = delete;
  JsonSerializer(JsonSerializer&&) = delete;

  JsonSerializer& operator=(const JsonSerializer&) = delete;
  JsonSerializer& operator=(JsonSerializer&&) = delete;


  void SerializeMap(size_t n) override {
    writer_.StartObject();
    if (n) {
      stack_.push({State::kObject, n});
    } else {
      writer_.EndObject();
    }
  }
  void SerializeArray(size_t n) override {
    writer_.StartArray();
    if (n) {
      stack_.push({State::kArray, n});
    } else {
      writer_.EndArray();
    }
  }
  void SerializeKey(const std::string& key) override {
    writer_.Key(key);
  }
  void SerializeValue(const Any& value) override {
    Write(value);
    Pop();
  }


  void Write(const Any& value) {
    if (std::holds_alternative<int64_t>(value)) {
      writer_.Int64(std::get<int64_t>(value));
      return;
    }
    if (std::holds_alternative<double>(value)) {
      writer_.Double(std::get<double>(value));
      return;
    }
    if (std::holds_alternative<bool>(value)) {
      writer_.Bool(std::get<bool>(value));
      return;
    }
    if (std::holds_alternative<std::string>(value)) {
      writer_.String(std::get<std::string>(value));
      return;
    }
    assert(false);
  }
  void Pop() {
    if (!stack_.empty() && !--stack_.top().n) {
      switch (stack_.top().type) {
      case State::kArray:
        writer_.EndArray();
        break;
      case State::kObject:
        writer_.EndObject();
        break;
      }
      stack_.pop();
      return Pop();
    }
  }


  rapidjson::OStreamWrapper stream_;

  W writer_;

  std::stack<State> stack_;
};


std::unique_ptr<iSerializer> iSerializer::CreateJson(std::ostream* out) {
  assert(out);
  return std::make_unique<
      JsonSerializer<rapidjson::Writer<rapidjson::OStreamWrapper>>>(out);
}

std::unique_ptr<iSerializer> iSerializer::CreatePrettyJson(std::ostream* out) {
  assert(out);

  auto ret = std::make_unique<
      JsonSerializer<rapidjson::PrettyWriter<rapidjson::OStreamWrapper>>>(out);
  ret->writer_.SetIndent(' ', 2);
  return ret;
}

}  // namespace mnian::core
