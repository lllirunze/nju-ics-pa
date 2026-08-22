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
***************************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <cpu/difftest.h>
#include <memory/paddr.h>
#include <utils.h>
#include "sdb.h"

#define SNAPSHOT_MAGIC "NEMU-SNAPSHOT"
#define SNAPSHOT_VERSION 1

typedef struct {
  char magic[sizeof(SNAPSHOT_MAGIC)];
  uint32_t version;
  uint32_t cpu_state_size;
  uint64_t pmem_size;
} SnapshotHeader;

static bool write_all(FILE *fp, const void *buf, size_t size) {
  return fwrite(buf, 1, size, fp) == size;
}

static bool read_all(FILE *fp, void *buf, size_t size) {
  return fread(buf, 1, size, fp) == size;
}

bool snapshot_save(const char *path) {
  SnapshotHeader header = {
    .magic = SNAPSHOT_MAGIC,
    .version = SNAPSHOT_VERSION,
    .cpu_state_size = sizeof(CPU_state),
    .pmem_size = CONFIG_MSIZE,
  };
  FILE *fp = fopen(path, "wb");

  if (fp == NULL) {
    printf("Cannot save snapshot '%s': %s\n", path, strerror(errno));
    return false;
  }

  bool success = write_all(fp, &header, sizeof(header)) &&
                 write_all(fp, &cpu, sizeof(cpu)) &&
                 write_all(fp, guest_to_host(PMEM_LEFT), CONFIG_MSIZE);
  if (fclose(fp) != 0) {
    success = false;
  }

  if (!success) {
    printf("Cannot save snapshot '%s': write failed\n", path);
  }
  return success;
}

bool snapshot_load(const char *path) {
  FILE *fp = fopen(path, "rb");
  SnapshotHeader header;
  CPU_state saved_cpu;
  uint64_t expected_size = sizeof(header) + sizeof(saved_cpu) + (uint64_t)CONFIG_MSIZE;

  if (fp == NULL) {
    printf("Cannot load snapshot '%s': %s\n", path, strerror(errno));
    return false;
  }

  if (fseek(fp, 0, SEEK_END) != 0 || ftell(fp) != (long)expected_size ||
      fseek(fp, 0, SEEK_SET) != 0 || !read_all(fp, &header, sizeof(header)) ||
      memcmp(header.magic, SNAPSHOT_MAGIC, sizeof(header.magic)) != 0 ||
      header.version != SNAPSHOT_VERSION ||
      header.cpu_state_size != sizeof(CPU_state) || header.pmem_size != CONFIG_MSIZE) {
    fclose(fp);
    printf("Cannot load snapshot '%s': incompatible or truncated snapshot\n", path);
    return false;
  }

  bool success = read_all(fp, &saved_cpu, sizeof(saved_cpu)) &&
                 read_all(fp, guest_to_host(PMEM_LEFT), CONFIG_MSIZE);
  fclose(fp);
  if (!success) {
    printf("Cannot load snapshot '%s': read failed\n", path);
    return false;
  }

  cpu = saved_cpu;
  nemu_state.state = NEMU_STOP;
  nemu_state.halt_pc = cpu.pc;
  nemu_state.halt_ret = 0;
  IFDEF(CONFIG_DIFFTEST, if (difftest_is_attached()) difftest_attach());
  return true;
}
