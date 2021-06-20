/**
 * @file component.c
 * @brief Game Boy Component Simulation
 *
 * @author S. Horvath-Mikulas & R.Gerber
 * @date 2020
 */

#include "component.h"

// ==== see component.h ========================================
int component_create(component_t* c, size_t mem_size) {
	// check argument validity
	M_REQUIRE_NON_NULL(c);
	// double check for overflow (built in calloc)
	M_REQUIRE(mem_size < SIZE_MAX / sizeof(data_t), ERR_MEM, "input memory size (%d) exceeds max_mem_size (%d)", (int)mem_size, (int) SIZE_MAX / sizeof(data_t));
	
	// initialization of the component structure
	component_t result = {NULL, 0, 0};
	
	// if the size of memory is 0, the component does not have memory,
	// i.e. its mem pointer is NULL
	if (mem_size != 0) {
		// dynamic allocation in the heap for the memory structure
		result.mem = calloc(1, sizeof(memory_t));
		// in case a problem occured quit with error code
		M_REQUIRE_NON_NULL(result.mem);
		// dynamic allocation in the heap for the memory
		M_EXIT_IF_ERR(mem_create(result.mem, mem_size));
	}
	
	result.start = 0;
	result.end = 0;
	*c = result;

	return ERR_NONE;
}

// ==== see component.h ========================================
int component_shared(component_t* c, component_t* c_old) {
	// check argument validity
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c_old);
	M_REQUIRE_NON_NULL(c_old->mem);

	// deconnect c from the bus
	c->start = 0;
	c->end = 0;
	// c shares the same memory space as c_old
	c->mem = c_old->mem;
	
	return ERR_NONE;
}

// ==== see component.h ========================================
void component_free(component_t* c) {
	// check argument validity
	if(c != NULL) {
		c->start = 0;
		c->end = 0;
		// free up the memory (space in heap for memory and its size to 0)
		mem_free(c->mem);
		// free up the component
		free(c->mem);
		c->mem = NULL;
	}
}
