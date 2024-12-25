#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define DECIMAL_LEN 16

char *get_decimal_number(int num) {
  if (num == 0) return "0";
  if (num == INT32_MIN) return "-2147483648";
  static char buffer[DECIMAL_LEN];
  // assert(buffer);
  int index = 0;
  if (num < 0) {
    buffer[index++] = '-';
    num = -num;
  }
  int left = index;
  while (num > 0) {
    buffer[index++] = (num % 10) + '0';
    num /= 10;
  }
  int right = index-1;
  char temp;
  while (left < right) {
    temp = buffer[left];
    buffer[left] = buffer[right];
    buffer[right] = temp;
    left++;
    right--;
  }
  buffer[index] = '\0';
  return buffer;
}

int vnprintf(size_t n, const char *fmt, va_list ap) {
  int len = 0;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case 'd':
          int num = va_arg(ap, int);
          char *decimal_str = get_decimal_number(num);
          while (*decimal_str != '\0') {
            putch(*decimal_str);
            decimal_str++;
            len++;
          }
          break;
        case 's':
          char *str = va_arg(ap, char*);
          while (*str != '\0') {
            putch(*str);
            str++;
            len++;
          }
          break;
        default:
          panic("Not implemented: printf()\n");
          break;
      }
    }
    else {
      putch(*fmt);
      len++;
    }
    fmt++;
  }
  return len;
}

int printf(const char *fmt, ...) {
  // panic("Not implemented");
  va_list ap;
  va_start(ap, fmt);
  int len = vnprintf(-1, fmt, ap);
  va_end(ap);
  return len;
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
        panic("Not implemented: sprintf()\n");
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
