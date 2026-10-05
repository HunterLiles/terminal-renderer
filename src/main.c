#include "core/time_utils.h"
#include "renderer/renderer.h"
#include "terminal/tui.h"
#include "terminal/ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {

  struct timespec start;
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    perror("clock_gettime");
    return EXIT_FAILURE;
  }

  if (!init_terminal())
    return EXIT_FAILURE;

  int result = EXIT_SUCCESS;
  Ui ui = {0};
  FrameStats stats = {0};
  struct timespec previous_frame = {0};
  int frame_completed = 0;
  double stats_elapsed = 0.0;
  unsigned int stats_frames = 0;

  if (!clear_screen()) {
    perror("clear screen");
    result = EXIT_FAILURE;
    goto cleanup;
  }
  while (app.is_running) {
    struct timespec frame_start;
    if (clock_gettime(CLOCK_MONOTONIC, &frame_start) == -1) {
      perror("clock_gettime");
      result = EXIT_FAILURE;
      break;
    }
    ui_handle_key(&ui, input());
    if (!app.is_running)
      break;

    int viewport_status = update_viewport();
    if (viewport_status == -1) {
      result = EXIT_FAILURE;
      break;
    }
    if (viewport_status == 0) {
      frame_completed = 0;
      stats_elapsed = 0.0;
      stats_frames = 0;
      continue;
    }

    if (frame_completed) {
      stats_elapsed += elapsed_seconds(previous_frame, frame_start);
      stats_frames++;
      if (stats_elapsed >= 0.25) {
        stats.fps = stats_frames / stats_elapsed;
        stats.frame_ms = stats_elapsed * 1000.0 / stats_frames;
        stats_elapsed = 0.0;
        stats_frames = 0;
      }
    }

    Rect rect = ui_render_area(app.viewport.width, app.viewport.height);

    // Draw last completed frame before starting new frame.
    if (!draw_screen()) {
      perror("draw screen");
      result = EXIT_FAILURE;
      break;
    }

    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
      perror("clock_gettime");
      result = EXIT_FAILURE;
      break;
    }
    double time = elapsed_seconds(start, now);

    const char **back = back_buf.data;
    for (int y = 0; y < app.viewport.height; y++)
      for (int x = 0; x < app.viewport.width; x++)
        back[(size_t)y * back_buf.width + x] = " ";

    if (rect.width > 0 && rect.height > 0) {
      if (!render(rect.width, rect.height, time) ||
          !generate_image(rect.width, rect.height)) {
        perror("render frame");
        result = EXIT_FAILURE;
        break;
      }

      Grid image = get_image();
      const BrailleCell *cells = image.data;
      for (int y = 0; y < rect.height; y++)
        for (int x = 0; x < rect.width; x++)
          back[(size_t)(rect.y + y) * back_buf.width + rect.x + x] =
              cells[(size_t)y * image.width + x];

      draw_rect(rect);
    }
    if (!ui_draw(&ui, &back_buf, &stats)) {
      perror("draw menu");
      result = EXIT_FAILURE;
      break;
    }

    copy_back_to_front();
    previous_frame = frame_start;
    frame_completed = 1;
  }
cleanup:
  restore_terminal();
  ui_free(&ui);
  free_renderer();
  return result;
}
