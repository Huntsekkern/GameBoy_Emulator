/**
 * @file alu.c
 * @brief ALU for GameBoy Emulator
 *
 * @author S. Horvath-Mikulas & R.Gerber
 * @date 2020
 */

#include "alu.h"
#include "stdio.h"

int update_flags(flags_t* flags_to_update, uint16_t value, bit_t sub_bool, uint8_t msb_low, uint8_t msb_high);

// ==== see alu.h ========================================
flag_bit_t get_flag(flags_t flags, flag_bit_t flag) {
	// check argument validity
	switch (flag) {
		case FLAG_Z:
		case FLAG_N:
		case FLAG_H:
		case FLAG_C:	return flags & flag;
		default: 		return 0;
	}
}

// ==== see alu.h ========================================
void set_flag(flags_t* flags, flag_bit_t flag) {
	// check argument validity
	if (flags != NULL) {
		switch (flag) {
			case FLAG_Z:
			case FLAG_N:
			case FLAG_H:
			case FLAG_C:	*flags |= (uint8_t) flag;
							break;
			default: ;
		}
	}
}

// ==== private function ================================
int update_flags(flags_t* flags, uint16_t value, bit_t sub_bool, uint8_t low, uint8_t msb_high) {
	M_REQUIRE_NON_NULL(flags);
	
	// reset all flags to be 0 and set those which need to be modified
	*flags = 0;
	// check whether the result value is 0
	if (value == 0) set_Z(flags);
	// set the N flag in case of a substraction
	if(sub_bool) set_N(flags);
	// check whether half-carry occured by looking if the bit 4 of the operation of the lower halfs != 0
	if (bit_get(low, 4) != 0) set_H(flags);
	// check whether carry occured by looking if the msb of the operation of the upper halfs != 0
	if (bit_get(msb_high, 0) != 0) set_C(flags);
	
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_add8(alu_output_t* result, uint8_t x, uint8_t y, bit_t c0) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(c0 == 0 || c0 == 1, ERR_BAD_PARAMETER, "input carry (%u) should be binary (== 0 || == 1)", c0);
	// divide 8 bit addition into two times 4 bit additions, i.e. lsb4 and msb4
	uint8_t add_lsb4 = (uint8_t) (lsb4(x) + lsb4(y) + c0);
	uint8_t add_msb4 = (uint8_t) (msb4(x) + msb4(y) + msb4(add_lsb4));
	// merge the result of the disjoint operations
	uint8_t merge_add = merge4(add_lsb4, add_msb4);
	// assign the result of addition to the value of the ALU operation
	result->value = merge_add;
	
	M_EXIT_IF_ERR(update_flags(&result->flags, merge_add, 0, add_lsb4, msb4(add_msb4)));
	
	// return with no errors
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_sub8(alu_output_t* result, uint8_t x, uint8_t y, bit_t b0) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(b0 == 0 || b0 == 1, ERR_BAD_PARAMETER, "input borrow (%u) should be binary (== 0 || == 1)", b0);
	// divide 8 bit subtraction into two times 4 bit subtraction, i.e. lsb4 and msb4
	uint8_t sub_lsb4 = lsb4(x) - (lsb4(y) + b0);
	uint8_t sub_msb4 = msb4(x) - msb4(y) + msb4(sub_lsb4);
	// merge the result of the disjoint operations
	uint8_t merge_sub = merge4(sub_lsb4, sub_msb4);
	// assign the result of addition to the value of the ALU operation
	result->value = merge_sub;
	
	M_EXIT_IF_ERR(update_flags(&result->flags, merge_sub, 1, sub_lsb4, (y + b0) > x));
	
	// return with no errors
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_add16_low(alu_output_t* result, uint16_t x, uint16_t y) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	
	// divide 16 bit addition into two times 8 bit additions, i.e. lsb8 and msb8
	uint16_t add_lsb8 = lsb8(x) + lsb8(y);
	uint16_t add_msb8 = msb8(x) + msb8(y) + msb8(add_lsb8);
	// merge the result of the disjoint operations
	uint16_t merge_add = merge8(add_lsb8, add_msb8);
	// assign the result of addition to the value of the ALU operation
	result->value = merge_add;
	
	M_EXIT_IF_ERR(update_flags(&result->flags, merge_add, 0, lsb4(x) + lsb4(y), msb8(add_lsb8)));
	
	// return with no errors
	return ERR_NONE;
}

/*
 * Since the first grading, we lost points on alu_add16_high for both the addition part and the flags computationsparts with no explanations.
 * We acknowledge that our flags computation was wrong, but we do not understand why we lost points for the addition part as the addition result hould
 * be the same as in the alu_add16_low case. Could you please explain your decision?
 */

// ==== see alu.h ========================================
int alu_add16_high(alu_output_t* result, uint16_t x, uint16_t y) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	
	// divide 16 bit addition into two times 8 bit additions, i.e. lsb8 and msb8
	uint16_t add_lsb8 = lsb8(x) + lsb8(y);
	uint16_t add_msb8 = msb8(x) + msb8(y) + msb8(add_lsb8);
	// merge the result of the disjoint operations
	uint16_t merge_add = merge8(add_lsb8, add_msb8);
	// assign the result of addition to the value of the ALU operation
	result->value = merge_add;
	
	M_EXIT_IF_ERR(update_flags(&result->flags, merge_add, 0, lsb4(msb8(x))+lsb4(msb8(y))+msb8(add_lsb8), msb8(add_msb8)));
	
	// return with no errors
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_shift(alu_output_t* result, uint8_t x, rot_dir_t dir) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(dir == LEFT || dir == RIGHT, ERR_BAD_PARAMETER, "input direction (%d) should be LEFT (0) or RIGHT (1)", (int)dir);

	// define all flags to be 0 and set those which need to be modified
	flags_t flags = 0;
	
	switch (dir) {

		case LEFT:	// if msb bit is 1 set the carry
					if(bit_get(x, 7) == 1) set_C(&flags);
					x = x << 1;
					break;
		
		case RIGHT:	// if lsb bit is 1 set the carry
					if(bit_get(x, 0) == 1) set_C(&flags);
					x = x >> 1;
					break;
	}
	// check after the shift whether the result is 0
	if(x == 0) set_Z(&flags);
	
	// assign the result of addition to the value of the ALU operation
	result->value = x;
	// assign the flags of addition to the flags of the ALU operation
	result->flags = flags;
	
	// return with no errors
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_shiftR_A(alu_output_t* result, uint8_t x) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	M_EXIT_IF_ERR(alu_shift(result, x, RIGHT));
	// check the msb bit to decide the sign of the arithmetic shift
	// in case of negative value modify the 8th bit of the value of ALU
	if (bit_get(x, 7) == 1) result->value |= (1 << (BIT_8 - 1));
	
	// return with no errors
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(dir == LEFT || dir == RIGHT, ERR_BAD_PARAMETER, "input direction (%d) should be LEFT (0) or RIGHT (1)", (int)dir);
	
	// define all flags to be 0 and set those which need to be modified
	flags_t flags = 0;
	
	// check if carry occures in both directions
	switch (dir) {
		
		case LEFT:	if(bit_get(x, 7) == 1) set_C(&flags);
					break;
		
		case RIGHT:	if(bit_get(x, 0) == 1) set_C(&flags);
					break;
	}

	// rotate the value x by to the corresponding direction
	bit_rotate(&x, dir, 1);

	// check after rotation whether the result is 0
	if (x == 0) set_Z(&flags);
	
	// assign the result of addition to the value of the ALU operation
	result->value = x;
	// assign the flags of addition to the flags of the ALU operation
	result->flags = flags;
	
	// return with no errors
	return ERR_NONE;
}

// ==== see alu.h ========================================
int alu_carry_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir, flags_t flags) {
	// check argument validity
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(dir == LEFT || dir == RIGHT, ERR_BAD_PARAMETER, "input direction (%d) should be LEFT (0) or RIGHT (1)", (int)dir);
	M_REQUIRE(lsb4(flags) == 0, ERR_BAD_PARAMETER, "input flags (%u) should have the form bxxxx0000", flags);
	
	// define all flags to be 0 and set those which need to be modified
	flags_t flags_o = 0;
	bit_t carry = 0;
	
	// check if carry occures in both directions
	switch (dir) {
		case LEFT:	if(bit_get(x, (BIT_8 - 1)) == 1) set_C(&flags_o);
					carry = (get_C(flags) == 0) ? 0 : 1;
					x = (x << 1) | carry;
					break;
		
		case RIGHT:	if(bit_get(x, 0) == 1) set_C(&flags_o);
					x = (x >> 1);
					bit_edit(&x, (BIT_8 - 1), get_C(flags));
					break;
	}	
	// check whether the result is 0
	if (x == 0) set_Z(&flags_o);
	
	// assign the result of addition to the value of the ALU operation
	result->value = x;
	// assign the flags of addition to the flags of the ALU operation
	result->flags = flags_o;
	
	// return with no errors
	return ERR_NONE;
}
