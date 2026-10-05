#include "tui.h"
#include "core/time_utils.h"

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define ESC "\x1b"

App app;
static struct termios start;
static char *output_buf;
static size_t output_capacity;

static Grid front_buf = {.elem_size = sizeof(const char *)};
Grid back_buf = {.elem_size = sizeof(const char *)};

static volatile sig_atomic_t stop_requested = 0;

static void request_stop(int signal_number) {
  (void)signal_number;
  stop_requested = 1;
}

static int resize_buffers(int width, int height) {
  if (!grid_resize(&front_buf, width, height) ||
      !grid_resize(&back_buf, width, height)) {
    perror("resize terminal buffers");
    return 0;
  }

  const char **front = front_buf.data;
  const char **back = back_buf.data;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      front[y * width + x] = " ";
      back[y * width + x] = " ";
    }
  }
  return 1;
}

int init_terminal(void) {

  if (tcgetattr(0, &start) == -1) {
    perror("tcgetattr");
    return 0;
  }

  struct winsize size = {0};
  if (ioctl(0, TIOCGWINSZ, &size) == -1) {
    perror("TIOCGWINSZ");
    return 0;
  }
  if (size.ws_col == 0 || size.ws_row == 0) {
    fprintf(stderr, "Terminal dimensions must be nonzero.\n");
    return 0;
  }

  struct sigaction action = {0};
  action.sa_handler = request_stop;
  sigemptyset(&action.sa_mask);
  if (sigaction(SIGINT, &action, NULL) == -1) {
    perror("sigaction");
    return 0;
  }

  if (!resize_buffers(size.ws_col, size.ws_row)) {
    grid_free(&front_buf);
    grid_free(&back_buf);
    return 0;
  }

  struct termios tty = start;

  tty.c_lflag &= ~(ICANON | ECHO);

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  if (tcsetattr(0, TCSAFLUSH, &tty) == -1) {
    perror("tcsetattr");
    tcsetattr(0, TCSAFLUSH, &start);
    grid_free(&front_buf);
    grid_free(&back_buf);
    return 0;
  }

  app = (App){.is_running = 1,
              .viewport = {.width = size.ws_col, .height = size.ws_row}};

  return 1;
}

int update_viewport(void) {
  struct winsize size = {0};
  if (ioctl(0, TIOCGWINSZ, &size) == -1) {
    perror("TIOCGWINSZ");
    return -1;
  }
  if (size.ws_col == 0 || size.ws_row == 0)
    return 0;

  if (size.ws_col == app.viewport.width && size.ws_row == app.viewport.height)
    return 1;

  if (!resize_buffers(size.ws_col, size.ws_row))
    return -1;

  app.viewport = (Viewport){.width = size.ws_col, .height = size.ws_row};
  return 1;
}

void copy_back_to_front(void) {
  const char **front = front_buf.data;
  const char **back = back_buf.data;

  for (int y = 0; y < app.viewport.height; y++) {
    for (int x = 0; x < app.viewport.width; x++) {
      front[y * app.viewport.width + x] = back[y * app.viewport.width + x];
    }
  }
}

TuiKey input(void) {
  static int escape_state;
  static struct timespec escape_start;
  if (stop_requested) {
    app.is_running = 0;
    return KEY_NONE;
  }

  unsigned char c;
  while (read(0, &c, 1) > 0) {
    if (escape_state == 1) {
      if (c == '[' || c == 'O') {
        escape_state = 2;
        continue;
      }
      escape_state = 0;
      if (c == 'q')
        app.is_running = 0;
      return KEY_ESCAPE;
    }
    if (escape_state == 2) {
      if (c >= 0x40 && c <= 0x7e) {
        escape_state = 0;
        if (c == 'A')
          return KEY_UP;
        if (c == 'B')
          return KEY_DOWN;
      }
      continue;
    }
    switch (c) {
    case 'q':
      app.is_running = 0;
      return KEY_NONE;
    case 'r':
      return KEY_MENU;
    case 'o':
      return KEY_OBJECT_MENU;
    case 'k':
      return KEY_UP;
    case 'j':
      return KEY_DOWN;
    case '\r':
    case '\n':
      return KEY_ENTER;
    case 0x1b:
      if (clock_gettime(CLOCK_MONOTONIC, &escape_start) != 0)
        return KEY_ESCAPE;
      escape_state = 1;
      break;
    default:
      break;
    }
  }
  if (escape_state) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0 &&
        elapsed_seconds(escape_start, now) > 0.05) {
      escape_state = 0;
      return KEY_ESCAPE;
    }
  }
  return KEY_NONE;
}

static int write_all(const char *data, size_t length) {
  while (length > 0) {
    ssize_t written = write(STDOUT_FILENO, data, length);
    if (written < 0) {
      if (errno == EINTR)
        continue;
      return 0;
    }
    if (written == 0) {
      errno = EIO;
      return 0;
    }
    data += written;
    length -= (size_t)written;
  }
  return 1;
}

int clear_screen(void) {
  const char set[] = ESC "[?1049h" ESC "[?25l" ESC "[?7l" ESC "[2J" ESC "[H";
  return write_all(set, sizeof(set) - 1);
}

void restore_terminal(void) {
  const char restore[] = ESC "[?7h" ESC "[?25h" ESC "[?1049l";
  if (!write_all(restore, sizeof(restore) - 1))
    perror("restore screen");

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &start) == -1)
    perror("restore terminal");

  grid_free(&front_buf);
  grid_free(&back_buf);
  free(output_buf);
  output_buf = NULL;
  output_capacity = 0;
}

void draw_rect(Rect rect) {
  const char **back = back_buf.data;
  if (rect.width <= 0 || rect.height <= 0)
    return;

  int right = rect.x + rect.width - 1;
  int bottom = rect.y + rect.height - 1;
  for (int x = rect.x; x <= right; x++) {
    back[(size_t)rect.y * back_buf.width + x] = "─";
    back[(size_t)bottom * back_buf.width + x] = "─";
  }
  for (int y = rect.y; y <= bottom; y++) {
    back[(size_t)y * back_buf.width + rect.x] = "│";
    back[(size_t)y * back_buf.width + right] = "│";
  }
  back[(size_t)rect.y * back_buf.width + rect.x] = "┌";
  back[(size_t)rect.y * back_buf.width + right] = "┐";
  back[(size_t)bottom * back_buf.width + rect.x] = "└";
  back[(size_t)bottom * back_buf.width + right] = "┘";
}

static int append_output(size_t *length, const char *data, size_t size) {
  if (size > SIZE_MAX - *length) {
    errno = ENOMEM;
    return 0;
  }
  size_t needed = *length + size;
  if (needed > output_capacity) {
    size_t capacity = output_capacity ? output_capacity : 4096;
    while (capacity < needed) {
      if (capacity > SIZE_MAX / 2) {
        capacity = needed;
        break;
      }
      capacity *= 2;
    }
    char *data_new = realloc(output_buf, capacity);
    if (data_new == NULL)
      return 0;
    output_buf = data_new;
    output_capacity = capacity;
  }
  memcpy(output_buf + *length, data, size);
  *length = needed;
  return 1;
}

int draw_screen(void) {
  const char **front = front_buf.data;
  size_t length = 0;

  for (int y = 0; y < app.viewport.height; y++) {
    char pos[32];

    int len = snprintf(pos, sizeof(pos), ESC "[%d;1H", y + 1);
    if (len < 0 || (size_t)len >= sizeof(pos)) {
      errno = EOVERFLOW;
      return 0;
    }
    if (!append_output(&length, pos, (size_t)len))
      return 0;

    for (int x = 0; x < app.viewport.width; x++) {
      const char *cell = front[y * front_buf.width + x];
      if (!append_output(&length, cell, strlen(cell)))
        return 0;
    }
  }
  return write_all(output_buf, length);
}
