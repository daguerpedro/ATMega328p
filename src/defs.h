#pragma once

#define setBit(REG, BIT)    ((REG) |= 1 << (BIT))
#define clearBit(REG, BIT)  ((REG) &= ~(1 << (BIT)))
#define toggleBit(REG, BIT) ((REG) ^= 1 << (BIT))
#define isBitSet(REG, BIT) (((REG) & 1 << (BIT)) != false)
#define isBitClear(REG, BIT) (!isBitSet(REG, BIT))
