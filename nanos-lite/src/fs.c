#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

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
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write, 0},
  {"/dev/events", 0, 0, events_read, invalid_write, 0},
  {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write, 0},
#include "files.h"
};

void init_fs() {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = cfg.width * cfg.height * sizeof(uint32_t);
}

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags;
  (void)mode;

  for (size_t i = 0; i < LENGTH(file_table); i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("file not found: %s", pathname);
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *f = &file_table[fd];

  if (f->read != NULL) {
    return f->read(buf, f->open_offset, len);
  }

  len = len < f->size - f->open_offset ? len : f->size - f->open_offset;
  size_t nread = ramdisk_read(buf, f->disk_offset + f->open_offset, len);
  f->open_offset += nread;
  return nread;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *f = &file_table[fd];
  size_t nwrite;

  if (f->write != NULL) {
    nwrite = f->write(buf, f->open_offset, len);
  } else {
    len = len < f->size - f->open_offset ? len : f->size - f->open_offset;
    nwrite = ramdisk_write(buf, f->disk_offset + f->open_offset, len);
  }
  f->open_offset += nwrite;
  return nwrite;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *f = &file_table[fd];
  intptr_t base;

  switch (whence) {
    case SEEK_SET: base = 0; break;
    case SEEK_CUR: base = f->open_offset; break;
    case SEEK_END: base = f->size; break;
    default: panic("invalid whence = %d", whence);
  }

  intptr_t new_offset = base + (intptr_t)offset;
  assert(new_offset >= 0 && (size_t)new_offset <= f->size);
  f->open_offset = new_offset;
  return f->open_offset;
}

int fs_close(int fd) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  return 0;
}
