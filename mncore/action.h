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


  iAction() = default;
  explicit iAction(Flags flags) : flags_(flags) {
  }
  virtual ~iAction() = default;

  iAction(const iAction&) = delete;
  iAction(iAction&&) = delete;

  iAction& operator=(const iAction&) = delete;
  iAction& operator=(iAction&&) = delete;


  virtual void Exec(const Param&) const = 0;

  virtual std::string GetName() const = 0;
  virtual std::string GetDescription() const = 0;


  void SetFlags(Flags v) {
    flags_ |= v;
  }
  void UnsetFlags(Flags v) {
    flags_ = static_cast<Flags>(flags_ & ~v);
  }


  Flags flags() const {
    return flags_;
  }

 private:
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
  NullAction() = default;

  NullAction(const NullAction&) = delete;
  NullAction(NullAction&&) = delete;

  NullAction& operator=(const NullAction&) = delete;
  NullAction& operator=(NullAction&&) = delete;


  void Exec(const Param&) const override {
  }

  std::string GetName() const override {
    return "NullAction";
  }
  std::string GetDescription() const override {
    return "does nothing";
  }
};

}  // namespace mnian::core
