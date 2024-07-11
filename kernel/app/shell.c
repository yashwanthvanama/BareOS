#include <bareio.h>
#include <barelib.h>
#include <shell.h>
#include <thread.h>
#include <syscall.h>
#include <sleep.h>
#include <semaphore.h>
#include <malloc.h>
#include <tty.h>
#include <fs.h>

#define PROMPT "bareOS$ "  /*  Prompt printed by the shell to the user  */
#define HELLOWITHARGS "hello "
#define HELLOWITHOUTARGS "hello\0"
#define ECHOWITHARGS "echo "
#define ECHOWITHOUTARGS "echo\0"
#define FORMATTER "$?"

int containsSubstring(const char*, const char*);
int startsWith(const char*, const char*);
void replaceString(char*, const char*, const char*);

/*
 * 'shell' loops forever, prompting the user for input, then calling a function based
 * on the text read in from the user.
 */
byte shell(char* arg) {
  char c;
  char input[1024];
  int i = 0;
  int lastCommandOutput = 0;
  while(1) {
    printf("%s",PROMPT);
    while(1) {
      c = tty_getc();
      if(c == '\025')
        continue;
      if(c != '\n') {
        input[i] = c;
        i++;
      }
      else {
        input[i] = '\0';
        break;
      }
    }
    if(*input == '\0') {
      continue;
    }
    if(containsSubstring(input, FORMATTER)) {
      char lastCommandOutputString[4];
      integerToString(lastCommandOutput, lastCommandOutputString);
      replaceString(input, FORMATTER, lastCommandOutputString);
    }
    if(startsWith(input, ECHOWITHARGS) || startsWith(input, ECHOWITHOUTARGS)) {
      int32 threadid = create_thread((void (*)(void))builtin_echo, input, i);
      resume_thread(threadid);
      lastCommandOutput = join_thread(threadid);
    }
    else if(startsWith(input, HELLOWITHARGS) || startsWith(input, HELLOWITHOUTARGS)) {
      int32 threadid = create_thread((void (*)(void))builtin_hello, input, i);
      resume_thread(threadid);
      lastCommandOutput = join_thread(threadid);
    }
    else {
      printf("Unknown command\n");
    }
    clearArray(input);
    i = 0;
  }
  return 0;
}

int containsSubstring(const char* str1, const char* str2) {
  int i, j;

    for (i = 0; str1[i] != '\0'; i++) {
        if (str1[i] == str2[0]) {
            for (j = 1; str2[j] != '\0' && str1[i + j] == str2[j]; j++) {
            }
            if (str2[j] == '\0') {
                return 1;
            }
        }
    }
    return 0;
}

int startsWith(const char* mainString, const char* prefixString) {
  int i = 0;
    while (prefixString[i] != '\0') {
        if (mainString[i] != prefixString[i]) {
            return 0;
        }
        i++;
    }
    return 1;
}

void replaceString(char* mainString, const char* oldString, const char* newString) {
    int i, j, k;
    int mainStringLength = 0, oldStringLength = 0, newStringLength = 0;

    while (mainString[mainStringLength] != '\0') {
        mainStringLength++;
    }
    while (oldString[oldStringLength] != '\0') {
        oldStringLength++;
    }
    while (newString[newStringLength] != '\0') {
        newStringLength++;
    }

    for (i = 0; i <= mainStringLength - oldStringLength; i++) {
        for (j = 0; j < oldStringLength && mainString[i + j] == oldString[j]; j++) {
        }
        if (j == oldStringLength) {
            for (k = mainStringLength; k >= i + oldStringLength; k--) {
                mainString[k + newStringLength - oldStringLength] = mainString[k];
            }

            for (k = 0; k < newStringLength; k++) {
                mainString[i + k] = newString[k];
            }
            mainStringLength = mainStringLength + newStringLength - oldStringLength;
        }
    }
}