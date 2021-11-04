// No copyright
#pragma once

#include <imgui.h>

#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "mncore/dir.h"


namespace mnian {

class InputPopup {
 public:
  using Handler = std::function<void(const std::string&)>;


  InputPopup() = delete;
  InputPopup(const std::string& id, const char* msg);
  virtual ~InputPopup() = default;

  InputPopup(const InputPopup&) = delete;
  InputPopup(InputPopup&&) = delete;


  void Open(const std::string& data, Handler&& handler) {
    data_    = data;
    handler_ = std::move(handler);
    open_    = true;
  }

  void Update();


  const std::string& data() const {
    return data_;
  }

 protected:
  virtual std::optional<std::string> ValidateData() const {
    return std::nullopt;
  }

 private:
  std::string id_;

  const char* msg_;

  std::string data_;

  Handler handler_;

  bool open_ = false;
};

class DirItemNameInputPopup : private InputPopup {
 public:
  using Handler = InputPopup::Handler;


  DirItemNameInputPopup() = delete;
  DirItemNameInputPopup(const std::string& id, const char* msg) :
      InputPopup(id, msg) {
  }

  DirItemNameInputPopup(const DirItemNameInputPopup&) = delete;
  DirItemNameInputPopup(DirItemNameInputPopup&&) = delete;

  DirItemNameInputPopup& operator=(const DirItemNameInputPopup&) = delete;
  DirItemNameInputPopup& operator=(DirItemNameInputPopup&&) = delete;


  void Open(const core::Dir*   dir,
            const std::string& name,
            Handler&&          handler) {
    dir_  = dir;
    prev_ = name;
    InputPopup::Open(name, std::move(handler));
  }
  void Open(const core::Dir*      dir,
            const core::iDirItem* target,
            Handler&&             handler) {
    Open(dir, target->name(), std::move(handler));
  }

  void Update() {
    InputPopup::Update();
  }

 protected:
  std::optional<std::string> ValidateData() const override {
    assert(dir_);

    const auto err = core::iDirItem::ValidateName(data());
    if (err) return *err;

    if (data() == prev_) {
      return std::string("changes nothing");
    }

    if (dir_->Find(data())) {
      return std::string("name duplication");
    }
    return std::nullopt;
  }

 private:
  const core::Dir* dir_;

  std::string prev_;
};

}  // namespace mnian
