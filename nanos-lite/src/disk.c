#include <common.h>

static AM_DISK_CONFIG_T disk_config;

void init_disk(void) {
  disk_config = io_read(AM_DISK_CONFIG);
  if (!disk_config.present) panic("disk device is not present");
  if (disk_config.blksz <= 0 || disk_config.blkcnt <= 0) {
    panic("invalid disk geometry: block size = %d, block count = %d",
        disk_config.blksz, disk_config.blkcnt);
  }
  Log("disk info: block size = %d, block count = %d, size = %d bytes",
      disk_config.blksz, disk_config.blkcnt, disk_config.blksz * disk_config.blkcnt);
}

static void check_range(size_t offset, size_t len) {
  size_t size = (size_t)disk_config.blksz * disk_config.blkcnt;
  if (offset > size || len > size - offset) {
    panic("disk access out of range: offset = %d, length = %d, size = %d",
        offset, len, size);
  }
}

size_t disk_read(void *buf, size_t offset, size_t len) {
  check_range(offset, len);
  uint8_t *dst = buf;
  uint8_t block[disk_config.blksz];

  while (len > 0) {
    size_t block_offset = offset % disk_config.blksz;
    size_t nread = len < disk_config.blksz - block_offset ? len : disk_config.blksz - block_offset;
    io_write(AM_DISK_BLKIO, .write = false, .buf = block,
        .blkno = offset / disk_config.blksz, .blkcnt = 1);
    memcpy(dst, block + block_offset, nread);
    dst += nread;
    offset += nread;
    len -= nread;
  }
  return (size_t)(dst - (uint8_t *)buf);
}

size_t disk_write(const void *buf, size_t offset, size_t len) {
  check_range(offset, len);
  const uint8_t *src = buf;
  uint8_t block[disk_config.blksz];

  while (len > 0) {
    size_t block_offset = offset % disk_config.blksz;
    size_t nwrite = len < disk_config.blksz - block_offset ? len : disk_config.blksz - block_offset;
    int blkno = offset / disk_config.blksz;
    io_write(AM_DISK_BLKIO, .write = false, .buf = block, .blkno = blkno, .blkcnt = 1);
    memcpy(block + block_offset, src, nwrite);
    io_write(AM_DISK_BLKIO, .write = true, .buf = block, .blkno = blkno, .blkcnt = 1);
    src += nwrite;
    offset += nwrite;
    len -= nwrite;
  }
  return (size_t)(src - (const uint8_t *)buf);
}
