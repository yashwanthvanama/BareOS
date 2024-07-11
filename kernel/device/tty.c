#include <barelib.h>
#include <semaphore.h>
#include <interrupts.h>
#include <tty.h>

uint32 tty_in_sem;         /* Semaphore used to lock `tty_getc` if waiting for data                                            */
uint32 tty_out_sem;        /* Semaphore used to lock `tty_putc` if waiting for space in the queue                              */
char tty_in[TTY_BUFFLEN];  /* Circular buffer for storing characters read from the UART until requested by a thread            */
char tty_out[TTY_BUFFLEN]; /* Circular buffer for storing character to be written to the UART while waiting for it to be ready */
uint32 tty_in_head = 0;    /* Index of the first character in `tty_in`                                                         */
uint32 tty_in_count = 0;   /* Number of characters in `tty_in`                                                                 */
uint32 tty_out_head = 0;   /* Index of the first character in `tty_out`                                                        */
uint32 tty_out_count = 0;  /* Number of characters in `tty_out`                                                                */

/*  Initialize the `tty_in_sem` and `tty_out_sem` semaphores  *
 *  for later TTY calls.                                      */
void tty_init(void) {
  tty_in_sem = sem_create(0);
  tty_out_sem = sem_create(0);
}

/*  Get a character  from the `tty_in`  buffer and remove  *
 *  it from the circular buffer.  If the buffer is empty,  * 
 *  wait on  the semaphore  for data to be  placed in the  *
 *  buffer by the UART.                                    */
char tty_getc(void) {
  char mask = disable_interrupts();                               /*  Prevent interruption while char is being read  */
  /*if(tty_in_count == 0) {
    sem_wait(tty_in_sem);
  }*/
  while(tty_in_count == 0);
  char c = tty_in[tty_in_head];
  tty_in_head = (tty_in_head + 1) % TTY_BUFFLEN;
  tty_in_count -= 1;
  restore_interrupts(mask);
  return c;
}

/*  Place a character into the `tty_out` buffer and enable  *
 *  uart interrupts.   If the buffer is  full, wait on the  *
 *  semaphore  until notified  that there  space has  been  *
 *  made in the  buffer by the UART. */
void tty_putc(char ch) {
  char mask = disable_interrupts();                               /*  Prevent interruption while char is being written  */
  if(tty_out_count == TTY_BUFFLEN) {
    sem_wait(tty_out_sem);
  }
  tty_out[tty_out_head + tty_out_count] = ch;
  tty_out_count++;
  set_uart_interrupt(1);
  restore_interrupts(mask);
}
