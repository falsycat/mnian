// No copyright
//
// Editor is a root of all windows to edit node or do something, which are
// called Widget.
#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <unordered_map>

#include "mncore/serialize.h"


namespace mnian::core {

class iWidget : public iPolymorphicSerializable{
 public:
  friend class WidgetStore;


  using Id = uint64_t;


  iWidget() = delete;
  explicit iWidget(const char* type) : iPolymorphicSerializable(type) {
  }

  iWidget(const iWidget&) = delete;
  iWidget(iWidget&&) = delete;

  iWidget& operator=(const iWidget&) = delete;
  iWidget& operator=(iWidget&&) = delete;


  virtual void Update() = 0;


  Id id() const {
    return id_;
  }

 protected:
  virtual void ObserveNew() {
  }

 private:
  Id id_;
};


class WidgetStore final : public iSerializable {
 public:
  using ItemMap = std::unordered_map<iWidget::Id, std::unique_ptr<iWidget>>;


  WidgetStore() = default;

  WidgetStore(const WidgetStore&) = delete;
  WidgetStore(WidgetStore&&) = delete;

  WidgetStore& operator=(const WidgetStore&) = delete;
  WidgetStore& operator=(WidgetStore&&) = delete;


  void Add(std::unique_ptr<iWidget>&& w) {
    assert(w);

    auto ptr = w.get();
    items_[next_] = std::move(w);
    ptr->id_ = next_;
    ptr->ObserveNew();

    ++next_;
  }

  bool Remove(iWidget::Id id) {
    auto itr = items_.find(id);
    if (itr == items_.end()) return false;
    items_.erase(itr);
    return true;
  }

  void Clear() {
    next_ = 0;
    items_.clear();
  }


  void Update() {
    for (auto& item : items_) {
      item.second->Update();
    }
  }


  iWidget* Find(iWidget::Id id) const {
    auto itr = items_.find(id);
    if (itr == items_.end()) return nullptr;
    return itr->second.get();
  }


  bool Deserialize(iDeserializer*);

  void Serialize(iSerializer*) const override;

 private:
  ItemMap items_;

  iWidget::Id next_ = 0;
};

}  // namespace mnian::core
