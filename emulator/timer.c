/**
 * @file timer.c
 * @brief Game Boy Timer simulation
 *
 * @author S. Horvath-Mikulas & R. Gerber
 * @date 2020
 */

#include "timer.h"
#include "cpu-storage.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Auxiliary function
 * @brief Timer state checker
 *
 * @param timer timer
 * @return a bit that indicates whether the secondary timer should be modified
 */
bit_t timer_state(gbtimer_t*timer);

/**
 * Auxiliary function
 * @brief Incrementation of the secondary timer, TIMER interrupt
 *
 * @param timer timer
 * @param old_state previous state of the timer
 */
void timer_incr_if_state_change(gbtimer_t* timer, bit_t old_state);


// =========================================================
bit_t timer_state(gbtimer_t* timer) {
	// check arguments validity
	if(timer != NULL && timer->cpu != NULL) {
		// read the content of the TAC, configuration register for the secondary counter 
		data_t reg_tac = cpu_read_at_idx(timer->cpu, REG_TAC);
		int index = -1;
		// based on the two least significant bit, determine which bit of the primary counter should we listen to
		switch (reg_tac & 0x11) {
			case 0: index = 9;
					break;
			case 1: index = 3;
					break;
			case 2: index = 5;
					break;
			case 3: index = 7;
					break;
		}
		// check whether the secondary counter is activated (TAC 3rd lsb bit) and
		// whether the corresponding bit of the primary counter is active
		// if both are active return 1 else 0
		return (bit_get(reg_tac, 2) & (bit_t)((timer->counter & (1 << index)) >> index));
	} else {
		return 0;
	}
}

// =========================================================
void timer_incr_if_state_change(gbtimer_t* timer, bit_t old_state) {
	// check arguments validity
    if(timer != NULL && (old_state == 0 || old_state == 1)) {
		bit_t new_state = timer_state(timer);
		// if the old state switches from 1 to 0 increment the counter
		if ((old_state == 1) && (new_state == 0)) {
			if (cpu_read_at_idx(timer->cpu, REG_TIMA) == 0xFF) {
				cpu_write_at_idx(timer->cpu, REG_TIMA,  (cpu_read_at_idx(timer->cpu, REG_TMA)));
				cpu_request_interrupt(timer->cpu, TIMER);
			} else {
				cpu_write_at_idx(timer->cpu, REG_TIMA,  (cpu_read_at_idx(timer->cpu, REG_TIMA) + 1));
			} 
		}		
	 }
} 

// ==== see timer.h ========================================
int timer_init(gbtimer_t* timer, cpu_t* cpu) {
	// check arguments validity
    M_REQUIRE_NON_NULL(timer);
    M_REQUIRE_NON_NULL(cpu);

	// initialize the timer fields
    timer->cpu = cpu;
    timer->counter = 0;

	//M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_TAC, 4));

    return ERR_NONE;
} 

// ==== see timer.h ========================================
int timer_cycle(gbtimer_t* timer){
	// check arguments validity
    M_REQUIRE_NON_NULL(timer);

	// check the timer state, increment the primery counter by 4 and
	// check whether the secondary counter should be incremented or TIMER interrupt should be raised
    bit_t state = timer_state(timer);
    timer->counter += 4;
	M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_DIV,  msb8(timer->counter)));
    timer_incr_if_state_change(timer, state);

    return ERR_NONE;
} 

// ==== see timer.h ========================================
int timer_bus_listener(gbtimer_t* timer, addr_t addr) {
	// check arguments validity
	M_REQUIRE_NON_NULL(timer);

	// if there is a writing access to the counter register, reset its value
	if(addr == REG_DIV) {
		bit_t state = timer_state(timer);
		timer->counter = 0;
		M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_DIV, 0));
		timer_incr_if_state_change(timer, state);
	} else if(addr == REG_TAC) {
		timer_incr_if_state_change(timer, timer_state(timer));
	}
	
	return ERR_NONE;
} 

#ifdef __cplusplus
}
#endif
