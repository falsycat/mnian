// No copyright
#pragma once


namespace mnian::core {

class iApp {
 public:
  iApp() = default;
  virtual ~iApp() = default;

  iApp(const iApp&) = delete;
  iApp(iApp&&) = delete;

  iApp& operator=(const iApp&) = delete;
  iApp& operator=(iApp&&) = delete;
};

}  // namespace mnian::core
