#include <am.h>
#include <klib.h>
#include <nemu.h>

#define DISK_CONFIG_ADDR (DISK_ADDR + 0x00)
#define DISK_STATUS_ADDR (DISK_ADDR + 0x04)
#define DISK_BUF_ADDR    (DISK_ADDR + 0x08)
#define DISK_BLKNO_ADDR  (DISK_ADDR + 0x0c)
#define DISK_BLKCNT_ADDR (DISK_ADDR + 0x10)
#define DISK_CMD_ADDR    (DISK_ADDR + 0x14)

#define DISK_BLKSZ 512

void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->blkcnt = inl(DISK_CONFIG_ADDR);
  cfg->blksz = DISK_BLKSZ;
  cfg->present = cfg->blkcnt != 0;
}

void __am_disk_status(AM_DISK_STATUS_T *stat) {
  stat->ready = inl(DISK_STATUS_ADDR) != 0;
}

void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
  assert(io->buf != NULL && io->blkno >= 0 && io->blkcnt > 0);
  outl(DISK_BUF_ADDR, (uintptr_t)io->buf);
  outl(DISK_BLKNO_ADDR, io->blkno);
  outl(DISK_BLKCNT_ADDR, io->blkcnt);
  outl(DISK_CMD_ADDR, io->write ? 1 : 0);
  while (inl(DISK_STATUS_ADDR) == 0) {
  }
}
