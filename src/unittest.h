#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <pprint/pprint.h>

#define DEFINE_TEST_H(FUNC_NAME) void test_##FUNC_NAME(void)

#define TEST_START(FUNC_NAME) void test_##FUNC_NAME(void)

#define TEST_END() }

#define UNIT_TEST_TEST(VAL, OP, EQ)                                           \
  do {                                                                        \
    if (VAL OP EQ) {                                                          \
      printf("[\x1b[32mPASS\x1b[0m]  %s:%d\n", __FILE__, __LINE__);           \
    } else {                                                                  \
      printf("[\x1b[31mFAIL\x1b[0m]  %s:%d %s %s %s is not true\n", __FILE__, \
          __LINE__, #VAL, #OP, #EQ);                                          \
      exit(1);                                                                \
    }                                                                         \
  } while (0);

#define UNIT_TEST_DO(FUNC_NAME) test_##FUNC_NAME();

#endif
