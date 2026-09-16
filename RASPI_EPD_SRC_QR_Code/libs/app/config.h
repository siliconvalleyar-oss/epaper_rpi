#pragma once

// Detección de arquitectura: 32 vs 64 bits
#if (defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ == 4))
    #define CPU_32_BITS
#else
    #define CPU_64_BITS
#endif
