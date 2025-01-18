#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFFER_MAX_SIZE 8192

// flags
#define FLAGS_NONE      0b00000
#define FLAGS_LEFT      0b00001
#define FLAGS_NOTATION  0b00010
#define FLAGS_PREFIX    0b00100
#define FLAGS_ZERO      0b01000

// width
#define WIDTH_NONE 0

// number base
#define BASE_BIN 2
#define BASE_OCT 8
#define BASE_DEC 10
#define BASE_HEX 16

// format sequence
enum { SEQ_NONE, SEQ_FLAGS, SEQ_WIDTH, SEQ_PRECISION, SEQ_LENGTH, SEQ_SPECIFIER };

static char buffer[BUFFER_MAX_SIZE];
static int flags = FLAGS_NONE;
static int width = WIDTH_NONE;
static int format_sequence = SEQ_NONE;
static bool is_end = false;
static char number_chars[BASE_HEX] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void reset_format() {
  flags = FLAGS_NONE;
  width = WIDTH_NONE;
  format_sequence = SEQ_NONE;
  is_end = false;
  memset(buffer, 0, sizeof(buffer));
}

void set_format_param(int seq, int f, bool end) {
  if (seq < format_sequence) {
    panic("warning: unknown conversion type character\n");
  }
  format_sequence = seq;
  // todo: I don't implement the condition when flag is repeated.
  flags = flags | f;
  is_end = end;
}

char *num2str(int32_t num, int base) {
  buffer[BUFFER_MAX_SIZE-1] = '\0';
  int index = BUFFER_MAX_SIZE-2;
  bool is_negative = false;

  // if (base == BASE_DEC) num = (int32_t)num;

  if (num == 0) {
    buffer[index--] = '0';
  }
  else if (num == INT32_MIN) {
    // todo: INT32_MIN hasn't been padding yet.
    return "-2147483648";
  }
  else {
    if (num < 0) {
      is_negative = true;
      num = -num;
    }
    while (num > 0) {
      buffer[index--] = number_chars[num % base];
      num /= base;
    }
    if (is_negative) buffer[index--] = '-';
  }

  if (width != 0) {
    if ((flags & FLAGS_ZERO) == FLAGS_ZERO) {
      if (is_negative) index++;
      while (BUFFER_MAX_SIZE-2-index < width) buffer[index--] = '0';
      if (is_negative) buffer[index+1] = '-';
    }
    else {
      while (BUFFER_MAX_SIZE-2-index < width) buffer[index--] = ' ';
    }
  }

  return &buffer[index+1];
}

void sputch(char *out, char ch, int len) {
  if (out) out[len] = ch;
  else putch(ch);
}

int vnprintf(size_t n, const char *fmt, va_list ap) {
  return vsnprintf(NULL, n, fmt, ap);
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vnprintf(BUFFER_MAX_SIZE, fmt, ap);
  va_end(ap);
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, BUFFER_MAX_SIZE, fmt, ap);
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

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {

  int len = 0;
  while (*fmt != '\0') {
    if (*fmt == '%') {
      fmt++;

      reset_format();
      
      /**
       * todo: format: %[flags][width][.precision][length][specifier]
       * 
       * Issues:
       * I don't implement the condition when flag is repeated.
       * I don't implement the condition when flag is ignored.
       * I don't implement the condition when flag isn't used.
       * The max length of format (%s, %d, etc.) is BUFFER_MAX_SIZE because of `static char buffer[BUFFER_MAX_SIZE+1];`
       * 
       */

      while (*fmt != '\0') {
        switch(*fmt) {
          // flags
          case '0':
            set_format_param(SEQ_FLAGS, FLAGS_ZERO, false);
            break;

          // width
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            set_format_param(SEQ_WIDTH, FLAGS_NONE, false);
            while (*fmt >= '0' && *fmt <= '9') {
              width = width * 10 + (*fmt - '0');
              fmt++;
            }
            fmt--;
            // todo: The max length of format (%s, %d, etc.) is BUFFER_MAX_SIZE because of `static char buffer[BUFFER_MAX_SIZE+1];`
            assert(width >= 0 && width < BUFFER_MAX_SIZE);
            break;

          // .precision

          // length

          // specifier
          case 'd':
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            int dec_num = va_arg(ap, int);
            char *dec_str = num2str(dec_num, BASE_DEC);
            while (*dec_str != '\0') {
              sputch(out, *dec_str, len++);
              dec_str++;
              if (len == n-1) {out[len] = '\0'; return len;}
            }
            break;
          case 'x':
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            int hex_num = va_arg(ap, int);
            char *hex_str = num2str(hex_num, BASE_HEX);
            while (*hex_str != '\0') {
              sputch(out, *hex_str, len++);
              hex_str++;
              if (len == n-1) {out[len] = '\0'; return len;}
            }
            break;
          case 'p':
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            void *ptr = va_arg(ap, void*);
            uintptr_t addr = (uintptr_t)ptr;
            char *addr_str = num2str(addr, BASE_HEX);
            sputch(out, '0', len++);
            if (len == n-1) {out[len] = '\0'; return len;}
            sputch(out, 'x', len++);
            if (len == n-1) {out[len] = '\0'; return len;}
            while (*addr_str != '\0') {
              sputch(out, *addr_str, len++);
              addr_str++;
              if (len == n-1) {out[len] = '\0'; return len;}
            }
            break;
          case 'c':
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            char ch = (char)va_arg(ap, int);
            // todo: "%[flags][width]c" is not implemented.
            sputch(out, ch, len++);
            if (len == n-1) {out[len] = '\0'; return len;}
            break;
          case 's':
            // todo: "%[flags][width]s" is not implemented.
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            char *str = va_arg(ap, char*);
            while (*str != '\0') {
              sputch(out, *str, len++);
              str++;
              if (len == n-1) {out[len] = '\0'; return len;}
            }
            break;
          
          // todo: unfinished format
          case '-':
          case '+':
          case '#':
          case '.':
          case 'h':
          case 'l':
          case 'L':
          case 'a':
          case 'A':
          case 'o':
          case 'X':
          case 'u':
          case 'f':
          case 'e':
          case 'E':
          case 'g':
          case 'G':

          default:
            panic("Not implemented: vsnprintf() format - %%[flags][width][.precision][length][specifier]\n");
            break;
        }
        if (is_end == true) break;
        else fmt++;
      }
    }
    else {
      sputch(out, *fmt, len++);
      if (len == n-1) {out[len] = '\0'; return len;}
    }
    fmt++;
  }
  if (out) out[len] = '\0';
  return len;
  
}

#endif
