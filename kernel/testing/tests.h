#ifndef H_TESTER
#define H_TESTER

#define STDIN_COUNT 0x0
#define STDOUT_COUNT 0x1
#define STDOUT_MATCH 0x2
#define TIMEOUT 0x5
#define ALL -1
#define runner(txt, n) do {				   \
    init_tests(n ## _t, test_count(n ## _prompt));         \
    t__runner(txt, test_count(n ## _prompt), n ## _tests); \
  } while(0)
#define feedback(txt, n) t__printer("\n" txt " Tests:", n ## _t, n ## _prompt, test_count(n ## _prompt))
#define status_is(cond) (t__status & (0x1 << cond))
#define test_count(x) sizeof(x) / sizeof(x[0])
#define init_tests(x, c) for (int i=0; i<c; i++) x[i] = "OK"
#define assert(test, ls, err) if (!(test)) ls = err
#define maybe_timeout(idx, test) if (status_is(TIMEOUT)) t__mark_timeout(idx, test ## _t, test_count(test ## _prompt)); else
#define return_on_timeout(idx, test) if (status_is(TIMEOUT)) {		   \
    t__mark_timeout(idx, test ## _t, test_count(test ## _prompt)); \
    return;								   \
  }

void t__print(const char*);
void t__runner(const char* , uint32, void(*)(void));
void t__printer(const char*, char**, const char**, uint32);
void t__mem_restore(void);
void t__mem_snapshot(void);

int64  t__with_timeout(int32, void*, ...);
void t__mark_timeout(byte idx, char** arr, uint32 len);

void  t__print(const char*);
char* t__intcpy(char*, int);
char* t__hexcpy(char*, unsigned int);
char* t__strcpy(char*, const char*);
char* t__raw_stdout(void);
void t__set_io(const char* stdin, int32 stdin_c, int32 stdout_c);
byte t__check_io(uint32 count, const char* stdout);

extern uint32 t__status;

#endif
