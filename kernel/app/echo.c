#include <bareio.h>
#include <barelib.h>
#include <tty.h>


/*
 * 'builtin_echo' reads in a line of text from the UART
 * and prints it.  It will continue to do this until the
 * line read from the UART is empty (indicated with a \n
 * followed immediately by another \n).
 */
byte builtin_echo(char* arg) {
  if(*(arg + 4) != '\0') {
    printf("%s\n", (arg + 5));
    return 0;
  }
  else {
    char c;
    char input[1024];
    int i = 0;
    int characters_read = 0;
    while(1) {
      c = tty_getc();
      if(c == '\n') {
        input[i] = '\0';
        if(i == 0) {
          //printf("Characters read: %d\n", characters_read);
          return characters_read;
        }
        else {
          characters_read += i;
          printf("%s\n", input);
          i = 0;
        }
      }
      else {
        input[i] = c;
        i++;
      }
    }
  }
}
