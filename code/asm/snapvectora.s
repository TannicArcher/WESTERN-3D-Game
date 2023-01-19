#include "qasm.h"

#if id386
.data

fpucw:  .long   0
cw037F: .long   0x037F

.text

// void Sys_SnapVector( float *v )
.globl C(Sys_SnapVector)
C(Sys_SnapVector):
        pushl   %eax
        pushl   %ebp
        movl    %esp,%ebp

        fnstcw  fpucw
        movl    12(%ebp),%eax
        fldcw   cw037F
        flds    (%eax)
        fistpl  (%eax)
        fildl   (%eax)
        fstps   (%eax)
        flds    4(%eax)
        fistpl  4(%eax)
        fildl   4(%eax)
        fstps   4(%eax)
        flds    8(%eax)
        fistpl  8(%eax)
        fildl   8(%eax)
        fstps   8(%eax)
        fldcw   fpucw

        popl %ebp
        popl %eax
        ret

// void Sys_SnapVectorCW( float *v, unsigned short int cw )
.globl C(Sys_SnapVectorCW)
C(Sys_SnapVectorCW):
        pushl   %eax
        pushl   %ebp
        movl    %esp,%ebp

        fnstcw  fpucw
        movl    12(%ebp),%eax
        fldcw   16(%ebp)
        flds    (%eax)
        fistpl  (%eax)
        fildl   (%eax)
        fstps   (%eax)
        flds    4(%eax)
        fistpl  4(%eax)
        fildl   4(%eax)
        fstps   4(%eax)
        flds    8(%eax)
        fistpl  8(%eax)
        fildl   8(%eax)
        fstps   8(%eax)
        fldcw   fpucw

        popl %ebp
        popl %eax
        ret
#endif
