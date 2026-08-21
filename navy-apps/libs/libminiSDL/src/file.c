#include <sdl-file.h>
#include <stdlib.h>
#include <string.h>

static int64_t file_size(SDL_RWops *rw) {
  long position = ftell(rw->fp);
  if (position < 0 || fseek(rw->fp, 0, SEEK_END) != 0) return -1;
  long size = ftell(rw->fp);
  fseek(rw->fp, position, SEEK_SET);
  return size;
}

static int64_t file_seek(SDL_RWops *rw, int64_t offset, int whence) {
  if (fseek(rw->fp, offset, whence) != 0) return -1;
  return ftell(rw->fp);
}

static size_t file_read(SDL_RWops *rw, void *buf, size_t size, size_t nmemb) {
  return fread(buf, size, nmemb, rw->fp);
}

static size_t file_write(SDL_RWops *rw, const void *buf, size_t size, size_t nmemb) {
  return fwrite(buf, size, nmemb, rw->fp);
}

static int file_close(SDL_RWops *rw) {
  int ret = fclose(rw->fp);
  free(rw);
  return ret;
}

static int64_t mem_size(SDL_RWops *rw) {
  return rw->mem.size;
}

static int64_t mem_seek(SDL_RWops *rw, int64_t offset, int whence) {
  int64_t base;
  switch (whence) {
    case SEEK_SET: base = 0; break;
    case SEEK_CUR: base = rw->mem.position; break;
    case SEEK_END: base = rw->mem.size; break;
    default: return -1;
  }
  int64_t position = base + offset;
  if (position < 0 || position > rw->mem.size) return -1;
  rw->mem.position = position;
  return position;
}

static size_t mem_read(SDL_RWops *rw, void *buf, size_t size, size_t nmemb) {
  if (size == 0) return 0;
  size_t available = rw->mem.size - rw->mem.position;
  size_t count = nmemb < available / size ? nmemb : available / size;
  memcpy(buf, (uint8_t *)rw->mem.base + rw->mem.position, count * size);
  rw->mem.position += count * size;
  return count;
}

static size_t mem_write(SDL_RWops *rw, const void *buf, size_t size, size_t nmemb) {
  if (size == 0) return 0;
  size_t available = rw->mem.size - rw->mem.position;
  size_t count = nmemb < available / size ? nmemb : available / size;
  memcpy((uint8_t *)rw->mem.base + rw->mem.position, buf, count * size);
  rw->mem.position += count * size;
  return count;
}

static int mem_close(SDL_RWops *rw) {
  free(rw);
  return 0;
}

SDL_RWops* SDL_RWFromFile(const char *filename, const char *mode) {
  FILE *fp = fopen(filename, mode);
  if (fp == NULL) return NULL;

  SDL_RWops *rw = malloc(sizeof(*rw));
  if (rw == NULL) {
    fclose(fp);
    return NULL;
  }
  rw->size = file_size;
  rw->seek = file_seek;
  rw->read = file_read;
  rw->write = file_write;
  rw->close = file_close;
  rw->type = RW_TYPE_FILE;
  rw->fp = fp;
  return rw;
}

SDL_RWops* SDL_RWFromMem(void *mem, int size) {
  if (mem == NULL || size < 0) return NULL;
  SDL_RWops *rw = malloc(sizeof(*rw));
  if (rw == NULL) return NULL;
  rw->size = mem_size;
  rw->seek = mem_seek;
  rw->read = mem_read;
  rw->write = mem_write;
  rw->close = mem_close;
  rw->type = RW_TYPE_MEM;
  rw->fp = NULL;
  rw->mem.base = mem;
  rw->mem.size = size;
  rw->mem.position = 0;
  return rw;
}
