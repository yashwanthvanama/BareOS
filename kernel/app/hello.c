#include <bareio.h>
#include <barelib.h>


/*
 * 'builtin_hello' prints "Hello, <text>!\n" where <text> is the contents 
 * following "builtin_hello " in the argument and returns 0.  
 * If no text exists, print and error and return 1 instead.
 */
byte builtin_hello(char* arg) {
  if(*(arg+6) == '\0') {
    printf("Error - bad argument\n");
    return 1;
  }
  else {
    printf("Hello, %s!\n", arg+6);
    return 0;
  }
}
