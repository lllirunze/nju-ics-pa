#ifndef __FS_H__
#define __FS_H__

#include <common.h>

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;         // filename
  size_t size;        // file size
  size_t disk_offset; // offset of file in ramdisk
  ReadFn read;
  WriteFn write;
} Finfo;

const char* fs_name(int idx);

int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);

size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);

// int check_fd_idx(int idx);

#endif
