#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  // panic("Not implemented");
  int i = 0;
  va_list args;
  va_start(args, fmt);
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      if (*fmt == 'd') {
        int num = va_arg(args, int);
        if (num == 0) {
          out[i++] = '0';
        }
        else {
          if (num < 0) {
            out[i++] = '-';
            num = -num;
          }
          int left = i;
          while (num > 0) {
            out[i++] = (num % 10) + '0';
            num /= 10;
          }
          int right = i-1;
          char temp;
          while (left < right) {
            temp = out[left];
            out[left] = out[right];
            out[right] = temp;
            left++;
            right--;
          }
        }
      }
      else if (*fmt == 's') {
        char *str = va_arg(args, char*);
        while (*str != '\0') {
          out[i++] = *str;
          str++;
        }
      }
      else {
        panic("Not implemented, we only have \%d and \%s.");
      }
    }
    else out[i++] = *fmt;
    fmt++;
  }
  out[i] = '\0';
  va_end(args);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
