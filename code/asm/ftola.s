
#include "qasm.h"

#if id386

.data

temp:   .single   0.0
fpucw:  .long     0

// Precision Control Field , 2 bits / 0x0300
// PC24 0x0000   Single precision (24 bits).
// PC53 0x0200   Double precision (53 bits).
// PC64 0x0300   Extended precision (64 bits).

// Rounding Control Field, 2 bits / 0x0C00
// RCN  0x0000   Rounding to nearest (even).
// RCD  0x0400   Rounding down (directed, minus).
// RCU  0x0800   Rounding up (directed plus).
// RC0  0x0C00   Rounding towards zero (chop mode).


// rounding towards nearest (even)
cw027F: .long     0x027F
cw037F: .long     0x037F

// rounding towards zero (chop mode)
cw0E7F: .long     0x0E7F
cw0F7F: .long     0x0F7F


.text

//
// int qftol( void ) - default control word
//

.globl C(qftol)

C(qftol):
        fistpl temp
        movl temp,%eax
        ret


//
// int qftol027F( void ) - DirectX FPU
//

.globl C(qftol027F)

C(qftol027F):
        fnstcw fpucw
        fldcw  cw027F
        fistpl temp
        fldcw  fpucw
        movl temp,%eax
        ret

//
// int qftol037F( void ) - Linux FPU
//

.globl C(qftol037F)

C(qftol037F):
        fnstcw fpucw
        fldcw  cw037F
        fistpl temp
        fldcw  fpucw
        movl temp,%eax
        ret


//
// int qftol0F7F( void ) - ANSI
//

.globl C(qftol0F7F)

C(qftol0F7F):
        fnstcw fpucw
        fldcw  cw0F7F
        fistpl temp
        fldcw  fpucw
        movl temp,%eax
        ret

//
// int qftol0E7F( void )
//

.globl C(qftol0E7F)

C(qftol0E7F):
        fnstcw fpucw
        fldcw  cw0E7F
        fistpl temp
        fldcw  fpucw
        movl temp,%eax
        ret



//
// long Q_ftol( float q )
//

.globl C(Q_ftol)

C(Q_ftol):
        flds 4(%esp)
        fistpl temp
        movl temp,%eax
        ret


//
// long qftol0F7F( float q ) - Linux FPU
//

.globl C(Q_ftol0F7F)

C(Q_ftol0F7F):
        fnstcw fpucw
        flds 4(%esp)
        fldcw  cw0F7F
        fistpl temp
        fldcw  fpucw
        movl temp,%eax
        ret
#endif
