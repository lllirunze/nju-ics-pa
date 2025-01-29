#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len] != '\0') len++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  while (src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t p = 0;
  while(src[p] != '\0' && p < n) {
    dst[p] = src[p];
    p++;
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t i = strlen(dst), j = 0;
  while (src[j] != '\0') {
    dst[i] = src[j];
    i++;
    j++;
  }
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;
  while (s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] == s2[i]) i++;
    else return s1[i] - s2[i];
  }
  if (s1[i] == '\0' && s2[i] == '\0') return 0;
  else if (s1[i] == '\0') return -1;
  else return 1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  while (s1[i] != '\0' && s2[i] != '\0' && i < n) {
    if (s1[i] == s2[i]) i++;
    else return s1[i] - s2[i];
  }
  if (i == n) return 0;
  if (s1[i] == '\0' && s2[i] == '\0') return 0;
  else if (s1[i] == '\0') return -1;
  else return 1;
}

void *memset(void *s, int c, size_t n) {
  size_t i = 0;
  for (i = 0; i < n && *((char *)s + i) != '\0'; i++) {
    *((char *)s + i) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if(dst < src || dst >= src + n) memcpy(dst, src, n);
  else {
    while(n--){
      *((char*)dst + n) = *((char*)src + n);
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t p = 0;
  while(p < n){
    *((char*)out + p) = *((char*)in + p);
    p++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i = 0;
  while (*((char *)s1 + i) != '\0' && *((char *)s2 + i) != '\0' && i < n) {
    if (*((char *)s1 + i) == *((char *)s2 + i)) i++;
    else return *((char *)s1 + i) - *((char *)s2 + i);
  }
  if (i == n) return 0;
  if (*((char *)s1 + i) == '\0' && *((char *)s2 + i) == '\0') return 0;
  else if (*((char *)s1 + i) == '\0') return -1;
  else return 1;
}

#endif
