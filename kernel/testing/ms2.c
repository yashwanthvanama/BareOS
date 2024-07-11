#include <barelib.h>
#include <bareio.h>
#include "tests.h"

extern uint32 t__hello_called;
extern uint32 t__echo_called;
extern uint32 t__status;

byte builtin_echo(char*);
byte builtin_hello(char*);
byte __real_shell(char*);

static const char* general_prompt[] = {
		       "  Program Compiles:                         ",
		       "  `echo` is callable:                       ",
		       "  `hello` is callable:                      "
};
static const char* echo_prompt[] = {
		       "  Prints the argument text:                 ",
		       "  Echos a line of text:                     ",
		       "  Returns when encountering an empty line:  ",
		       "  Returns 0 when an argument is passed in:  ",
		       "  Returns the number of characters read:    "
};
static const char* hello_prompt[] = {
		       "  Prints error on no argument received:     ",
		       "  Prints the correct string:                ",
		       "  Returns 1 on error:                       ",
		       "  Returns 0 on success:                     "
};
static const char* shell_prompt[] = {
		       "  Prints error on bad command:              ",
		       "  Reads only up to newline each command:    ",
		       "  Sucessfully calls `echo`:                 ",
		       "  Successfully calls `hello`:               ",
		       "  Shell loops after command completes:      ",
		       "  Shell replaces '$?' with previous result: "
};

static char* general_t[test_count(general_prompt)];
static char* echo_t[test_count(echo_prompt)];
static char* hello_t[test_count(echo_prompt)];
static char* shell_t[test_count(shell_prompt)];

static void general_tests(void) {
  t__set_io("", -1, 20);
  t__with_timeout(10, (void (*)(void))builtin_echo, "echo stuff");
  maybe_timeout(1, general) {
    assert(!status_is(TIMEOUT), general_t[1], "FAIL - Timeout occured when calling 'builtin_echo'");
  }

  t__set_io("", -1, 20);
  t__with_timeout(10, (void (*)(void))builtin_hello, "hello stuff");
  maybe_timeout(2, general) {
    assert(!status_is(TIMEOUT), general_t[2], "FAIL - Timeout occured when calling 'builtin_hello'");
  }
}

static void echo_tests(void) {
  byte result;
  t__set_io("", -1, 10);
  result = t__with_timeout(10, (void (*)(void))builtin_echo, "echo Echo test");
  t__check_io(0, "Echo test\n");

  maybe_timeout(0, echo) {
    assert(status_is(STDOUT_MATCH), echo_t[0], "FAIL - Printed text does not match expected output");
    assert(status_is(STDOUT_COUNT), echo_t[0], "FAIL - Output text is not the expected length");
  }

  maybe_timeout(3, echo) {
    assert(result == 0, echo_t[3], "FAIL - Returned non-zero value");
  }

  t__set_io("Echo this string\n", 18, 17);
  result = t__with_timeout(10, (void (*)(void))builtin_echo, "echo");
  t__check_io(17, "Echo this string\n");

  maybe_timeout(1, echo) {
    assert(status_is(STDOUT_MATCH), echo_t[1], "FAIL - Did not print line text from stdin");
  }

  t__set_io("differenter text\n\n", -1, 40);
  t__with_timeout(10, (void (*)(void))builtin_echo, "echo");

  maybe_timeout(2, echo) {
    assert(!status_is(TIMEOUT), echo_t[2], "TIMEOUT -- 'builtin_echo' never returned after empty newline");
  }

  t__set_io("differenter text\n\n", -1, 40);
  result = t__with_timeout(10, (void (*)(void))builtin_echo, "echo");

  maybe_timeout(3, echo) {
    assert(result == 16, echo_t[3], "FAIL - Did not return the correct count");
    assert(!status_is(TIMEOUT), echo_t[3], "TIMEOUT -- 'builtin_echo' never returned");
  }
}

static void hello_tests(void) {
  byte result;
  t__set_io("", -1, 21);
  result = t__with_timeout(10, (void (*)(void))builtin_hello, "hello");
  t__check_io(0, "Error - bad argument\n");

  maybe_timeout(0, hello) {
    assert(status_is(STDOUT_MATCH), hello_t[0], "FAIL - Text did not match the expected error");
    assert(status_is(STDOUT_COUNT), hello_t[0], "FAIL - Error text not the correct length");
  }
  maybe_timeout(2, hello) {
    assert(result == 1, hello_t[2], "FAIL - Return value was not 1 on error");
  }

  t__set_io("", -1, 17);
  result = t__with_timeout(10, (void (*)(void))builtin_hello, "hello username");
  t__check_io(0, "Hello, username!\n");

  maybe_timeout(1, hello) {
    assert(status_is(STDOUT_MATCH), hello_t[1], "FAIL - Text did not match the expected output");
    assert(status_is(STDOUT_COUNT), hello_t[1], "FAIL - Output text was not the expected length");
  }
  maybe_timeout(3, hello) {
    assert(result == 0, hello_t[3], "FAIL - Return value was non-zero on success");
  }
}

static void shell_tests(void) {
  t__mem_restore();
  t__set_io("badcall\n", 9, 32);
  t__with_timeout(10, (void (*)(void))__real_shell, "");
  t__check_io(9, "bareOS$ Unknown command\nbareOS$ ");

  maybe_timeout(0, shell) {
    assert(status_is(STDOUT_MATCH), shell_t[0], "FAIL - Error text did not match expected value");
    assert(status_is(STDOUT_COUNT), shell_t[0], "FAIL - Error text was not the expected length");
  }
  maybe_timeout(1, shell) {
    assert(status_is(STDIN_COUNT), shell_t[1], "FAIL - Reads from `uart_getc` after '\\n' character");
  }

  t__mem_restore();
  t__set_io("echo foobar\n", 13, 15);
  t__echo_called = 0;
  t__with_timeout(10, (void (*)(void))__real_shell, "");
  assert(t__echo_called, shell_t[2], "FAIL - hello not called from shell on 'hello'");

  t__mem_restore();
  t__set_io("hello value\n", 13, 23);
  t__hello_called = 0;
  t__with_timeout(10, (void (*)(void))__real_shell, "");

  assert(t__hello_called, shell_t[3], "FAIL - hello not called from shell on 'hello'");

  t__mem_restore();
  t__set_io("hello\necho $?\n", 15, 47);
  t__with_timeout(10, (void (*)(void))__real_shell, "");
  t__check_io(15, "bareOS$ Error - bad argument\nbareOS$ 1\nbareOS$ ");

  char* stdout = t__raw_stdout();
  maybe_timeout(4, shell) {
    assert(status_is(STDOUT_COUNT), shell_t[4], "FAIL - Shell did not loop after function");
  }
  maybe_timeout(5, shell) {
    assert(stdout[37] == '1', shell_t[5], "FAIL - '$?' not replaced with previous return value");
  }
}

void t__ms2(uint32 idx) {
  if (idx == 0) {
    t__print("\n");
    runner("general", general);
  }
  else if (idx == 1) {
    runner("echo", echo);
  }
  else if (idx == 2) {
    runner("hello", hello);
  }
  else if (idx == 3) {
    runner("shell", shell);
  }
  else {
    t__print("\n----------------------------\n");
    feedback("General", general);
    feedback("Echo", echo);
    feedback("Hello", hello);
    feedback("Shell", shell);
    t__print("\n");
  }
}
