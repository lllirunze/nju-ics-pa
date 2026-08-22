#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  if (len < 0) return;
  if (len >= 256) len = 255;
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  char command[256];
  size_t len = strnlen(cmd, sizeof(command) - 1);
  memcpy(command, cmd, len);
  command[len] = '\0';

  char *argv[16];
  int argc = 0;
  char *p = command;
  while (*p != '\0' && argc < (int)(sizeof(argv) / sizeof(argv[0])) - 1) {
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') break;
    argv[argc++] = p;
    while (*p != '\0' && *p != ' ' && *p != '\t') p++;
    if (*p != '\0') *p++ = '\0';
  }
  argv[argc] = NULL;
  if (argc == 0) return;

  if (strcmp(argv[0], "echo") == 0) {
    const char *args = cmd + strcspn(cmd, " \t\r\n");
    while (*args == ' ' || *args == '\t') args++;
    sh_printf("%s\n", args);
    return;
  }

  setenv("PATH", "/bin:/usr/bin", 1);
  if (execvp(argv[0], argv) < 0) {
    sh_printf("sh: %s: command failed\n", argv[0]);
  }
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
