/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <memory/paddr.h>

enum {
  reg_blkcnt,
  reg_status,
  reg_buf,
  reg_blkno,
  reg_nblks,
  reg_cmd,
  nr_reg,
};

#define BLKSZ 512

static uint32_t *disk_base = NULL;
static FILE *disk_fp = NULL;

static void disk_do_io(void) {
  uint32_t blkno = disk_base[reg_blkno];
  uint32_t nblks = disk_base[reg_nblks];
  uint32_t guest_buf = disk_base[reg_buf];
  size_t nbytes = (size_t)nblks * BLKSZ;

  Assert(nblks > 0 && blkno <= disk_base[reg_blkcnt] &&
      nblks <= disk_base[reg_blkcnt] - blkno,
      "invalid disk request: block = %u, count = %u, disk blocks = %u",
      blkno, nblks, disk_base[reg_blkcnt]);
  Assert(guest_buf >= CONFIG_MBASE && nbytes <= CONFIG_MSIZE - (guest_buf - CONFIG_MBASE),
      "disk buffer outside guest memory: " FMT_PADDR ", size = %zu", guest_buf, nbytes);

  uint8_t *buf = guest_to_host(guest_buf);
  Assert(fseek(disk_fp, (long)blkno * BLKSZ, SEEK_SET) == 0, "disk seek failed");
  if (disk_base[reg_cmd] != 0) {
    Assert(fwrite(buf, 1, nbytes, disk_fp) == nbytes, "disk write failed");
    Assert(fflush(disk_fp) == 0, "disk flush failed");
  } else {
    size_t nread = fread(buf, 1, nbytes, disk_fp);
    Assert(nread == nbytes, "disk image is shorter than its advertised size");
  }
}

static void disk_io_handler(uint32_t offset, int len, bool is_write) {
  if (!is_write || offset != reg_cmd * sizeof(uint32_t) || len != sizeof(uint32_t)) return;
  disk_do_io();
  disk_base[reg_status] = 1;
}

void init_disk() {
  const char *path = CONFIG_DISK_IMG_PATH;
  char default_path[512];
  if (path[0] == '\0') {
    const char *navy_home = getenv("NAVY_HOME");
    if (navy_home != NULL) {
      snprintf(default_path, sizeof(default_path), "%s/build/ramdisk.img", navy_home);
      path = default_path;
    }
  }

  disk_base = (uint32_t *)new_space(sizeof(uint32_t) * nr_reg);
  memset(disk_base, 0, sizeof(uint32_t) * nr_reg);
  if (path[0] != '\0') disk_fp = fopen(path, "r+b");
  Assert(disk_fp != NULL, "cannot open disk image '%s'", path);

  Assert(fseek(disk_fp, 0, SEEK_END) == 0, "cannot seek disk image");
  long size = ftell(disk_fp);
  Assert(size > 0, "disk image is empty");
  Assert(size % BLKSZ == 0, "disk image size (%ld) is not a multiple of %d", size, BLKSZ);
  rewind(disk_fp);
  disk_base[reg_blkcnt] = size / BLKSZ;
  disk_base[reg_status] = 1;

#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("disk", CONFIG_DISK_CTL_PORT, disk_base, sizeof(uint32_t) * nr_reg, disk_io_handler);
#else
  add_mmio_map("disk", CONFIG_DISK_CTL_MMIO, disk_base, sizeof(uint32_t) * nr_reg, disk_io_handler);
#endif
}
