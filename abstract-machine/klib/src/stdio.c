#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  for (int i = 0; i < len && i < (int)sizeof(buf) - 1; i ++) putch(buf[i]);
  return len;
}

static void append_char(char *out, size_t n, size_t *len, char c) {
  if (n > 0 && *len < n - 1) out[*len] = c;
  (*len) ++;
}

static void append_unsigned(char *out, size_t n, size_t *len, unsigned int value,
                            unsigned int base, bool uppercase) {
  char digits[] = "0123456789abcdef";
  if (uppercase) strcpy(digits, "0123456789ABCDEF");
  char buf[sizeof(value) * 8];
  int count = 0;
  do {
    buf[count ++] = digits[value % base];
    value /= base;
  } while (value != 0);
  while (count > 0) append_char(out, n, len, buf[-- count]);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t len = 0;
  for (; *fmt != '\0'; fmt ++) {
    if (*fmt != '%') {
      append_char(out, n, &len, *fmt);
      continue;
    }

    fmt ++;
    switch (*fmt) {
      case '%': append_char(out, n, &len, '%'); break;
      case 'c': append_char(out, n, &len, (char)va_arg(ap, int)); break;
      case 's': {
        const char *s = va_arg(ap, const char *);
        if (s == NULL) s = "(null)";
        while (*s != '\0') append_char(out, n, &len, *s ++);
        break;
      }
      case 'd': {
        int value = va_arg(ap, int);
        unsigned int magnitude = value < 0 ? 0u - (unsigned int)value : (unsigned int)value;
        if (value < 0) append_char(out, n, &len, '-');
        append_unsigned(out, n, &len, magnitude, 10, false);
        break;
      }
      case 'u': append_unsigned(out, n, &len, va_arg(ap, unsigned int), 10, false); break;
      case 'x': append_unsigned(out, n, &len, va_arg(ap, unsigned int), 16, false); break;
      case 'X': append_unsigned(out, n, &len, va_arg(ap, unsigned int), 16, true); break;
      case '\0': fmt --; break;
      default:
        append_char(out, n, &len, '%');
        append_char(out, n, &len, *fmt);
    }
  }
  if (n > 0) out[len < n ? len : n - 1] = '\0';
  return (int)len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, (size_t)-1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsprintf(out, fmt, ap);
  va_end(ap);
  return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return len;
}

#endif
