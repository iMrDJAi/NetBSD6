/*	$NetBSD: run-atoint.c,v 1.1.1.3.6.2 2015/11/08 00:16:09 snj Exp $	*/

/* AUTOGENERATED FILE. DO NOT EDIT. */

//=======Test Runner Used To Run Each Test Below=====
#define RUN_TEST(TestFunc, TestLineNum) \
{ \
  Unity.CurrentTestName = #TestFunc; \
  Unity.CurrentTestLineNumber = TestLineNum; \
  Unity.NumberOfTests++; \
  if (TEST_PROTECT()) \
  { \
      setUp(); \
      TestFunc(); \
  } \
  if (TEST_PROTECT() && !TEST_IS_IGNORED) \
  { \
    tearDown(); \
  } \
  UnityConcludeTest(); \
}

//=======Automagically Detected Files To Include=====
#include "unity.h"
#include <setjmp.h>
#include <stdio.h>
#include "config.h"
#include "ntp_stdlib.h"
#include "ntp_calendar.h"

//=======External Functions This Runner Calls=====
extern void setUp(void);
extern void tearDown(void);
extern void test_RegularPositive(void);
extern void test_RegularNegative(void);
extern void test_PositiveOverflowBoundary(void);
extern void test_NegativeOverflowBoundary(void);
extern void test_PositiveOverflowBig(void);
extern void test_IllegalCharacter(void);


//=======Test Reset Option=====
void resetTest(void);
void resetTest(void)
{
  tearDown();
  setUp();
}

char const *progname;


//=======MAIN=====
int main(int argc, char *argv[])
{
  progname = argv[0];
  UnityBegin("atoint.c");
  RUN_TEST(test_RegularPositive, 7);
  RUN_TEST(test_RegularNegative, 8);
  RUN_TEST(test_PositiveOverflowBoundary, 9);
  RUN_TEST(test_NegativeOverflowBoundary, 10);
  RUN_TEST(test_PositiveOverflowBig, 11);
  RUN_TEST(test_IllegalCharacter, 12);

  return (UnityEnd());
}
