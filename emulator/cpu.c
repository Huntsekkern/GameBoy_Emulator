/**
 * @file cpu.c
 * @brief Game Boy CPU simulation
 *
 * @date 2019
 */


#include "cpu.h"

#include "error.h"
#include "opcode.h"
#include "cpu-alu.h"
#include "cpu-registers.h"
#include "cpu-storage.h"
#include "util.h"
#include "bit.h"

#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf

bit_t check_cc(const instruction_t* lu, cpu_t* cpu);

// ==== see cpu.h =======================================================
int cpu_init(cpu_t* cpu)
{
	// check argument validity
	M_REQUIRE_NON_NULL(cpu);
	// init all values, flags, registers, etc. to 0 / NULL
	cpu->alu.flags = 0;
	cpu->alu.value = 0;
	for(int i = 0; i < numbers_of_reg_pairs; ++i) {
		cpu_reg_pair_set(cpu, i, 0);
	}
	cpu->PC = 0;
	cpu->SP = 0;
	cpu->IE = 0;
	cpu->IF = 0;
	cpu->IME = 0;
	cpu->HALT = 0;
	cpu->idle_time = 0;
	cpu->bus = NULL;
	cpu->write_listener = 0;
	
	component_t* high_ram = &cpu->high_ram;
	// Contrary to what was written in the feedback, we do need the +1 here because we want to include REG_IE within the high_ram space
	M_EXIT_IF_ERR(component_create(high_ram, HIGH_RAM_SIZE+1));
    return ERR_NONE;
}

// ==== see cpu.h =======================================================
int cpu_plug(cpu_t* cpu, bus_t* bus)
{
	// check arguments validity
	M_REQUIRE_NON_NULL(cpu);
	M_REQUIRE_NON_NULL(bus);
	cpu->bus = bus;
    M_EXIT_IF_ERR(bus_plug(*(cpu->bus), &(cpu->high_ram), HIGH_RAM_START, HIGH_RAM_END+1));
    (*(cpu->bus))[REG_IE] = &cpu->IE;
    (*(cpu->bus))[REG_IF] = &cpu->IF;
    return ERR_NONE;
}

// ==== see cpu.h =======================================================
void cpu_free(cpu_t* cpu)
{
	// check argument validity
	if(cpu != NULL) {
		bus_unplug(*(cpu->bus), &(cpu->high_ram));
		component_free(&(cpu->high_ram));
		cpu->bus = NULL;
	}
}

// ---------------------------------------------------------------------
bit_t check_cc(const instruction_t* lu, cpu_t* cpu) {
		data_t cc = extract_cc(lu->opcode);
		bit_t act = 0;

		switch (cc) {
			case 0: if(get_Z(cpu->F) == 0) act = 1;
			break;
			case 1: if(get_Z(cpu->F) != 0) act = 1;
			break;
			case 2: if(get_C(cpu->F) == 0) act = 1;
			break;
			case 3: if(get_C(cpu->F) != 0) act = 1;
			break;
		} // switch
		return act;
}

//=========================================================================
/**
 * @brief Executes an instruction
 * @param lu instruction
 * @param cpu, the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
static int cpu_dispatch(const instruction_t* lu, cpu_t* cpu)
{
	// check arguments validity
    M_REQUIRE_NON_NULL(lu);
    M_REQUIRE_NON_NULL(cpu);
    
	//reset to 0 the ALU of the CPU (flags and value)
	cpu->alu.flags = 0;
	cpu->alu.value = 0;
	
	// execute the instruction lu based on the switch
    switch (lu->family) {

    // ALU
    case ADD_A_HLR:
    case ADD_A_N8:
    case ADD_A_R8:
    case INC_HLR:
    case INC_R8:
    case ADD_HL_R16SP:
    case INC_R16SP:
    case SUB_A_HLR:
    case SUB_A_N8:
    case SUB_A_R8:
    case DEC_HLR:
    case DEC_R8:
    case DEC_R16SP:
    case AND_A_HLR:
    case AND_A_N8:
    case AND_A_R8:
    case OR_A_HLR:
    case OR_A_N8:
    case OR_A_R8:
    case XOR_A_HLR:
    case XOR_A_N8:
    case XOR_A_R8:
    case CPL:
    case CP_A_HLR:
    case CP_A_N8:
    case CP_A_R8:
    case SLA_HLR:
    case SLA_R8:
    case SRA_HLR:
    case SRA_R8:
    case SRL_HLR:
    case SRL_R8:
    case ROTCA:
    case ROTA:
    case ROTC_HLR:
    case ROT_HLR:
    case ROTC_R8:
    case ROT_R8:
    case SWAP_HLR:
    case SWAP_R8:
    case BIT_U3_HLR:
    case BIT_U3_R8:
    case CHG_U3_HLR:
    case CHG_U3_R8:
    case LD_HLSP_S8:
    case DAA:
    case SCCF:
        M_EXIT_IF_ERR(cpu_dispatch_alu(lu, cpu));
        // update cpu->PC with lu->bytes 
		cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
        break;

    // STORAGE
    case LD_A_BCR:
    case LD_A_CR:
    case LD_A_DER:
    case LD_A_HLRU:
    case LD_A_N16R:
    case LD_A_N8R:
    case LD_BCR_A:
    case LD_CR_A:
    case LD_DER_A:
    case LD_HLRU_A:
    case LD_HLR_N8:
    case LD_HLR_R8:
    case LD_N16R_A:
    case LD_N16R_SP:
    case LD_N8R_A:
    case LD_R16SP_N16:
    case LD_R8_HLR:
    case LD_R8_N8:
    case LD_R8_R8:
    case LD_SP_HL:
    case POP_R16:
    case PUSH_R16:
        M_EXIT_IF_ERR(cpu_dispatch_storage(lu, cpu));
        // update cpu->PC with lu->bytes 
		cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
        break;


	// Most of the control instructions substract lu->bytes only because 
    // JUMP
    case JP_CC_N16: {
		if(check_cc(lu, cpu)) {
            cpu->PC = cpu_read_addr_after_opcode(cpu);
			cpu->idle_time = (uint8_t) (cpu->idle_time + lu->xtra_cycles);
		} else {
			cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
		}
        break;
	}

    case JP_HL:
		cpu->PC = cpu_HL_get(cpu);
        break;

    case JP_N16:
		cpu->PC = cpu_read_addr_after_opcode(cpu);
        break;

    case JR_CC_E8: {
		if(check_cc(lu, cpu)) {
			// cast to int because interpreted in 2-complement
			cpu->PC = (uint16_t) (cpu->PC + lu->bytes + (int8_t) cpu_read_data_after_opcode(cpu));
			cpu->idle_time = (uint8_t) (cpu->idle_time + lu->xtra_cycles);
		} else {
			cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
		}
        break;
	}

    case JR_E8:
    	// cast to int because interpreted in 2-complement
		cpu->PC = (uint16_t) ((cpu->PC + lu->bytes + (int8_t) cpu_read_data_after_opcode(cpu)));
        break;


    // CALLS
    case CALL_CC_N16:
    	if(check_cc(lu, cpu)) {
			cpu_SP_push(cpu, (uint16_t) (cpu->PC + lu->bytes));
			cpu->PC = cpu_read_addr_after_opcode(cpu);
			cpu->idle_time = (uint8_t )(cpu->idle_time + lu->xtra_cycles);
		} else {
			cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
		}
        break;

    case CALL_N16:
		cpu_SP_push(cpu, (uint16_t) (cpu->PC + lu->bytes));
		cpu->PC = cpu_read_addr_after_opcode(cpu);
        break;


    // RETURN (from call)
    case RET:
		cpu->PC = cpu_SP_pop(cpu);
        break;

    case RET_CC:
        if(check_cc(lu, cpu)) {
			cpu->PC = cpu_SP_pop(cpu);
            cpu->idle_time = (uint8_t) (cpu->idle_time + lu->xtra_cycles);
		} else {
			cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
		}
        break;

    case RST_U3:
    	cpu_SP_push(cpu, (uint16_t) (cpu->PC + lu->bytes));
    	cpu->PC = (extract_reg(lu->opcode, 3) << 3);
        break;


    // INTERRUPT & MISC.
    case EDI:
		cpu->IME = extract_ime(lu->opcode);
		cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
        break;

    case RETI:
		cpu->IME = 1;
		cpu->PC = cpu_SP_pop(cpu);
        break;

    case HALT:
		cpu->HALT = 1;
		cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
        break;

    case STOP:
    case NOP:
        // ne rien faire
        cpu->PC = (uint16_t) (cpu->PC + lu->bytes);
        break;

    default: {
        fprintf(stderr, "Unknown instruction, Code: 0x%" PRIX8 "\n", cpu_read_at_idx(cpu, cpu->PC));
        return ERR_INSTR;
    } break;

    } // end of switch
    
    // update cpu->idle_time with lu->cycles
    cpu->idle_time = (uint8_t) (cpu->idle_time + lu->cycles);

    return ERR_NONE;
}





/**
 * @brief Check for interrupts and if there's none, pass on the instruction to dispatch
 * @param cpu (modified), the CPU which shall run
 * @return error code
 *
 * See cpu.h
 */
// ----------------------------------------------------------------------
static int cpu_do_cycle(cpu_t* cpu)
{
	// check arguments validity
    M_REQUIRE_NON_NULL(cpu);

    // In case of interrupt, change the PC accordingly
    if(cpu->IME && (cpu->IE & cpu->IF)) {
		cpu->IME = 0;
		uint8_t upcoming_interrupt = INTERRUPT_COUNT;
		for(uint8_t i = 0; i < INTERRUPT_COUNT; ++i) {
			if(bit_get((cpu->IE & cpu->IF), i)) {
				upcoming_interrupt = i;
				// We decided to use cpu_write here instead of CPU->IF because cpu_write directly updates the write_listener as needed.
				uint8_t old_reg_IF = (cpu_read_at_idx(cpu, REG_IF));
				bit_unset(&old_reg_IF, i);
				M_EXIT_IF_ERR(cpu_write_at_idx(cpu, REG_IF, old_reg_IF));
				break;
			}
		}
		cpu_SP_push(cpu, cpu->PC);
		cpu->PC = (uint16_t) (0x40 + (upcoming_interrupt << 3));
		cpu->idle_time = (uint8_t) (cpu->idle_time + 5);
		
		// Otherwise, do the normal procedure
	} else {
		// read the opcode and transform it into an instruction to pass it as an argument
		data_t first_byte = cpu_read_at_idx(cpu, cpu->PC);
		if(first_byte != PREFIXED) {
			M_EXIT_IF_ERR(cpu_dispatch(&instruction_direct[first_byte], cpu));
		} else  {
			// if the first byte was a prefix, then dispatch according to the second byte
			M_EXIT_IF_ERR(cpu_dispatch(&instruction_prefixed[cpu_read_data_after_opcode(cpu)], cpu));
		}
	}
	
    return ERR_NONE;
}

// ======================================================================
/**
 * See cpu.h
 */
int cpu_cycle(cpu_t* cpu)
{
	// check arguments validity
    M_REQUIRE_NON_NULL(cpu);
    M_REQUIRE_NON_NULL(cpu->bus);
    
	// reset the write_listener to only listen once to each write
	cpu->write_listener = 0;
	
	if(cpu->idle_time > 0) {
		cpu->idle_time = (uint8_t) (cpu->idle_time - 1);
		return ERR_NONE;
	} 

	// if the gameboy is paused, but an interrupt for which we're actively listening is raised, restart
    if(cpu->HALT == 1 && (cpu->IE & cpu->IF) != 0 && cpu->idle_time == 0) {
		cpu->HALT = 0;	
	}
	 
	if(cpu->HALT == 0 && cpu->idle_time == 0) {
		M_EXIT_IF_ERR(cpu_do_cycle(cpu));
		cpu->idle_time = (uint8_t) (cpu->idle_time - 1);
	}

    return ERR_NONE;
}

// ======================================================================
/**
 * See cpu.h
 */
void cpu_request_interrupt(cpu_t* cpu, interrupt_t i) {
	// We decided to use cpu_write here instead of CPU->IF because cpu_write directly updates the write_listener as needed.
	// However, as the timer unit-test do not call cpu_plug, this particular unit-test doesn't link bus[REG_IF] with CPU->IF, 
	// so the unit-test fails when checking for CPU.IF
	// We then added the cpu->IF line because it do no harm to the regular operation but still allows to pass the unit-test-timer
	cpu_write_at_idx(cpu, REG_IF, (uint8_t) (1 << i));
	cpu->IF = (uint8_t) (1 << i);
}
