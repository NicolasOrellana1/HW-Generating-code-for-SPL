// $Id: regname.h,v 1.2 2024/10/23 13:38:20 leavens Exp $
#ifndef _REGNAME_H
#define _REGNAME_H
#include "machine_types.h"

#define NUM_REGISTERS 8

// some important register numbers in the ISA
#define GP 0
#define SP 1
#define FP 2
#define RA 7

// requires 0 <= n && n < NUM_REGISTERS
// Return the standard symbolic name for n
extern const char *regname_get(reg_num_type n);
#endif

