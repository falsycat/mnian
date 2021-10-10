// No copyright
//
// Action represents a command executed by user interaction. It has also visual
// data such as display name, description, and icon.
//
#pragma once

#include <string>
#include <utility>
#include <vector>


namespace mnian::core {

// This is an interface of Action. To create new Action, inherit this and
// override Exec() method.
class iAction {
 public:
  struct Meta {
    std::string name;
    std::string description;
  };

  enum Reason {
    kUnknown,
  };
  struct Param {
    Reason reason;
  };

  enum Flag : uint16_t {
    kEnabled = uint16_t{1} << 0,
    kShown   = uint16_t{1} << 1,
  };
  using Flags = uint16_t;


  static constexpr Flags kDefaultFlags = kEnabled | kShown;


  iAction() = delete;
  explicit iAction(Meta&& meta) : meta_(std::move(meta)) {
  }
  virtual ~iAction() = default;

  iAction(const iAction&) = delete;
  iAction(iAction&&) = delete;

  iAction& operator=(const iAction&) = delete;
  iAction& operator=(iAction&&) = delete;


  virtual void Exec(const Param&) = 0;


  void SetFlags(Flags v) {
    flags_ |= v;
  }
  void UnsetFlags(Flags v) {
    flags_ = static_cast<Flags>(flags_ & ~v);
  }


  const std::string& name() const {
    return meta_.name;
  }
  const std::string& description() const {
    return meta_.description;
  }

  Flags flags() const {
    return flags_;
  }

 private:
  const Meta meta_;

  Flags flags_ = kDefaultFlags;
};


// This is an interface of Actionable, which means what has one or more actions
// related to itself.
// Use Enable/Shown flags from iAction to change the list of them, because it's
// immutable and built at constructor. Don't forget that all of
// Action passed to the constructor must be owned by `this`, and they don't need
// to be initialized at this time.
class iActionable {
 public:
  using ActionList = std::vector<const iAction*>;


  iActionable() = delete;
  explicit iActionable(ActionList&& actions) : actions_(std::move(actions)) {
  }
  virtual ~iActionable() = default;

  iActionable(const iActionable&) = delete;
  iActionable(iActionable&&) = delete;

  iActionable& operator=(const iActionable&) = delete;
  iActionable& operator=(iActionable&&) = delete;


  const std::vector<const iAction*>& actions() const {
    return actions_;
  }

 private:
  const ActionList actions_;
};


// This is an Action that does nothing.
class NullAction : public iAction {
 public:
  static inline const Meta kDefaultMeta = {
    .name        = "null",
    .description = "does nothing",
  };


  NullAction() : iAction(Meta(kDefaultMeta)) {
  }
  explicit NullAction(Meta&& meta) : iAction(std::move(meta)) {
  }

  NullAction(const NullAction&) = delete;
  NullAction(NullAction&&) = delete;

  NullAction& operator=(const NullAction&) = delete;
  NullAction& operator=(NullAction&&) = delete;


  void Exec(const Param&) override {
  }
};

}  // namespace mnian::core
