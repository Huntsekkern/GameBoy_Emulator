/**
 * @file bus.c
 * @brief Game Boy Bus Emulator
 *
 * @author S. Horvath-Mikulas & R. Gerber
 * @date 2020
 */

#include "bus.h"

// ==== see bus.h ========================================
int bus_plug (bus_t bus, component_t* c, addr_t start, addr_t end) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE(start <= end, ERR_ADDRESS ,"end address %u is smaller than start address %u", start, end);
	// check whether there is any component plugged to the predefined interval
	for (int i = start; i <= end; i++) {
		M_REQUIRE(bus[i] == NULL, ERR_ADDRESS, "one part of memory already occupied at address %u to %u", start, end);
	}
	
	// if no error occured, plug the component to the bus wo any offset
	return bus_forced_plug(bus, c, start, end, 0);
}


// ==== see bus.h ========================================
int bus_forced_plug (bus_t bus, component_t* c, addr_t start, addr_t end, addr_t offset) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE(start <= end, ERR_ADDRESS ,"end address %u is smaller than start address %u", start, end);

	// update the component start and end fields to indicate its position on the bus
	c->start = start;
	c->end = end;
	int err = bus_remap(bus, c, offset);
	if(err != 0) {
		bus_unplug(bus, c);
	}
	return err;
}


// ==== see bus.h ========================================
int bus_remap(bus_t bus, component_t* c, addr_t offset) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c->mem);
	M_REQUIRE_NON_NULL(c->mem->memory);  
	
	M_REQUIRE(c->end + offset < c->mem->size + c->start, ERR_ADDRESS, "end - start + offset is bigger or equal than memory size %d", c->mem->size);

	// map part of the memory of the component (shifted by an offset) to the corresponding bus address (memory map)
	for (int i = 0; i < c->end - c->start + 1; i++) {
		bus[c->start + i] = &(c->mem->memory[offset + i]);
	}
	
	return ERR_NONE;
}

// ==== see bus.h ========================================
int bus_unplug(bus_t bus, component_t* c) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(c);
	
	// deconnect the component from the bus
	for(int i = c->start; i <= c->end; ++i) {
		bus[i] = NULL;
	}

	// update the bus related fields of the component
	// start and end being 0 indicates the disconnection
	c->start = 0;
	c->end = 0;
	
	return ERR_NONE;
}

// ==== see bus.h ========================================
int bus_read(const bus_t bus, addr_t address, data_t* data) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(data);
	
	if (bus[address] == NULL) {
		// if no component is pluged to this address, put 0xFF to data
		*data = 0xFF;
	} else {
		*data = *bus[address];
	}

	return ERR_NONE;
}


// ==== see bus.h ========================================
int bus_write(bus_t bus, addr_t address, data_t data) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE(bus[address] != NULL, ERR_BAD_PARAMETER, "address %d non-valide", address);
	
	*bus[address] = data;
	
	return ERR_NONE;
}

// ==== see bus.h ========================================
int bus_read16(const bus_t bus, addr_t address, addr_t* data16) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(data16);
	M_REQUIRE(address != 0xFFFF, ERR_ADDRESS, "address %d can't be 0xFFFF, because there's only 8bits left read to then", address);
	
	if (bus[address] == NULL || bus[address+1] == NULL) {
		// if no component is pluged to this address, put 0xFF to data
		*data16 = 0xFF;
	} else {
		*data16 = (*bus[address] | (*bus[address+1] << 8));
	}

	return ERR_NONE;
}

// ==== see bus.h ========================================
int bus_write16(bus_t bus, addr_t address, addr_t data16) {
	// check argument validity
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE(bus[address] != NULL, ERR_ADDRESS, "address %d non-valide", address);
	M_REQUIRE(bus[address+1] != NULL, ERR_ADDRESS, "address %d non-valide", address);
	
	*bus[address] = lsb8(data16);
	*bus[address+1] = msb8(data16);
	
	return ERR_NONE;
}
