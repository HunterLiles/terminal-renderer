#pragma once

#include <time.h>

static inline double elapsed_seconds(struct timespec start,
                                     struct timespec end) {
  return (double)(end.tv_sec - start.tv_sec) +
         (end.tv_nsec - start.tv_nsec) / 1e9;
}
