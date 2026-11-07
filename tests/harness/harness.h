#ifndef ER_HARNESS_H
#define ER_HARNESS_H

/*
 * Minimal test harness: tests are plain void functions listed in tables,
 * and a failed check aborts only the test that made it.  Each test runs in
 * its own empty work directory.
 */

#include <stddef.h>

#include "compare.h"
#include "platform.h"

#if defined(_MSC_VER)
#define ER_NORETURN __declspec (noreturn)
#elif defined(__GNUC__)
#define ER_NORETURN __attribute__ ((noreturn))
#else
#define ER_NORETURN
#endif

typedef void (*er_test_fn) (void);

typedef struct
{
  const char *name;
  er_test_fn fn;
  const void *ctx; /* available to the test via er_test_ctx() */
} er_test;

/* a suite is a table of tests ending with a {NULL} entry */
typedef struct
{
  const char *name;
  const er_test *tests;
} er_suite;

/* defined in suites.c: the table of all suites, ending with a {NULL} entry */
const er_suite *er_get_suites (void);

ER_NORETURN void er_fail (const char *file, int line, const char *fmt, ...);

#define ER_FAIL(...) er_fail (__FILE__, __LINE__, __VA_ARGS__)
#define ER_CHECK(cond)                                         \
  do                                                           \
  {                                                            \
    if (!(cond))                                               \
      er_fail (__FILE__, __LINE__, "check failed: %s", #cond); \
  } while (0)
#define ER_CHECK_MSG(cond, ...)                  \
  do                                             \
  {                                              \
    if (!(cond))                                 \
      er_fail (__FILE__, __LINE__, __VA_ARGS__); \
  } while (0)
#define ER_CHECK_NEAR(a, b, tol)                                                                                   \
  do                                                                                                               \
  {                                                                                                                \
    double er_a_ = (a), er_b_ = (b), er_t_ = (tol);                                                                \
    if (!(er_a_ - er_b_ <= er_t_ && er_b_ - er_a_ <= er_t_))                                                       \
      er_fail (__FILE__, __LINE__, "%s = %.12g, expected %s = %.12g (tolerance %g)", #a, er_a_, #b, er_b_, er_t_); \
  } while (0)

/* the ctx of the running test */
const void *er_test_ctx (void);

/* Absolute paths to files in the fixture directory, the expected-output
   directory and the directory holding the evalresp programs.  Each result
   is valid until four more path calls have been made. */
const char *er_data_path (const char *name);
const char *er_data_dir (void);
const char *er_expected_path (const char *name);
const char *er_bin_path (const char *program);

/* Fail the running test unless result matches target (see compare.h). */
void er_assert_files_match (const char *result, const char *target, const er_cmp *opts);

#endif
