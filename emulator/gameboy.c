/**
 * @file gameboy.c
 * @brief Gameboy Header for GameBoy Emulator
 *
 * @author S. Horvath-Mikulas & R. Gerber
 * @date 2020
 */

#include "gameboy.h"
#include "bootrom.h"

/**
 * Auxiliary function
 * @brief Prints out the characters sent to the serial port of the gameboy
 *
 * @param gameboy pointer to gameboy
 * @param addr address to listen to whether there has been a writing to
 * @return error message
 */
#ifdef BLARGG
	#include "cpu-storage.h"
	static int blargg_bus_listener(gameboy_t* gameboy, addr_t addr);
#endif


#define component_setup(index, name) \
	M_EXIT_IF_ERR(component_create( &(gameboy->components[index]), MEM_SIZE(name))); \
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &(gameboy->components[index]), name ## _START, name ## _END))

// ==== see gameboy.h ========================================
int gameboy_create(gameboy_t* gameboy, const char* filename) {
	// check arguments validity
	M_REQUIRE_NON_NULL(gameboy);
	M_REQUIRE_NON_NULL(filename);
	
	// start the booting state of the gameboy, i.e. activate the bootrom
	gameboy->boot = 1;
	// initialize its components, bus and cycle fields to null
	memset(gameboy->components, 0, GB_NB_COMPONENTS*sizeof(component_t));
	memset(gameboy->bus, 0, BUS_SIZE*sizeof(data_t*));
	gameboy->cycles = 1;
	
	// create and plug its work_RAM component
	component_setup(0, WORK_RAM);
	
	// create and plug echo_RAM and share work_RAM's memory with it 
	component_t echo_RAM = {NULL, 0, 0};
	M_EXIT_IF_ERR(component_shared(&echo_RAM, &(gameboy->components[0])));
	M_EXIT_IF_ERR(bus_plug(gameboy->bus, &echo_RAM, ECHO_RAM_START, ECHO_RAM_END));

	// create and plug its video_RAM component
	component_setup(1, VIDEO_RAM);

	// create and plug its extern_RAM component
	component_setup(2, EXTERN_RAM);

	// create and plug its graph_RAM component
	component_setup(3, GRAPH_RAM);

	// create and plug its registers component
	component_setup(4, REGISTERS);

	// create and plug its useless component
	component_setup(5, USELESS);
	
	//create and plug its cpu component
	cpu_t* cpu = &(gameboy->cpu);
	M_EXIT_IF_ERR(cpu_init(cpu));
	M_EXIT_IF_ERR(cpu_plug(cpu, &gameboy->bus));
	
	//initialize its timer
	gbtimer_t* timer = &(gameboy->timer);
	M_EXIT_IF_ERR(timer_init(timer, &gameboy->cpu));
	
	// create its cartridge component
	// It will be replugged at the same time the bootrom is disabled (see bootrom_bus_listener in bootrom.c).
	cartridge_t* cartridge = &(gameboy->cartridge);
	M_EXIT_IF_ERR(cartridge_init(cartridge, filename));
	M_EXIT_IF_ERR(cartridge_plug(&(gameboy->cartridge), gameboy->bus));

	// create and plug its bootrom component
	component_t* bootrom = &(gameboy->bootrom);
	M_EXIT_IF_ERR(bootrom_init(bootrom));
	M_EXIT_IF_ERR(bootrom_plug(bootrom, gameboy->bus));
	
	//init and plug its screen
	M_EXIT_IF_ERR(lcdc_init(gameboy));
	M_EXIT_IF_ERR(lcdc_plug(&(gameboy->screen), gameboy->bus));
	
	//init and plug its joypad
	M_EXIT_IF_ERR(joypad_init_and_plug(&(gameboy->pad), cpu));

	return ERR_NONE;
}

// ==== see gameboy.h ========================================
void gameboy_free(gameboy_t* gameboy) {
	// check arguments validity
	if (gameboy != NULL) {
		//free and unplug each of the 6 components of the gameboy
		for(int i = 0; i < GB_NB_COMPONENTS; ++i) {
			bus_unplug(gameboy->bus, &(gameboy->components[i]));
			component_free(&(gameboy->components[i]));
		}
		//free the cartridge and the cpu
		bus_unplug(gameboy->bus, &gameboy->bootrom);
		component_free(&gameboy->bootrom);
		bus_unplug(gameboy->bus, &(gameboy->cartridge.c));
		cartridge_free(&gameboy->cartridge);
		cpu_free(&gameboy->cpu);
		lcdc_free(&gameboy->screen);
	} 
}


// ==== see gameboy.h ========================================
int gameboy_run_until(gameboy_t* gameboy, uint64_t cycle) {
	// check arguments validity
	M_REQUIRE_NON_NULL(gameboy);

	for (uint64_t i = gameboy->cycles; i < cycle; i++) {

		M_EXIT_IF_ERR(timer_cycle(&gameboy->timer));
		M_EXIT_IF_ERR(cpu_cycle(&gameboy->cpu));
		M_EXIT_IF_ERR(lcdc_cycle(&gameboy->screen, i));

		
		//call each listener
		M_EXIT_IF_ERR(timer_bus_listener(&gameboy->timer, gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(bootrom_bus_listener(gameboy, gameboy->cpu.write_listener));
		#ifdef BLARGG
			M_EXIT_IF_ERR(blargg_bus_listener(gameboy, gameboy->cpu.write_listener));
		#endif
		M_EXIT_IF_ERR(lcdc_bus_listener(&gameboy->screen, gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(joypad_bus_listener(&gameboy->pad, gameboy->cpu.write_listener));


		
		gameboy->cycles++;

        #ifdef BLARGG_EARLY
			if (((gameboy->cycles)%17556) == 0) {
				cpu_request_interrupt(&gameboy->cpu, VBLANK);
			}
		#endif
		
	}
	
	return ERR_NONE;
}

#ifdef BLARGG
	static int blargg_bus_listener(gameboy_t* gameboy, addr_t addr)
	{
		// check arguments validity
		M_REQUIRE_NON_NULL(gameboy);
		
		if(addr == BLARGG_REG) {
			printf("%c", cpu_read_at_idx(&(gameboy->cpu), BLARGG_REG));
		}
		return ERR_NONE;
	}
#endif
