#ifndef SAFE_ALLOC_H_
#define SAFE_ALLOC_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *__safe_malloc_impl(size_t size, const char *file,
                                       int line) {
  void *p = malloc(size);
  if (p == NULL) {
    fprintf(stderr, "malloc(%zu) failed at %s:%d: %s\n", size, file, line,
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  return p;
}

#define safe_malloc(size) __safe_malloc_impl((size), __FILE__, __LINE__)

static inline void *__safe_calloc_impl(size_t nmemb, size_t size,
                                       const char *file, int line) {
  void *p = calloc(nmemb, size);
  if (p == NULL) {
    fprintf(stderr, "calloc(%zu, %zu) failed at %s:%d: %s\n", nmemb, size, file,
            line, strerror(errno));
    exit(EXIT_FAILURE);
  }
  return p;
}

#define safe_calloc(nmemb, size)                                               \
  __safe_calloc_impl((nmemb), (size), __FILE__, __LINE__)

#endif // !SAFE_ALLOC_H_
