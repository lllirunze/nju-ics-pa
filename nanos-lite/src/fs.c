#include <fs.h>
#include <device.h>
#include <ramdisk.h>

static int ft_size;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO};

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
  [FD_STDIN]    = {"stdin",           0, 0, 0, invalid_read,  invalid_write},
  [FD_STDOUT]   = {"stdout",          0, 0, 0, invalid_read,  serial_write},
  [FD_STDERR]   = {"stderr",          0, 0, 0, invalid_read,  serial_write},
  [FD_FB]       = {"/dev/fb",         0, 0, 0, invalid_read,  fb_write},
  [FD_EVENTS]   = {"/dev/events",     0, 0, 0, events_read,   invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo",  0, 0, 0, dispinfo_read, invalid_write}, 
#include "files.h"

};

const char* fs_name(int idx) {
  assert(idx >= 0 && idx < ft_size);
  return file_table[idx].name;
}


void init_fs() {
  // TODO: initialize the size of /dev/fb
  ft_size = sizeof(file_table)/sizeof(file_table[0]);
  AM_GPU_CONFIG_T gc = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = gc.width * gc.height * sizeof(uint32_t);
  Log("Initializing file system: %d items", ft_size);
}

int fs_open(const char *pathname, int flags, int mode) {

  int i=0;
  for (; i<ft_size; i++) {
    if (file_table[i].name != NULL && strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].fseek_offset = 0;
      return i;
    }
  }
  
  return -1;
}

int fs_close(int fd) {
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {

  if (fd < 0 || fd >= ft_size) return -1;
  if (file_table[fd].read != NULL) return file_table[fd].read(buf, 0, len);
  if (file_table[fd].size < 0) return -1;
  if (len < 0) return -1;

  if (file_table[fd].fseek_offset + len > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].fseek_offset;
  }
  ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].fseek_offset, len);
  file_table[fd].fseek_offset += len;

  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {

  if (fd < 0 || fd >= ft_size) return -1;
  if (file_table[fd].write != NULL) return file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].fseek_offset, len);
  if (file_table[fd].size < 0) return -1;
  if (len < 0) return -1;
  
  if (file_table[fd].fseek_offset + len > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].fseek_offset;
  }
  ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].fseek_offset, len);
  file_table[fd].fseek_offset += len;

  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {

  if (fd < 0 || fd >= ft_size) return -1;

  switch (whence) {
    case SEEK_SET:
      if (offset >= 0 && offset <= file_table[fd].size) {
        file_table[fd].fseek_offset = offset;
        return file_table[fd].fseek_offset;
      } 
      break;
    case SEEK_CUR:
      if (file_table[fd].fseek_offset + offset >= 0 && 
          file_table[fd].fseek_offset + offset <= file_table[fd].size) {
        file_table[fd].fseek_offset += offset;
        return file_table[fd].fseek_offset;
      }
      break;
    case SEEK_END:
      if (file_table[fd].size + offset >= 0 &&
          file_table[fd].size + offset <= file_table[fd].size) {
        file_table[fd].fseek_offset = file_table[fd].size + offset;
        return file_table[fd].fseek_offset;
      }
      break;
    default:
      panic("not implemented.");
      break;
  }
  return -1;
}
