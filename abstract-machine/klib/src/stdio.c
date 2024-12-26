#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFFER_MAX_SIZE 128

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

char *num2str(int num, int base) {
  buffer[BUFFER_MAX_SIZE-1] = '\0';
  int index = BUFFER_MAX_SIZE-2;

  if (num == 0) {
    buffer[index--] = '0';
  }
  else if (num == INT32_MIN) {
    // todo: INT32_MIN hasn't been padding yet.
    return "-2147483648";
  }
  else {
    bool is_negative = false;
    if (num < 0) {
      is_negative = true;
      num = -num;
    }
    while (num > 0) {
      // buffer[index--] = (num % base) + '0';
      buffer[index--] = number_chars[num % base];
      num /= base;
    }
    if (is_negative) buffer[index--] = '-';
  }

  // todo: padding
  if (width != 0) {
    if ((flags & FLAGS_ZERO) == FLAGS_ZERO) {
      while (BUFFER_MAX_SIZE-2-index < width) buffer[index--] = '0';
    }
    else {
      while (BUFFER_MAX_SIZE-2-index < width) buffer[index--] = ' ';
    }
  }

  return &buffer[index+1];
}

int vnprintf(size_t n, const char *fmt, va_list ap) {
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
      is_end = false;
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
              putch(*dec_str);
              dec_str++;
              len++;
            }
            break;
          case 'x':
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            int hex_num = va_arg(ap, int);
            
            // panic("Not implemented: printf() format - %%[flags][width]x\n");

            char *hex_str = num2str(hex_num, BASE_HEX);
            while (*hex_str != '\0') {
              putch(*hex_str);
              hex_str++;
              len++;
            }

            break;
          case 'c':
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            char ch = (char)va_arg(ap, int);
            // todo: "%[flags][width]c" is not implemented.
            putch(ch);
            len++;
            break;
          case 's':
            // todo: "%[flags][width]s" is not implemented.
            set_format_param(SEQ_SPECIFIER, FLAGS_NONE, true);
            char *str = va_arg(ap, char*);
            while (*str != '\0') {
              putch(*str);
              str++;
              len++;
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
          case 'p':

          default:
            panic("Not implemented: printf() format - %%[flags][width][.precision][length][specifier]\n");
            break;
        }
        if (is_end == true) break;
        else fmt++;
      }

      
      // switch (*fmt) {
      //   case 'd':
      //     int num = va_arg(ap, int);
      //     char *decimal_str = get_decimal_number(num);
      //     while (*decimal_str != '\0') {
      //       putch(*decimal_str);
      //       decimal_str++;
      //       len++;
      //     }
      //     break;
      //   case 's':
      //     char *str = va_arg(ap, char*);
      //     while (*str != '\0') {
      //       putch(*str);
      //       str++;
      //       len++;
      //     }
      //     break;
      //   case 'c':
      //     char ch = (char)va_arg(ap, int);
      //     putch(ch);
      //     len++;
      //     break;
      //   default:
      //     panic("Not implemented: printf()\n");
      //     break;
      // }
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
