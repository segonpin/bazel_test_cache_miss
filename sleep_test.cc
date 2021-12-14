#include <unistd.h>

/**
 * This is a "test." It is a cc_binary, so it can also be
 * compiled as a cc_test and Bazel will decide on whether it passed or failed
 * based on exit code (which is always 0 here, so the test "passes").
 */
int main(void) {
  sleep(5);
  return 0;
}
