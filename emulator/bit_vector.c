/**
 * @file bit_vector.c
 * @brief Bit vectors for PPS projects
 *
 * @author 
 * @date 2020
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>

#include "bit_vector.h"
#include "image.h"

#define ROUND32(x) (x % IMAGE_LINE_WORD_BITS == 0 ? x : (x + (IMAGE_LINE_WORD_BITS - x%IMAGE_LINE_WORD_BITS)))
#define WORD_NB(size) ROUND32(size)/IMAGE_LINE_WORD_BITS
#define MOD32(index) index % IMAGE_LINE_WORD_BITS
#define QUOTIENT32(index) index / IMAGE_LINE_WORD_BITS
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

//=========================================================================
bit_vector_t* bit_vector_create(size_t size, bit_t value) {
	// check arguments validity
	if (size <= 0) {return NULL;}
	if (size > SIZE_MAX / sizeof(uint32_t)) {return NULL;}
	if (value != 0 && value != 1) {return NULL;}
	
	bit_vector_t* result = malloc(sizeof(bit_vector_t) + sizeof(uint32_t) * (WORD_NB(size) - 1));

	// if the malloc was successful, set-up the vector according to the parameters
	if (result != NULL) {
		result->size = size;
		memset(result->content, 0, sizeof(uint32_t) * (WORD_NB(size)));
		if (value == 1) {
			for (size_t i = 0; i < WORD_NB(size) - 1; i++) {
				result->content[i] = UINT32_MAX;
			}
			result->content[WORD_NB(size) - 1] = (MOD32(size) == 0) ? UINT32_MAX :(uint32_t) ((1 << (MOD32(size))) - 1);
		} 
  	}
	return result;
}

//=========================================================================
bit_vector_t* bit_vector_cpy(const bit_vector_t* pbv) {
	// check arguments validity
	if(pbv == NULL) {
		return NULL;
	} else {
		// init a new vector
		bit_vector_t* result = bit_vector_create(pbv->size, 0);
		// if successful, fill it with the argument content 
		if(result != NULL) {
			for(size_t i = 0; i < WORD_NB(pbv->size); ++i) {
				result->content[i] = pbv->content[i];
			}
		}
		return result;
	}
}

//=========================================================================
bit_t bit_vector_get(const bit_vector_t* pbv, size_t index) {
	// check arguments validity
	if(pbv == NULL || pbv->content == NULL || index >= pbv->size) {
		return 0;
	} else {
		// Get the right word, shift it by the modulo of the index and mask it
		size_t modulo = MOD32(index); 
		size_t nbr = QUOTIENT32(index); 
		return (bit_t) ((pbv->content[nbr] >> modulo) & 1);
	}
}

//=========================================================================
bit_vector_t* bit_vector_not(bit_vector_t* pbv) {
	// check arguments validity
	if(pbv == NULL || pbv->content == NULL) {
		return NULL;
	} 
	// do the negation
	for(size_t i = 0; i < WORD_NB(pbv->size); ++i) {
		pbv->content[i] = ~pbv->content[i];
	}
	
	// put to 0 the msbs of the top word in case the size is not a multiple of 32
	if (MOD32(pbv->size) != 0) {
		pbv->content[WORD_NB(pbv->size) - 1] &= (uint32_t) ((1 << MOD32(pbv->size)) - 1); 
	}
	return pbv;
}

//=========================================================================
bit_vector_t* bit_vector_and(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
	// check arguments validity
	if(pbv1 == NULL || pbv1->content == NULL || pbv2 == NULL || pbv2->content == NULL || pbv1->size != pbv2->size) {
		return NULL;
	}
	
	for(size_t i = 0; i < WORD_NB(pbv1->size); ++i) {
		pbv1->content[i] &= pbv2->content[i];
	}
	return pbv1;
}

//=========================================================================
bit_vector_t* bit_vector_or(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
	// check arguments validity
	if(pbv1 == NULL || pbv1->content == NULL || pbv2 == NULL || pbv2->content == NULL || pbv1->size != pbv2->size) {
		return NULL;
	}

	for(size_t i = 0; i < WORD_NB(pbv1->size); ++i) {
		pbv1->content[i] |= pbv2->content[i];
	}
	return pbv1;
}

//=========================================================================
bit_vector_t* bit_vector_xor(bit_vector_t* pbv1, const bit_vector_t* pbv2) {
	// check arguments validity
	if(pbv1 == NULL || pbv1->content == NULL || pbv2 == NULL || pbv2->content == NULL || pbv1->size != pbv2->size) {
		return NULL;
	}
	
	for(size_t i = 0; i < WORD_NB(pbv1->size); ++i) {
		pbv1->content[i] ^= pbv2->content[i];
	}
	return pbv1;
}

//=========================================================================
/**
 * @brief Helper enum type for the extract functions
 */
typedef enum {
    zero = 0, 
	wrap = 1
} extract_type;

/**
 * @brief Extract only the group of 32 bits of the pbv containing the index passed as argument
 * @param pbv pointer to bit vector
 * @param typ type of the extraction (zero = 0 or wrap = 1)
 * @param index index from where to start extraction
 * @return uint32_t of the pbv containing the given index
 */
static uint32_t bit_vector_contains_index(const bit_vector_t* pbv, extract_type typ, int64_t index) {
	
	if(typ == zero && (index < 0 || index >= (int64_t) ROUND32(pbv->size)))		{ return 0; }
	if(typ == wrap && index >= (int64_t) ROUND32(pbv->size))					{ index = (index % (int64_t) pbv->size); }
	if(typ == wrap && index < 0) 												{ index = (((index % (int64_t) pbv->size) + (int64_t) pbv->size) % (int64_t) pbv->size); }

	return pbv->content[QUOTIENT32(index)];
}

//=========================================================================
bit_vector_t* bit_vector_extract_zero_ext(const bit_vector_t* pbv, int64_t index, size_t size) {
	
	// check the arguments: if size is 0 it returns the NULL vector
	if(size == 0) {
		return NULL;
	}
	// create a vector of zeros with size of bit-length
	bit_vector_t* result = bit_vector_create(size, 0);
	
	// check the arguments: if pbv is NULL it returns a vector with zeros
	if(pbv == NULL || pbv->content == NULL) {
		return result;
	}
	
	//special case if multiple of 32
	if(MOD32(index) == 0) {
		for(size_t i = 0; i < WORD_NB(size); ++i) {
			result->content[i] = bit_vector_contains_index(pbv, zero, index + (int64_t)i*IMAGE_LINE_WORD_BITS);
		}

	} else {
		// recreate the resulting vector by merging two adjacent words shifted as required
		int64_t shift_right = index > 0 ? MOD32(index) : (IMAGE_LINE_WORD_BITS + (MOD32(index)));
		int64_t shift_left = IMAGE_LINE_WORD_BITS - shift_right;
		for(size_t i = 0; i < WORD_NB(size); ++i) {
			result->content[i] = (bit_vector_contains_index(pbv, zero, index + (int64_t)i*IMAGE_LINE_WORD_BITS) >> shift_right) | (bit_vector_contains_index(pbv, zero, index + ((int64_t)i+1)*IMAGE_LINE_WORD_BITS) << shift_left);
		}		
	}

	return result;
}

//=========================================================================
bit_vector_t* bit_vector_extract_wrap_ext(const bit_vector_t* pbv, int64_t index, size_t size) {
	
	// check the arguments: if size is 0 it returns the NULL vector
	// check the arguments: if pbv is NULL it returns the NULL vector
	if(size == 0 || pbv == NULL || pbv->content == NULL) {
		return NULL;
	}

	// create a vector of zeros with size of bit-length
	bit_vector_t* result = bit_vector_create(size, 0);
	

	//special case if multiple of 32
	if(MOD32(index) == 0) {
		for(size_t i = 0; i < WORD_NB(size); ++i) {
			// if regular params
			if(size <= 32 && pbv->size >= size && MOD32(pbv->size) == 0 && MOD32(size) == 0) {
				result->content[i] = bit_vector_contains_index(pbv, wrap, index + (int64_t)i*IMAGE_LINE_WORD_BITS);
			} else { // with irregular params
				for(size_t j = 0; j < size/pbv->size; ++j) {
					result->content[i] = result->content[i] | bit_vector_contains_index(pbv, wrap, index + (int64_t)i*IMAGE_LINE_WORD_BITS) << j*pbv->size;
				}
			}
		}
	
	} else {
		// recreate the resulting vector by merging two adjacent words shifted as required
		int64_t shift_right = index > 0 ? MOD32(index) : (IMAGE_LINE_WORD_BITS + (MOD32(index)));
		int64_t shift_left = IMAGE_LINE_WORD_BITS - shift_right;
		for(size_t i = 0; i < WORD_NB(size); ++i) {
			result->content[i] = (bit_vector_contains_index(pbv, wrap, index + (int64_t)i*IMAGE_LINE_WORD_BITS) >> shift_right) | (bit_vector_contains_index(pbv, wrap, index + ((int64_t)i+1)*IMAGE_LINE_WORD_BITS) << shift_left);
		}		
	}
	
	return result;
}

//=========================================================================
bit_vector_t* bit_vector_shift(const bit_vector_t* pbv, int64_t shift) {
	// check arguments validity
	if(pbv == NULL) {
		return NULL;
	}
	return bit_vector_extract_zero_ext(pbv, -shift, pbv->size);
}

//=========================================================================
bit_vector_t* bit_vector_join(const bit_vector_t* pbv1, const bit_vector_t* pbv2, int64_t shift) {
	// check arguments validity
	if(pbv1 == NULL || pbv1->content == NULL || pbv2 == NULL || pbv2->content == NULL || pbv1->size != pbv2->size || shift < 0 || shift > (int64_t)pbv1->size) {
		return NULL;
	}
	
	// init a new vector and only proceed if successful
	bit_vector_t* result = bit_vector_create(pbv1->size, 0);
	if(result == NULL) { return NULL; }

	for(int i = 0; i < (int64_t) WORD_NB(pbv1->size); ++i) {
		if (i < QUOTIENT32(shift))  {
			// words entirely taken from pbv1
			result->content[i] = pbv1->content[i];
		} else if (i > QUOTIENT32(shift) || MOD32(shift) == 0)  {
			// words entirely taken from pbv2 (the second condition is tested only when i = QUOTIENT32(shift))
			result->content[i] = pbv2->content[i];
		} else {
			// middle word merged from both vectors (when i = QUOTIENT32(shift), but MOD32(shift) != 0)
			result->content[i] |= (pbv1->content[i] & (uint32_t) ((1 << MOD32(shift)) - 1));
			result->content[i] |= (pbv2->content[i] & ~(uint32_t)((1 << ((MOD32(shift)) -1)) - 1));
		}
	}
	return result;
}

//=========================================================================
int bit_vector_print(const bit_vector_t* pbv) {
	// check arguments validity
	if (pbv == NULL) {return 0;}
	// track all returns of printing functions
	int count = 0;
	
	// print all the bits from MSB to LSB
	for (size_t i = 0; i < pbv->size; i++) {
			count += printf("%u", bit_vector_get(pbv, pbv->size - 1 - i));
	}
	
	return count;
}

//=========================================================================
int bit_vector_println(const char* prefix, const bit_vector_t* pbv) {
	// check arguments validity
	if (pbv != NULL && prefix != NULL) {
		// add all returns of printing functions
		int count = printf("%s", prefix);
		count += bit_vector_print(pbv);
		count += printf("\n");
		return count;
	} else {
		return 0;
	}
}

//=========================================================================
void bit_vector_free(bit_vector_t** pbv) {
	// check argument validity
	if (pbv != NULL && *pbv != NULL) {
		// free up the space in the heap reserved for the pbv
		free((*pbv));
		*pbv = NULL;
	}
}

#ifdef __cplusplus
}
#endif
