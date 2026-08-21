#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len] != '\0') len ++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *ret = dst;
  while ((*dst ++ = *src ++) != '\0') {}
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *ret = dst;
  size_t i = 0;
  for (; i < n && src[i] != '\0'; i ++) dst[i] = src[i];
  for (; i < n; i ++) dst[i] = '\0';
  return ret;
}

char *strcat(char *dst, const char *src) {
  char *ret = dst;
  while (*dst != '\0') dst ++;
  while ((*dst ++ = *src ++) != '\0') {}
  return ret;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 == *s2 && *s1 != '\0') { s1 ++; s2 ++; }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; i ++) {
    if (s1[i] != s2[i]) return (unsigned char)s1[i] - (unsigned char)s2[i];
    if (s1[i] == '\0') return 0;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  for (size_t i = 0; i < n; i ++) p[i] = (unsigned char)c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = dst;
  const unsigned char *s = src;
  if (d < s) {
    for (size_t i = 0; i < n; i ++) d[i] = s[i];
  } else if (d > s) {
    for (size_t i = n; i > 0; i --) d[i - 1] = s[i - 1];
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *d = out;
  const unsigned char *s = in;
  for (size_t i = 0; i < n; i ++) d[i] = s[i];
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *a = s1;
  const unsigned char *b = s2;
  for (size_t i = 0; i < n; i ++) {
    if (a[i] != b[i]) return a[i] - b[i];
  }
  return 0;
}

#endif
