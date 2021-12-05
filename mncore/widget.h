// No copyright
//
// Editor is a root of all windows to edit node or do something, which are
// called Widget.
#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include "mncore/serialize.h"


namespace mnian::core {

class iDirItem;
class iNode;


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


  template <typename T = iWidget>
  T* DeserializeWidgetRef(iDeserializer* des) const {
    static_assert(std::is_base_of<iWidget, T>::value);

    const auto id = des->value<iWidget::Id>();
    if (!id) {
      des->logger().MNCORE_LOGGER_WARN("expected widget id");
      des->LogLocation();
      return nullptr;
    }
    return dynamic_cast<T*>(Find(*id));
  }


  // Returns true if the current store is properly replaced by new one,
  // otherwise false and changes nothing.
  bool Deserialize(iDeserializer*);

  void Serialize(iSerializer*) const override;

 private:
  ItemMap items_;

  iWidget::Id next_ = 0;
};


class WidgetMap final {
 public:
  WidgetMap() = default;

  WidgetMap(const WidgetMap&) = delete;
  WidgetMap(WidgetMap&&) = delete;

  WidgetMap& operator=(const WidgetMap&) = delete;
  WidgetMap& operator=(WidgetMap&&) = delete;


  void Bind(iWidget* w, iDirItem* d) {
    wd_[w].insert(d);
    dw_[d].insert(w);
  }
  void Bind(iWidget* w, iNode* n) {
    wn_[w].insert(n);
    nw_[n].insert(w);
  }

  void Forget(iWidget* w) {
    for (auto d : wd_[w]) dw_[d].erase(w);
    for (auto n : wn_[w]) nw_[n].erase(w);
    wd_.erase(w);
    wn_.erase(w);
  }


  const std::unordered_set<iWidget*>& Find(iDirItem* d) const {
    static const std::unordered_set<iWidget*> empty_ = {};

    auto itr = dw_.find(d);
    if (itr == dw_.end()) return empty_;
    return itr->second;
  }
  const std::unordered_set<iWidget*>& Find(iNode* n) const {
    static const std::unordered_set<iWidget*> empty_ = {};

    auto itr = nw_.find(n);
    if (itr == nw_.end()) return empty_;
    return itr->second;
  }

  iWidget* Find(iDirItem* d, const char* type) const {
    const auto& wset = Find(d);
    for (auto w : wset) {
      if (w->type() == type) return w;
    }
    return nullptr;
  }
  iWidget* Find(iNode* n, const char* type) const {
    const auto& wset = Find(n);
    for (auto w : wset) {
      if (w->type() == type) return w;
    }
    return nullptr;
  }

 private:
  std::unordered_map<iWidget*, std::unordered_set<iDirItem*>> wd_;
  std::unordered_map<iDirItem*, std::unordered_set<iWidget*>> dw_;

  std::unordered_map<iWidget*, std::unordered_set<iNode*>> wn_;
  std::unordered_map<iNode*, std::unordered_set<iWidget*>> nw_;
};

}  // namespace mnian::core
