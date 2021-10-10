// No copyright
//
// Editor is a root of all windows to edit node or do something, which are
// called Widget.
#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "mncore/serialize.h"


namespace mnian::core {

class iWidget : public iPolymorphicSerializable{
 public:
  iWidget() = delete;
  explicit iWidget(const char* type) : iPolymorphicSerializable(type) {
  }

  iWidget(const iWidget&) = delete;
  iWidget(iWidget&&) = delete;

  iWidget& operator=(const iWidget&) = delete;
  iWidget& operator=(iWidget&&) = delete;


  virtual void Update() = 0;
};


class iEditor : public iWidget {
 public:
  iEditor() = delete;
  explicit iEditor(const char* type) : iWidget(type) {
  }

  iEditor(const iEditor&) = delete;
  iEditor(iEditor&&) = delete;

  iEditor& operator=(const iEditor&) = delete;
  iEditor& operator=(iEditor&&) = delete;


  virtual void Add(std::unique_ptr<iWidget>&&) = 0;
  virtual void Remove(const iWidget*) = 0;
};

}  // namespace mnian::core
