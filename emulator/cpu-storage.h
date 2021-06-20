#pragma once

/**
 * @file cpu-storage.h
 * @brief CPU model for PPS-GBemul project, storage part
 *
 * @author J.-C. Chappelier & C. Hölzl, EPFL
 * @date 2019
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "memory.h"
#include "opcode.h"
#include "cpu.h"

#include "timer.h"

#include "error.h"
#include "cpu-registers.h" // cpu_BC_get
#include "gameboy.h" // REGISTER_START
#include "util.h"
#include <inttypes.h> // PRIX8
#include <stdio.h> // fprintf



/**
 * @brief Reads data from the bus at a given adress
 *
 * @param cpu cpu to read from
 * @param addr address to read at
 *
 * @return data read
 */
data_t cpu_read_at_idx(const cpu_t* cpu, addr_t addr);

/**
 * @brief Reads data at HL address from bus
 */
#define cpu_read_at_HL(cpu) \
    cpu_read_at_idx(cpu, cpu_HL_get(cpu))
    
// Self-made Macros for cpu_read_at

#define cpu_read_at_BC(cpu) \
    cpu_read_at_idx(cpu, cpu_BC_get(cpu))

#define cpu_read_at_DE(cpu) \
    cpu_read_at_idx(cpu, cpu_DE_get(cpu))

#define cpu_read_at_AF(cpu) \
    cpu_read_at_idx(cpu, cpu_AF_get(cpu))

#define cpu_read_at_A(cpu) \
    cpu_read_at_idx(cpu, cpu_A_get(cpu))

#define cpu_read_at_B(cpu) \
    cpu_read_at_idx(cpu, cpu_B_get(cpu))

#define cpu_read_at_C(cpu) \
    cpu_read_at_idx(cpu, cpu_C_get(cpu))

#define cpu_read_at_D(cpu) \
    cpu_read_at_idx(cpu, cpu_D_get(cpu))

#define cpu_read_at_E(cpu) \
    cpu_read_at_idx(cpu, cpu_E_get(cpu))

#define cpu_read_at_H(cpu) \
    cpu_read_at_idx(cpu, cpu_H_get(cpu))

#define cpu_read_at_L(cpu) \
    cpu_read_at_idx(cpu, cpu_L_get(cpu))
    

/**
 * @brief Reads data after opcode from bus
 */
#define cpu_read_data_after_opcode(cpu)\
    cpu_read_at_idx(cpu,(addr_t)((cpu)->PC + 1))

/**
 * @brief Reads 16bit data from the bus at a given adress
 *
 * @param cpu cpu to read from
 * @param addr address to read at
 *
 * @return data16 read
 */
addr_t cpu_read16_at_idx(const cpu_t* cpu, addr_t addr);

/**
 * @brief Reads 16bit data after opcode from bus
 */
#define cpu_read_addr_after_opcode(cpu) \
    FROM_GameBoy_16(cpu_read16_at_idx(cpu, (addr_t)((cpu)->PC + 1)))

/**
 * @brief Write data to the bus at a given adress
 *
 * @param cpu cpu to write to
 * @param addr address to write at
 * @param data data to write
 *
 * @return error code
 */
int cpu_write_at_idx(cpu_t* cpu, addr_t addr, data_t data);

#define cpu_write_at_HL(cpu, data) \
    cpu_write_at_idx(cpu, cpu_HL_get(cpu), data)
    
    // Self-made Macros for cpu_write_at

#define cpu_write_at_BC(cpu, data) \
    cpu_write_at_idx(cpu, cpu_BC_get(cpu), data)

#define cpu_write_at_DE(cpu, data) \
    cpu_write_at_idx(cpu, cpu_DE_get(cpu), data)

#define cpu_write_at_AF(cpu, data) \
    cpu_write_at_idx(cpu, cpu_AF_get(cpu), data)

#define cpu_write_at_A(cpu, data) \
    cpu_write_at_idx(cpu, cpu_A_get(cpu), data)

#define cpu_write_at_B(cpu, data) \
    cpu_write_at_idx(cpu, cpu_B_get(cpu), data)

#define cpu_write_at_C(cpu, data) \
    cpu_write_at_idx(cpu, cpu_C_get(cpu), data)

#define cpu_write_at_D(cpu, data) \
    cpu_write_at_idx(cpu, cpu_D_get(cpu), data)

#define cpu_write_at_E(cpu, data) \
    cpu_write_at_idx(cpu, cpu_E_get(cpu), data)

#define cpu_write_at_H(cpu, data) \
    cpu_write_at_idx(cpu, cpu_H_get(cpu), data)

#define cpu_write_at_L(cpu, data) \
    cpu_write_at_idx(cpu, cpu_L_get(cpu), data)

/**
 * @brief Write 16bit data to the bus at a given adress
 *
 * @param cpu cpu to write to
 * @param addr address to write at
 * @param data16 16bit data to write
 *
 * @return error code
 */
int cpu_write16_at_idx(cpu_t* cpu, addr_t addr, addr_t data16);

/**
 * @brief Executes a cpu storage instruction
 * @param lu instruction
 * @param cpu, the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
int cpu_dispatch_storage(const instruction_t* lu, cpu_t* cpu);


/**
 * @brief Push 16bit data to the stack
 *
 * @param cpu cpu to use
 * @param data16 16bit data to push to stack
 *
 * @return error code
 */
int cpu_SP_push(cpu_t* cpu, addr_t data16);


/**
 * @brief Pop 16bit data from the stack
 *
 * @param cpu cpu to use
 *
 * @return 16bit poped data
 */
addr_t cpu_SP_pop(cpu_t* cpu);


#ifdef __cplusplus
}
#endif
