/**
 * @file bootrom.c
 * @brief Game Boy Boot ROM
 *
 * @author S. Horvath-Mikulas & R. Gerber
 * @date 05/2020
 */


#include "bootrom.h"

// ==== see bootrom.h ========================================
int bootrom_init(component_t* c) {
	// check arguments validity
	M_REQUIRE_NON_NULL(c);

	// create a component for the bootrom
	M_EXIT_IF_ERR(component_create(c, MEM_SIZE(BOOT_ROM)));
	// feed the memory array allocated for the bootrom by its predefined content
	data_t data[MEM_SIZE(BOOT_ROM)] = GAMEBOY_BOOT_ROM_CONTENT;
	
	for (int i = 0; i < MEM_SIZE(BOOT_ROM); i++) {
		c->mem->memory[i] = data[i];
	}

	return ERR_NONE;
}

// ==== see bootrom.h ========================================
int bootrom_bus_listener(gameboy_t* gameboy, addr_t addr) {
	// check arguments validity
	M_REQUIRE_NON_NULL(gameboy);
	
	// whenever there is a writing access on the bus at the bootrom register
	// unplug the bus, delete the bootrom component and change the gameboy boot state
	if(gameboy->boot && (addr == REG_BOOT_ROM_DISABLE)) {
		M_EXIT_IF_ERR(cartridge_plug(&(gameboy->cartridge), gameboy->bus));
		gameboy->boot = 0;

		int j = 0;
		for (size_t i = 0; i <= 0xFFFF; i++)
		{
    		if (gameboy->bus[i] == NULL) j++;
		}

	}

	return ERR_NONE;
}

