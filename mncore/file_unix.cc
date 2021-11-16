// No copyright
//
// This file is compiled under only UNIX build.
#include "mncore/file.h"

#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>


#if defined(__APPLE__) || defined(__NetBSD__)
# define st_atim st_atimespec
# define st_ctim st_ctimespec
# define st_mtim st_mtimespec
#endif


namespace mnian::core {

class UnixFile : public iFile {
 public:
  UnixFile() = delete;
  UnixFile(const std::filesystem::path& path, int fd) :
      iFile("file://"+path.string()), fd_(fd) {
  }
  ~UnixFile() {
    close(fd_);
    // TODO(falsycat): error handling
  }

  UnixFile(const UnixFile&) = delete;
  UnixFile(UnixFile&&) = delete;

  UnixFile& operator=(const UnixFile&) = delete;
  UnixFile& operator=(UnixFile&&) = delete;


  size_t Read(uint8_t* buf, size_t size, size_t offset) override {
    if (size > SSIZE_MAX) size = SSIZE_MAX;

    const auto actual_offset = lseek(fd_, static_cast<off_t>(offset), SEEK_SET);
    if (actual_offset != static_cast<off_t>(offset)) return 0;

    const auto actual_read = read(fd_, buf, size);
    return actual_read > 0? static_cast<size_t>(actual_read): 0;
  }
  size_t Write(const uint8_t* buf, size_t size, size_t offset) override {
    if (size > SSIZE_MAX) size = SSIZE_MAX;

    const auto actual_offset = lseek(fd_, static_cast<off_t>(offset), SEEK_SET);
    if (actual_offset != static_cast<off_t>(offset)) return 0;

    const auto actual_write = write(fd_, buf, size);
    return actual_write > 0? static_cast<size_t>(actual_write): 0;
  }
  bool Truncate(size_t size) override {
    return ftruncate(fd_, static_cast<off_t>(size));
  }
  bool Flush() override {
    return fsync(fd_) == 0;
  }

  std::filesystem::file_time_type GetLastModified() const override {
    struct stat buf;
    if (fstat(fd_, &buf) < 0) return {};

    const auto& t = buf.st_mtim;
    return std::filesystem::file_time_type(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::seconds(t.tv_sec) +
            std::chrono::nanoseconds(t.tv_nsec)));
  }

 private:
  int fd_;
};


std::unique_ptr<iFile> iFile::CreateForNative(
    const std::filesystem::path& path) {
  const int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd < 0) return nullptr;
  return std::make_unique<UnixFile>(path, fd);
}

}  // namespace mnian::core
