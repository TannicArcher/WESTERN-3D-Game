
#ifndef __ASM_I386__
#define __ASM_I386__

#include "../qcommon/q_platform.h"

#ifdef __ELF__
.section .note.GNU-stack,"",@progbits
#endif

#ifdef __ELF__
#define C(label) label
#else
#define C(label) _##label
#endif

#endif
