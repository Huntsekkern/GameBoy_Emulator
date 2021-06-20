/**
 * @file memory.c
 * @brief Memory for Gamemu
 *
 * @author S. Horvath-Mikulas & R. Gerber
 * @date 2020
 */
 
#include "memory.h"

// ==== see memory.h ========================================
int mem_create(memory_t* mem, size_t size) {
	// check argument validity
	M_REQUIRE_NON_NULL(mem);
	M_REQUIRE(size > 0, ERR_BAD_PARAMETER, "size can't be %u", size);
	// double check for overflow (built in calloc)
	M_REQUIRE(size <= SIZE_MAX / sizeof(size_t), ERR_MEM, "size (%u) must be < than %u", , size, SIZE_MAX / sizeof(size_t));

	// initialization of the memory structure
	memory_t result = {NULL, 0};
	
	// dynamic allocation in the heap for the memory
	result.memory = calloc(size, sizeof(data_t));
	// in case a problem occured quit with error code
	M_REQUIRE_NON_NULL(result.memory);
	result.size = size;
	*mem = result;

	return ERR_NONE;
}

// ==== see memory.h ========================================
void mem_free(memory_t* mem) {
	// check argument validity
	if (mem != NULL) {
		// free up the space in the heap reserved for the memory
		free(mem->memory);
		mem->memory = NULL;
		mem->size = 0;
	}
}
