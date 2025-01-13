#include <fs.h>
#include <device.h>
#include <ramdisk.h>

static int ft_size;
static size_t fs_offset;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin",  0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
  
  /**
   * 1. The sentence `#include "files.h"` is used to add other files into `file_table`.
   *    You don't need to do anything else to add the contents of these files.
   * 2. We only consider write of STDOUT and STDERR. 
   *    (using putch() to output it to serial port)
   * 3. Other operations are ignored.
   */

};

const char* fs_name(int idx) {
  assert(idx >= 0 && idx < ft_size);
  return file_table[idx].name;
}


void init_fs() {
  // TODO: initialize the size of /dev/fb
  ft_size = sizeof(file_table)/sizeof(file_table[0]);
  Log("Initializing file system: %d items", ft_size);
}

int fs_open(const char *pathname, int flags, int mode) {
  /**
   * 1. If fs_open doesn't find `pathname`, nanos will end by assertion.
   * 2. To simplify, we allow all user program can read/write all existing file.
   *    So we can ignore `flags` and `mode` in fs_open.
   */ 
  int i=0;
  for (; i<ft_size; i++) {
    if (file_table[i].name != NULL && strcmp(file_table[i].name, pathname) == 0) {
      fs_offset = file_table[i].disk_offset;
      return i;
    }
  }
  return -1;
}

int fs_close(int fd) {
  /**
   * 1. fs_close() directly return 0, 
   *    because simple file system doesn't maintain the status of opening files.
   */
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {
  /** 
   * 1. Use ramdisk_read() and ramdisk_write() to read/write files.
   * 2. Because the size of all files is fixed, offset cannot exceed the edge of file.
   */
  if (fd < 0 || fd >= ft_size) return -1;
  if (file_table[fd].size < 0) return -1;
  if (len < 0) return -1;

  if (fs_offset + len > file_table[fd].disk_offset + file_table[fd].size) {
    len = file_table[fd].disk_offset + file_table[fd].size - fs_offset;
  }
  ramdisk_read(buf, fs_offset, len);
  fs_offset += len;

  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  /** 
   * 1. Use ramdisk_read() and ramdisk_write() to read/write files.
   * 2. Because the size of all files is fixed, offset cannot exceed the edge of file.
   */
  if (fd < 0 || fd >= ft_size) return -1;
  if (file_table[fd].write != NULL) return file_table[fd].write(buf, 0, len);
  if (file_table[fd].size < 0) return -1;
  if (len < 0) return -1;

  if (fs_offset + len > file_table[fd].disk_offset + file_table[fd].size) {
    len = file_table[fd].disk_offset + file_table[fd].size - fs_offset;
  }
  ramdisk_write(buf, fs_offset, len);
  fs_offset += len;

  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  /**
   * 1. Because the size of all files is fixed, offset cannot exceed the edge of file.
   */
  if (fd < 0 || fd >= ft_size) return -1;

  switch (whence) {
    case SEEK_SET:
      if (offset >= 0 && offset <= file_table[fd].size) {
        fs_offset = file_table[fd].disk_offset + offset;
        return fs_offset - file_table[fd].disk_offset;
      } 
      break;
    case SEEK_CUR:
      if (fs_offset - file_table[fd].disk_offset + offset >= 0 && 
          fs_offset - file_table[fd].disk_offset + offset <= file_table[fd].size) {
        fs_offset = fs_offset + offset;
        return fs_offset - file_table[fd].disk_offset;
      }
      break;
    case SEEK_END:
      if (file_table[fd].size + offset >= 0 &&
          file_table[fd].size + offset <= file_table[fd].size) {
        fs_offset = file_table[fd].disk_offset + file_table[fd].size + offset;
        return fs_offset - file_table[fd].disk_offset;
      }
      break;
    default:
      panic("not implemented.");
      break;
  }
  return -1;
}

// int check_fd_idx(int idx) {
//   assert(idx >= 0 && idx < ft_size);
//   return idx;
// }

