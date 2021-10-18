// No copyright
#include "mncore/serialize.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

#include <iostream>
#include <stack>
#include <tuple>


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
      Pop();
    }
  }
  void SerializeArray(size_t n) override {
    writer_.StartArray();
    if (n) {
      stack_.push({State::kArray, n});
    } else {
      writer_.EndArray();
      Pop();
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


class JsonDeserializer : public iDeserializer {
 public:
  using Enc = rapidjson::UTF8<>;


  static constexpr auto kFlags =
      rapidjson::kParseValidateEncodingFlag |
      rapidjson::kParseIterativeFlag        |
      rapidjson::kParseStopWhenDoneFlag     |
      rapidjson::kParseCommentsFlag         |
      rapidjson::kParseTrailingCommasFlag;


  JsonDeserializer() = delete;
  JsonDeserializer(iApp*                       app,
                   iLogger*                    logger,
                   const DeserializerRegistry* reg) :
      iDeserializer(app, logger, reg), stack_({&doc_}) {
  }

  JsonDeserializer(const JsonDeserializer&) = delete;
  JsonDeserializer(JsonDeserializer&&) = delete;

  JsonDeserializer& operator=(const JsonDeserializer&) = delete;
  JsonDeserializer& operator=(JsonDeserializer&&) = delete;


  Key DoEnter(const Key& key) override {
    auto [realkey, value] = Stack(key);
    SetValue(value);
    stack_.push(std::move(value));
    return realkey;
  }

  void DoLeave() override {
    stack_.pop();
    SetValue(&cur());
  }


  std::tuple<Key, rapidjson::GenericValue<Enc>*> Stack(const Key& key) {
    if (cur().IsObject() && std::holds_alternative<std::string>(key)) {
      const auto  obj = cur().GetObject();
      const auto& str = std::get<std::string>(key);

      const auto itr = obj.FindMember(str);
      if (itr != obj.MemberEnd()) {
        return {key, &itr->value};
      }
      return {key, nullptr};
    }
    if (cur().IsObject() && std::holds_alternative<size_t>(key)) {
      const auto obj = cur().GetObject();

      const auto index = std::get<size_t>(key);
      if (index >= obj.MemberCount()) {
        return {key, nullptr};
      }

      const auto itr = obj.MemberBegin() + static_cast<intmax_t>(index);
      return {Key(itr->name.GetString()), &itr->value};
    }
    if (cur().IsArray() && std::holds_alternative<size_t>(key)) {
      const auto arr = cur().GetArray();

      const auto index = std::get<size_t>(key);
      if (index < arr.Size()) {
        return {key, &arr[static_cast<rapidjson::SizeType>(index)]};
      }
      return {key, nullptr};
    }
    return {key, nullptr};
  }

  void SetValue(const rapidjson::GenericValue<Enc>* value) {
    const auto type = value? value->GetType(): rapidjson::kNullType;
    switch (type) {
    case rapidjson::kNullType:
      SetUndefined();
      break;
    case rapidjson::kFalseType:
      SetField(false);
      break;
    case rapidjson::kTrueType:
      SetField(true);
      break;
    case rapidjson::kObjectType:
      SetMapOrArray(value->GetObject().MemberCount());
      break;
    case rapidjson::kArrayType:
      SetMapOrArray(value->GetArray().Size());
      break;
    case rapidjson::kStringType:
      SetField(std::string(value->GetString()));
      break;
    case rapidjson::kNumberType:
      if (value->IsInt64()) {
        SetField(value->GetInt64());
      } else {
        SetField(value->GetDouble());
      }
      break;
    default:
      assert(false);
    }
  }


  rapidjson::GenericValue<Enc>& cur() const {
    return *stack_.top();
  }

  rapidjson::GenericDocument<Enc> doc_;

  std::stack<rapidjson::GenericValue<Enc>*> stack_;
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


std::unique_ptr<iDeserializer> iDeserializer::CreateJson(
    iApp*                       app,
    iLogger*                    logger,
    const DeserializerRegistry* reg,
    std::istream*               in) {
  assert(in);

  auto ret = std::make_unique<JsonDeserializer>(app, logger, reg);

  auto& d = ret->doc_;

  rapidjson::IStreamWrapper st(*in);
  d.ParseStream<JsonDeserializer::kFlags>(st);

  if (d.HasParseError()) {
    logger->MNCORE_LOGGER_WARN("JSON parse error");
    logger->MNCORE_LOGGER_WRITE(
        iLogger::kAddition, rapidjson::GetParseError_En(d.GetParseError()));
    logger->MNCORE_LOGGER_WRITE(
        iLogger::kAddition, "at "+std::to_string(d.GetErrorOffset()));
    return nullptr;
  }

  ret->SetValue(&d);
  return ret;
}

}  // namespace mnian::core
