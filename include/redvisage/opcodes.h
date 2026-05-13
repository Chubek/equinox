#ifndef REDVISAGE_OPCODES_H
#define REDVISAGE_OPCODES_H

#include <stdint.h>

enum redvisage_eop {
    REDVISAGE_EOP_CONST_BASE = 1000,
    REDVISAGE_EOP_VAR = 2000,
    REDVISAGE_EOP_STATE_INIT,
    REDVISAGE_EOP_ADD,
    REDVISAGE_EOP_MUL,
    REDVISAGE_EOP_LT,
    REDVISAGE_EOP_PRINT,
    REDVISAGE_EOP_YIELD,
    REDVISAGE_EOP_GAMMA,
    REDVISAGE_EOP_THETA,
    REDVISAGE_EOP_EXTRACT
};

static inline uint32_t redvisage_encode_const_int(int64_t value) {
    return (uint32_t)(REDVISAGE_EOP_CONST_BASE + (value & 0x7fffffff));
}

static inline int redvisage_is_const_opcode(uint32_t op) {
    return op >= REDVISAGE_EOP_CONST_BASE && op < REDVISAGE_EOP_VAR;
}

static inline int64_t redvisage_decode_const_int(uint32_t op) {
    return (int64_t)(op - REDVISAGE_EOP_CONST_BASE);
}

#endif
