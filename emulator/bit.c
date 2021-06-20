/**
 * @file bit.c
 * @brief Bit operations for GameBoy Emulator
 *
 * @author S. Horvath-Mikulas & R.Gerber
 * @date 2020
 */

#include "bit.h"

// ==== see bit.h ========================================
uint8_t lsb4(uint8_t value) {
    return value & ((1 << 4) - 1);
}

// ==== see bit.h ========================================
uint8_t msb4(uint8_t value) {
    return value >> 4;
}

// ==== see bit.h ========================================
uint8_t lsb8(uint16_t value) {
    return value & ((1 << 8) - 1);
}

// ==== see bit.h ========================================
uint8_t msb8(uint16_t value) {
    return value >> 8;
}

// ==== see bit.h ========================================
uint16_t merge8(uint8_t v1, uint8_t v2) {
    return v1 | (v2 << 8); 
}

// ==== see bit.h ========================================
uint8_t merge4(uint8_t v1, uint8_t v2) {
    return lsb4(v1) | (v2 << 4);
}

// ==== see bit.h ========================================
bit_t bit_get(uint8_t value, int index) {
	index = CLAMP07(index);
	int mask = 1 << index;
	return (value & mask) >> index;
}

// ==== see bit.h ========================================
void bit_set(uint8_t* value, int index) {
    if (value != NULL) {
	    index = CLAMP07(index);
        *value |= (1 << index);
    }
}

// ==== see bit.h ========================================
void bit_unset(uint8_t* value, int index) {
    if (value != NULL) {
	    index = CLAMP07(index);
        *value &= ~(1 << index);
    }
}

// ==== see bit.h ========================================
void bit_rotate(uint8_t* value, rot_dir_t dir, int d) {
	if (value != NULL && (dir == LEFT || dir == RIGHT)) {
        d = CLAMP07(d);
	    uint8_t int_med1 = 0;
	    uint8_t int_med2 = 0;
	    switch (dir) {
		    case LEFT:
			    int_med1 = *value >> (BIT_8 - d);
			    int_med2 = *value << d;
			    break;
		    case RIGHT:
			    int_med1 = *value << (BIT_8 - d);
			    int_med2 = *value >> d;
			    break;
		}
	    *value = int_med1 | int_med2;
    }
}

// ==== see bit.h ========================================
void bit_edit(uint8_t* value, int index, uint8_t v) {
    if (value != NULL) {
	    if (v == 0) {
            bit_unset(value, index);
        } else {
            bit_set(value, index);
        }
    } 
}