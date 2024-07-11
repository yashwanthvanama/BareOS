/*
 *  This file contains functions for initializing and managing the
 *  'Platform-Level Interrupt Controller' or PLIC.
 *  These functions must be called to allow interrupts to be passed
 *  to the interrupt vector (see '__traps' in bootstrap.s).
 */

#include <barelib.h>
#include <interrupts.h>

#define TRAP_EXTERNAL_ENABLE 0x800
#define EXTERNAL_PENDING_ADDR 0xc001000
#define EXTERNAL_THRESH_ADDR  0xc200000

extern void uart_handler(void);

/*
 *  This function is called during bootstrapping to enable interrupts
 *  from the PLIC (see bootstrap.s)
 */
void plic_init(void) {
  uint32* plic_thresh_addr = (uint32*)EXTERNAL_THRESH_ADDR;
  *plic_thresh_addr  = 0x0;
  set_interrupt(TRAP_EXTERNAL_ENABLE);
}


/* 
 * This function is automatically triggered whenver an external 
 * event occurs.  (see '__traps' in bootstrap.s)
 */
interrupt handle_plic(void) {
  uint32* plic_pending_addr = (uint32*)EXTERNAL_PENDING_ADDR;
  if (*plic_pending_addr == 0x400)
      uart_handler();
  *plic_pending_addr = 0x0;
}

