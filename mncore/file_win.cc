// No copyright
//
// This file is compiled under only Windows build.
#include "mncore/file.h"

#define NOMINMAX
#include <windows.h>
#undef NOMINMAX


namespace mnian::core {

class WinFile : public iFile {
 public:
  static constexpr auto DWORD_MAX    = std::numeric_limits<DWORD>::max();
  static constexpr auto LONGLONG_MAX = std::numeric_limits<LONGLONG>::max();


  WinFile() = delete;
  WinFile(const std::filesystem::path& path, HANDLE fh) :
      iFile("file://"+path.generic_string()), fh_(fh) {
  }
  ~WinFile() {
    CloseHandle(fh_);
  }

  WinFile(const WinFile&) = delete;
  WinFile(WinFile&&) = delete;

  WinFile& operator=(const WinFile&) = delete;
  WinFile& operator=(WinFile&&) = delete;


  size_t Read(uint8_t* buf, size_t size, size_t offset) override {
    if (size > DWORD_MAX) size = DWORD_MAX;

    OVERLAPPED overlap = {
      .Offset     = static_cast<DWORD>((offset >>  0) & 0xFFFFFFFF),
      .OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF),
    };
    DWORD read;
    if (!ReadFile(fh_, buf, static_cast<DWORD>(size), &read, &overlap)) {
      return 0;
    }
    return static_cast<size_t>(read);
  }
  size_t Write(const uint8_t* buf, size_t size, size_t offset) override {
    if (size > DWORD_MAX) size = DWORD_MAX;

    OVERLAPPED overlap = {
      .Offset     = static_cast<DWORD>((offset >>  0) & 0xFFFFFFFF),
      .OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF),
    };
    DWORD wrote;
    if (!WriteFile(fh_, buf, static_cast<DWORD>(size), &wrote, &overlap)) {
      return 0;
    }
    return static_cast<size_t>(wrote);
  }
  bool Truncate(size_t size) override {
    if (size > LONGLONG_MAX) return false;

    LARGE_INTEGER sz = {
      .QuadPart = static_cast<LONGLONG>(size),
    };
    return
        SetFilePointerEx(fh_, sz, nullptr, FILE_BEGIN) &&
        SetEndOfFile(fh_);
  }
  bool Flush() override {
    return FlushFileBuffers(fh_);
  }

  std::filesystem::file_time_type GetLastModified() const override {
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(fh_, &info)) {
      return {};
    }
    const auto& ft = info.ftLastWriteTime;

    const auto t =
        (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    return std::filesystem::file_time_type(std::chrono::milliseconds(t/100));
  }

 private:
  HANDLE fh_;
};


std::unique_ptr<iFile> iFile::CreateForNative(
    const std::filesystem::path& path) {
  const auto str = path.wstring();

  const HANDLE hnd = CreateFileW(
      str.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0,        /* = don't share */
      nullptr,  /* = no security specification */
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL);
  if (hnd == INVALID_HANDLE_VALUE) return nullptr;

  return std::make_unique<WinFile>(path, hnd);
}

}  // namespace mnian::core
