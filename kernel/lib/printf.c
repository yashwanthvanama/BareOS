#include <barelib.h>
#include <bareio.h>
#include <tty.h>

void integerToString(unsigned long input, char charArray[]);
void integerToHex(unsigned long input, char charArray[]);
void printArgument(char charArray[]);
void clearArray(char charArray[]);

void printf(const char* input, ...){
  va_list arguments;
  va_start(arguments, input);
  char x;
  char charArray[20];

  while(*input != '\0') {
    x = (char)(*input);
    if(x == '%' && *(input + 1) == 'd') {
      unsigned long y = va_arg(arguments, unsigned long);
      integerToString(y, charArray);
      printArgument(charArray);
      input = input + 1;
      clearArray(charArray);
    }
    else if(x == '%' && *(input+1) == 'x') {
      tty_putc('0');
      tty_putc('x');
      unsigned long y = va_arg(arguments,unsigned long);
      //tty_putc(y);
      integerToHex(y,charArray);
      printArgument(charArray);
      input = input + 1;
      clearArray(charArray);
    }
    else if(x == '%' && *(input + 1) == 's') {
      char* s = va_arg(arguments, char*);
      while(*s != '\0') {
        tty_putc((char)(*s));
        s = s+1;
      }
      input = input + 1;
    }
    else if(x == '%' && *(input + 1) == 'c') {
      int c = va_arg(arguments, int);
      tty_putc((char)c);
      input = input + 1;
    }
    else{
      tty_putc(x);
    }
    input = input + 1;
  }

  va_end(arguments);
}

void integerToString(unsigned long input, char charArray[]){
  int i = 0;
  int isNegative = 0;

  if(input < 0) {
    isNegative = 1;
    input = -input;
  }
  
  do {
    charArray[i++] = input%10 + '0';
    input = input/10;
  } while(input > 0);

  if(isNegative == 1) {
    charArray[i++] = '-';
  }

  int start = 0;
  int end = i - 1;
  while (start<end) {
    int temp = charArray[start];
    charArray[start] = charArray[end];
    charArray[end] = temp;
    start++;
    end--;
  }
  charArray[i] = '\0';
}


void integerToHex(unsigned long input, char charArray[]) {
  int i = 0;
  while(input != 0) {
    int remainder = input%16;

    if(remainder < 10) {
      charArray[i] = '0' + remainder;
    }
    else {
      charArray[i] = remainder + 'a' - 10;
    }

    i = i + 1;
    input = input/16;
  }

  int start = 0;
  int end = i-1;
  while (start < end) {
    int temp = charArray[start];
    charArray[start] = charArray[end];
    charArray[end] = temp;
    start++;
    end--;
  }
  charArray[i] = '\0';
}

void printArgument(char charArray[]) {
  int i = 0;

  while(charArray[i] != '\0') {
    tty_putc(charArray[i]);
    i++;
  }
}

void clearArray(char charArray[]) {
  int i = 0;

  while(charArray[i] != '\0') {
    charArray[i] = '\0';
    i++;
  }
}
