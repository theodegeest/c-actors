#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>

// ******* LOGGING LEVEL SELECTION

// #define LOGGING
// #define LOG_INFO
#define LOG_WARNING

// ******* LOGGING LEVEL SELECTION

#ifdef LOGGING
/* variadic macro that forwards everything after the prefix to printf.
   Use do{...}while(0) so it behaves like a statement in all contexts. */
#define log(...)                                                               \
  do {                                                                         \
    printf("LOGGING: " __VA_ARGS__);                                           \
  } while (0)
#else
/* compile-time disabled: no code, no argument evaluation */
#define log(...) ((void)0)
#endif

#ifdef LOG_INFO
/* variadic macro that forwards everything after the prefix to printf.
   Use do{...}while(0) so it behaves like a statement in all contexts. */
#define info(...)                                                              \
  do {                                                                         \
    printf("INFO: " __VA_ARGS__);                                              \
  } while (0)
#else
/* compile-time disabled: no code, no argument evaluation */
#define info(...) ((void)0)
#endif

#ifdef LOG_WARNING
#define warning(...)                                                           \
  do {                                                                         \
    printf("WARNING: " __VA_ARGS__);                                           \
  } while (0)
#else
#define warning(...) ((void)0)
#endif

#endif // !LOG_H_
