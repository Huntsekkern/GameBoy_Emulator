/**
 * @file cartridge.c
 * @brief Game Boy Cartridge simulation
 *
 * @author S. Horvath-Mikulas & R. Gerber
 * @date 2020
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cartridge.h"

// ==== see cartridge.h ========================================
int cartridge_init_from_file(component_t* c, const char* filename) {
    // check arguments validity
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(c->mem->memory);
    M_REQUIRE_NON_NULL(filename);

    // open the file that stores the cartride content
    FILE* input = NULL;
    input = fopen(filename, "r");
    M_REQUIRE_NON_NULL_CUSTOM_ERR(input, ERR_IO);

    // read the file content and check for requirements
    size_t amount_bytes = fread(c->mem->memory, c->mem->size, sizeof(data_t), input);
	M_REQUIRE(amount_bytes > 0, ERR_IO, "the amount of bytes read should be strictly positive, but was %i", amount_bytes);
	M_REQUIRE(amount_bytes <= c->mem->size, ERR_IO, "the amount of bytes read should be <= than bank_ROM_size(%i), but was %i", c->mem->size, amount_bytes);
    M_REQUIRE(c->mem->memory[147] == 0, ERR_NOT_IMPLEMENTED, "cartridge type read at address 0x147 is %i, but only 0 is accepted ", i);

    fclose(input);

    return ERR_NONE;
}

// ==== see cartridge.h ========================================
int cartridge_init(cartridge_t* ct, const char* filename) {
    // check arguments validity
    M_REQUIRE_NON_NULL(ct);
    M_REQUIRE_NON_NULL(filename);

    // create the component and
    // initialize its memory field with its content loaded from a file
    component_t* bank_ROM = &ct->c;
	M_EXIT_IF_ERR(component_create(bank_ROM, BANK_ROM_SIZE));
    M_EXIT_IF_ERR(cartridge_init_from_file(bank_ROM, filename));

    return ERR_NONE;
}

// ==== see cartridge.h ========================================
int cartridge_plug(cartridge_t* ct, bus_t bus) {
    // check arguments validity
    M_REQUIRE_NON_NULL(ct);

    // force the bus plug (bootrom is present at the lower address space)
    M_EXIT_IF_ERR(bus_forced_plug(bus, &(ct->c), BANK_ROM0_START, BANK_ROM1_END, 0));

    return ERR_NONE;
}

// ==== see cartridge.h ========================================
void cartridge_free(cartridge_t* ct) {
    // check arguments validity
    if (ct != NULL) {
        component_free(&(ct->c));
    }
}

#ifdef __cplusplus
}
#endif
