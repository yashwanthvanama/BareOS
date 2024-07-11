#include <barelib.h>
#include <bareio.h>
#include "tests.h"

extern uint32* text_start;    /*                                             */
extern uint32* data_start;    /*  These variables automatically contain the  */
extern uint32* bss_start;     /*  lowest  address of important  segments in  */
extern uint32* mem_start;     /*  memory  created  when  the  kernel  boots  */
extern uint32* mem_end;       /*    (see mmap.c and kernel.ld for source)    */

extern uint32 t__uart_putc_called;

void __real_initialize(void);

static const char* general_prompt[] = {
				       "  Program Compiles:                     ",
				       "  `printf' is callable:                 ",
};
static const char* printf_prompt[] = {
				       "  Calls `uart_putc`:                    ",
				       "  Text passed to `uart_putc`:           ",
				       "  Decimal '%d' template prints int:     ",
				       "  Hexidecimal '%x' template prints hex: ",
};
static const char* initialization_prompt[] = {
				       "  Prints correct Text start:            ",
				       "  Prints correct Kernel size:           ",
				       "  Prints correct Data start:            ",
				       "  Prints correct Memory start:          ",
				       "  Prints correct Memory size:           ",
};

static char* general_t[test_count(general_prompt)];
static char* printf_t[test_count(printf_prompt)];
static char* initialization_t[test_count(initialization_prompt)];

static byte same_line(const char* a, const char* b) {
  int i;
  for (i=0; a[i] == b[i] && a[i] != '\n'; i++);
  return a[i] == b[i];
}

static void general_tests(void) {
  t__set_io("", -1, 5);
  t__with_timeout(10, (void (*)(void))printf, "basic");
  maybe_timeout(1, general) {
    assert(!status_is(TIMEOUT), general_t[1], "FAIL - Timeout occured when calling printf");
  }
}

static void printf_tests(void) {
  t__set_io("", -1, 5);
  t__with_timeout(10, (void (*)(void))printf, "basic");
  t__check_io(0, "basic");

  maybe_timeout(0, printf) {
    assert(t__uart_putc_called, printf_t[0], "FAIL - 'uart_putc' never called");
  }
  maybe_timeout(1, printf) {
    assert(status_is(STDOUT_MATCH), printf_t[1], "FAIL - Output text does not match template argument");
    assert(status_is(STDOUT_COUNT), printf_t[1], "FAIL - Output text is not the correct length");
    assert(!status_is(TIMEOUT), printf_t[1], "TIMEOUT -- printf did not return");
  }
  
  t__set_io("", -1, 13);
  t__with_timeout(10, (void (*)(void))printf, "int: %d feet\n", 12);
  t__check_io(0, "int: 12 feet\n");

  maybe_timeout(2, printf) {
    assert(status_is(STDOUT_MATCH), printf_t[2], "FAIL - Decimal template text does not match the expected value");
    assert(status_is(STDOUT_COUNT), printf_t[2], "FAIL - Decimal template text is not the correct length");
    assert(!status_is(TIMEOUT), printf_t[2], "TIMEOUT -- printf did not return");
  }
  
  t__set_io("", -1, 19);
  t__with_timeout(10, (void (*)(void))printf, "hex: %x feet\n\n", 0x2ab30);
  t__check_io(0, "hex: 0x2ab30 feet\n\n");
  
  maybe_timeout(3, printf) {
    assert(status_is(STDOUT_MATCH), printf_t[3], "FAIL - Hexadecimal template text does not match the expected value");
    assert(status_is(STDOUT_COUNT), printf_t[3], "FAIL - Hexadecimal template text is not the correct length");
    assert(!status_is(TIMEOUT), printf_t[2], "TIMEOUT -- printf did not return");
  }
}

static void initialization_tests(void) {
  char test_text[5][256];
  char* ptr;
  for (int i=0; i<5; i++)
    for (int j=0; j<256; j++) test_text[i][j] = '\0';
  ptr = t__strcpy(test_text[0], "Kernel start: 0x");
  ptr = t__hexcpy(ptr, (unsigned long)text_start);
  ptr = t__strcpy(ptr, "\n");
  
  ptr = t__strcpy(test_text[1], "--Kernel size: ");
  ptr = t__intcpy(ptr, (unsigned long)(data_start - text_start));
  ptr = t__strcpy(ptr, "\n");
  
  ptr = t__strcpy(test_text[2], "Globals start: 0x");
  ptr = t__hexcpy(ptr, (unsigned long)data_start);
  ptr = t__strcpy(ptr, "\n");
  
  ptr = t__strcpy(test_text[3], "Heap/Stack start: 0x");
  ptr = t__hexcpy(ptr, (unsigned long)mem_start);
  ptr = t__strcpy(ptr, "\n");
  
  ptr = t__strcpy(test_text[4], "--Free Memory Available: ");
  ptr = t__intcpy(ptr, (unsigned long)(mem_end - mem_start));
  ptr = t__strcpy(ptr, "\n");

  t__set_io("", -1, 1000);
  t__with_timeout(10, (void (*)(void))__real_initialize);

  char* lines[5], *stdout=t__raw_stdout();
  uint32 idx = 0;
  lines[idx++] = &(stdout[0]);
  for (int i=0; stdout[i] != '\0' && idx < 5; i++)
    if (stdout[i] == '\n')
      lines[idx++] = &(stdout[i+1]);

  assert(same_line(test_text[0], lines[0]), initialization_t[0], "FAIL - Kernel start line does not match expected value");
  assert(same_line(test_text[1], lines[1]), initialization_t[1], "FAIL - Kernel size line does not match expected value");
  assert(same_line(test_text[2], lines[2]), initialization_t[2], "FAIL - Globals line does not match expected value");
  assert(same_line(test_text[3], lines[3]), initialization_t[3], "FAIL - Memory line does not match expected value");
  assert(same_line(test_text[4], lines[4]), initialization_t[4], "FAIL - Total Memory size line does not match expected value");
}

void t__ms1(uint32 idx) {
  if (idx == 0) {
    t__print("\n");
    runner("general", general);
  }
  else if (idx == 1) {
    runner("printf", printf);
  }
  else if (idx == 2) {
    runner("initialization", initialization);
  }
  else {
    t__print("\n----------------------------\n");
    feedback("General", general);
    feedback("Printf", printf);
    feedback("Initialization", initialization);
    t__print("\n");
  }
}
