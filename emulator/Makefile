## ======================================================================
## partial Makefile provided to students
##

GTK_INCLUDE := `pkg-config --cflags gtk+-3.0`
GTK_LIBS := `pkg-config --libs gtk+-3.0`

.PHONY: clean new style feedback submit1 submit2 submit

CFLAGS += -std=c11 -Wall -pedantic -g  

# a bit more checks if you'd like to (uncomment
# CFLAGS += -Wextra -Wfloat-equal -Wshadow                         \
# -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings \
# -Wconversion -Wunreachable-code

# uncomment if you want to add DEBUG flag
# CPPFLAGS += -DDEBUG

# uncomment if you want to add BLARGG flag
CPPFLAGS += -DBLARGG

# ----------------------------------------------------------------------
# feel free to update/modifiy this part as you wish

# all those libs are required on Debian, feel free to adapt it to your box
LDLIBS += -lcheck -lm -lrt -pthread -lsubunit 
#-lcs212gbfinalext

# As we didn't get an answer on the forum, we decided to go with "make" compiling but not executing the unit-test. 
# To execute them all at once after the "make", you can call "make check".

TARGETS := test-cpu-week08 test-cpu-week09 test-gameboy gbsimulator unit-test-alu unit-test-bit unit-test-bit-vector unit-test-bus unit-test-cartridge unit-test-component unit-test-cpu unit-test-cpu-dispatch-week08 unit-test-cpu-dispatch-week09 unit-test-memory unit-test-timer
CHECK_TARGETS := unit-test-alu unit-test-bit unit-test-bit-vector unit-test-bus unit-test-cartridge unit-test-component unit-test-cpu unit-test-cpu-dispatch-week08 unit-test-cpu-dispatch-week09 unit-test-memory unit-test-timer

all:: $(TARGETS)

OBJS = alu.o bit.o bus.o memory.o component.o
OBJS_NO_STATIC_TESTS =
OBJS_STATIC_TESTS = 
OBJS = $(OBJS_STATIC_TESTS) $(OBJS_NO_STATIC_TESTS)


# We decided to individually add the library in order to not include it for earlier tests
unit-test-cpu: LDFLAGS += -L.
unit-test-cpu: LDLIBS += -lcs212gbfinalext
unit-test-cpu-dispatch-week08: LDFLAGS += -L.
unit-test-cpu-dispatch-week08: LDLIBS += -lcs212gbfinalext
unit-test-cpu-dispatch-week09: LDFLAGS += -L.
unit-test-cpu-dispatch-week09: LDLIBS += -lcs212gbfinalext
unit-test-timer: LDFLAGS += -L.
unit-test-timer: LDLIBS += -lcs212gbfinalext

# We split the BLARGG flag in two, so that the LCDC-using gbsimulator do not run the artificial VBLANK interrupts
test-cpu-week08: LDFLAGS += -L.
test-cpu-week08: LDLIBS += -lcs212gbfinalext
test-cpu-week08: CPPFLAGS += -DBLARGG_EARLY
# test-cpu-week09 do not reach the correct result after the 5308 cycles stated in the instruction.
# but it does reach the correct result after 5740 cycles.
test-cpu-week09: LDFLAGS += -L.
test-cpu-week09: LDLIBS += -lcs212gbfinalext
test-cpu-week09: CPPFLAGS += -DBLARGG_EARLY
test-gameboy: LDFLAGS += -L.
test-gameboy: LDLIBS += -lcs212gbfinalext
test-gameboy: CPPFLAGS += -DBLARGG_EARLY
#test-image: GTK_INCLUDE := `pkg-config --cflags gtk+-3.0`
#test-image: GTK_LIBS := `pkg-config --libs gtk+-3.0`
test-image: CFLAGS += $(GTK_INCLUDE)
gbsimulator: CFLAGS += $(GTK_INCLUDE)
gbsimulator: LDLIBS += -lcs212gbfinalext
gbsimulator: LDLIBS += $(GTK_LIBS)
gbsimulator: LDFLAGS += -L.

unit-test-bit: unit-test-bit.o bit.o
unit-test-alu: unit-test-alu.o alu.o bit.o error.o
unit-test-bus: unit-test-bus.o bus.o component.o bit.o memory.o
unit-test-component: unit-test-component.o bus.o memory.o component.o bit.o
unit-test-memory: unit-test-memory.o bus.o memory.o component.o error.o bit.o
unit-test-cpu: unit-test-cpu.o error.o alu.o bit.o util.o cpu.o bus.o memory.o component.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o bit_vector.o image.o
unit-test-cpu-dispatch-week08: unit-test-cpu-dispatch-week08.o bus.o cpu-storage.o cpu-registers.o cpu-alu.o component.o bit.o alu.o memory.o opcode.o gameboy.o bootrom.o cartridge.o timer.o bit_vector.o image.o error.o
unit-test-cpu-dispatch-week09: unit-test-cpu-dispatch-week09.o cpu-storage.o cpu-registers.o cpu-alu.o bit.o alu.o bus.o component.o opcode.o memory.o timer.o bootrom.o cartridge.o bit_vector.o image.o error.o
unit-test-cartridge: unit-test-cartridge.o cartridge.o component.o bus.o memory.o bit.o
unit-test-timer: unit-test-timer.o timer.o bit.o cpu.o cpu-storage.o opcode.o bus.o cpu-registers.o component.o memory.o alu.o cpu-alu.o bit_vector.o image.o error.o
unit-test-bit-vector: unit-test-bit-vector.o bit_vector.o

test-cpu-week08: test-cpu-week08.o gameboy.o opcode.o error.o bus.o cpu.o component.o cpu-storage.o cpu-registers.o cpu-alu.o bit.o alu.o memory.o timer.o bootrom.o cartridge.o bit_vector.o image.o
test-cpu-week09: test-cpu-week09.o gameboy.o opcode.o error.o bus.o cpu.o component.o cpu-storage.o cpu-registers.o cpu-alu.o bit.o alu.o memory.o timer.o bootrom.o cartridge.o bit_vector.o image.o
test-gameboy: test-gameboy.o gameboy.o bus.o component.o cartridge.o timer.o bootrom.o cpu.o bit.o memory.o cpu-storage.o cpu-registers.o opcode.o cpu-alu.o alu.o error.o bit_vector.o image.o
test-image: test-image.o image.o bit_vector.o sidlib.o
	gcc $^ $(GTK_INCLUDE) $(GTK_LIBS) -o $@
gbsimulator: gbsimulator.o gameboy.o bus.o component.o cartridge.o timer.o bootrom.o cpu.o bit.o cpu-storage.o cpu-registers.o memory.o opcode.o cpu-alu.o alu.o image.o bit_vector.o libsid.so error.o
	gcc $(LDFLAGS) $^ $(LDLIBS) $(CFLAGS) -o $@

unit-test-alu_ext: unit-test-alu_ext.o cpu-storage.o cpu-registers.o cpu-alu.o alu.o bus.o bit.o error.o -lcs212gbcpuext -lcheck -lm -lrt  -lsubunit 
#-pthread
unit-test-cpu-dispatch: unit-test-cpu-dispatch.o cpu-storage.o cpu-registers.o cpu-alu.o opcode.o alu.o component.o memory.o bus.o bit.o error.o -lcs212gbcpuext  -lcheck -lm -lrt -lsubunit
#-pthread



alu.o: alu.c alu.h bit.h error.h
bit.o: bit.c bit.h
bit_vector.o: bit_vector.c bit_vector.h bit.h image.h
bit_vector\ (OG).o: bit_vector\ (OG).c bit_vector.h bit.h image.h
bootrom.o: bootrom.c bootrom.h bus.h memory.h error.h component.h bit.h \
 gameboy.h cpu.h alu.h cartridge.h timer.h lcdc.h image.h bit_vector.h \
 joypad.h
bus.o: bus.c bus.h memory.h error.h component.h bit.h
cartridge.o: cartridge.c cartridge.h component.h memory.h error.h bus.h \
 bit.h
component.o: component.c component.h memory.h error.h
cpu-alu.o: cpu-alu.c error.h bit.h alu.h cpu-alu.h opcode.h cpu.h bus.h \
 memory.h component.h cpu-storage.h timer.h cpu-registers.h gameboy.h \
 cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h
cpu.o: cpu.c cpu.h alu.h bit.h error.h bus.h memory.h component.h \
 opcode.h cpu-alu.h cpu-registers.h cpu-storage.h timer.h gameboy.h \
 cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h
cpu-registers.o: cpu-registers.c cpu-registers.h cpu.h alu.h bit.h \
 error.h bus.h memory.h component.h
cpu-storage.o: cpu-storage.c cpu-storage.h memory.h error.h opcode.h \
 bit.h cpu.h alu.h bus.h component.h timer.h cpu-registers.h gameboy.h \
 cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h
error.o: error.c
gameboy.o: gameboy.c gameboy.h bus.h memory.h error.h component.h bit.h \
 cpu.h alu.h cartridge.h timer.h lcdc.h image.h bit_vector.h joypad.h \
 bootrom.h
gbsimulator.o: gbsimulator.c sidlib.h lcdc.h cpu.h alu.h bit.h error.h \
 bus.h memory.h component.h image.h bit_vector.h gameboy.h cartridge.h \
 timer.h joypad.h
image.o: image.c error.h image.h bit_vector.h bit.h
libsid_demo.o: libsid_demo.c sidlib.h
memory.o: memory.c memory.h error.h
opcode.o: opcode.c opcode.h bit.h
sidlib.o: sidlib.c sidlib.h
test-cpu-week08.o: test-cpu-week08.c opcode.h bit.h cpu.h alu.h error.h \
 bus.h memory.h component.h cpu-storage.h timer.h cpu-registers.h \
 gameboy.h cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h
test-cpu-week09.o: test-cpu-week09.c opcode.h bit.h cpu.h alu.h error.h \
 bus.h memory.h component.h cpu-storage.h timer.h cpu-registers.h \
 gameboy.h cartridge.h lcdc.h image.h bit_vector.h joypad.h util.h
test-gameboy.o: test-gameboy.c gameboy.h bus.h memory.h error.h \
 component.h bit.h cpu.h alu.h cartridge.h timer.h lcdc.h image.h \
 bit_vector.h joypad.h util.h
test-image.o: test-image.c error.h util.h image.h bit_vector.h bit.h \
 sidlib.h
timer.o: timer.c timer.h component.h memory.h error.h bit.h cpu.h alu.h \
 bus.h cpu-storage.h opcode.h cpu-registers.h gameboy.h cartridge.h \
 lcdc.h image.h bit_vector.h joypad.h util.h
unit-test-alu.o: unit-test-alu.c tests.h error.h alu.h bit.h
unit-test-alu_ext.o: unit-test-alu_ext.c tests.h error.h alu.h bit.h \
 alu_ext.h
unit-test-bit.o: unit-test-bit.c tests.h error.h bit.h
unit-test-bit-vector.o: unit-test-bit-vector.c tests.h error.h \
 bit_vector.h bit.h image.h
unit-test-bus.o: unit-test-bus.c tests.h error.h bus.h memory.h \
 component.h bit.h util.h
unit-test-cartridge.o: unit-test-cartridge.c tests.h error.h cartridge.h \
 component.h memory.h bus.h bit.h cpu.h alu.h
unit-test-component.o: unit-test-component.c tests.h error.h bus.h \
 memory.h component.h bit.h
unit-test-cpu.o: unit-test-cpu.c tests.h error.h alu.h bit.h opcode.h \
 util.h cpu.h bus.h memory.h component.h cpu-registers.h cpu-storage.h \
 timer.h gameboy.h cartridge.h lcdc.h image.h bit_vector.h joypad.h \
 cpu-alu.h
unit-test-cpu-dispatch.o: unit-test-cpu-dispatch.c tests.h error.h alu.h \
 bit.h cpu.h bus.h memory.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h \
 timer.h gameboy.h cartridge.h lcdc.h image.h bit_vector.h joypad.h
unit-test-cpu-dispatch-week08.o: unit-test-cpu-dispatch-week08.c tests.h \
 error.h alu.h bit.h cpu.h bus.h memory.h component.h opcode.h gameboy.h \
 cartridge.h timer.h lcdc.h image.h bit_vector.h joypad.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h
unit-test-cpu-dispatch-week09.o: unit-test-cpu-dispatch-week09.c tests.h \
 error.h alu.h bit.h cpu.h bus.h memory.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h \
 timer.h gameboy.h cartridge.h lcdc.h image.h bit_vector.h joypad.h
unit-test-memory.o: unit-test-memory.c tests.h error.h bus.h memory.h \
 component.h bit.h
unit-test-timer.o: unit-test-timer.c util.h tests.h error.h timer.h \
 component.h memory.h bit.h cpu.h alu.h bus.h
util.o: util.c





# ----------------------------------------------------------------------
# This part is to make your life easier. See handouts how to make use of it.


clean::
	-@/bin/rm -f *.o *~ $(CHECK_TARGETS)

new: clean all

static-check:
	scan-build -analyze-headers --status-bugs -maxloop 64 make CC=clang new

style:
	astyle -n -o -A8 -xt0 *.[ch]

# all those libs are required on Debian, adapt to your box
$(CHECK_TARGETS): LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

check:: $(CHECK_TARGETS)
	$(foreach target,$(CHECK_TARGETS),./$(target) &&) true

# target to run tests
check:: all
	@if ls tests/*.*.sh 1> /dev/null 2>&1; then \
	  for file in tests/*.*.sh; do [ -x $$file ] || echo "Launching $$file"; ./$$file || exit 1; done; \
	fi

IMAGE=chappeli/feedback:latest
feedback:
	@docker pull $(IMAGE)
	@docker run -it --rm -v ${PWD}:/home/tester/done $(IMAGE)

SUBMIT_SCRIPT=../provided/submit.sh
submit1: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 1

submit2: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 2

submit:
	@printf 'what "make submit"??\nIt'\''s either "make submit1" or "make submit2"...\n'

libsid.so: sidlib.c
	$(CC) -fPIC -shared $(CPPFLAGS) $(CFLAGS) $(GTK_INCLUDE) $^ -o $@

libsid_demo.o: CFLAGS += $(GTK_INCLUDE)

libsid_demo: LDFLAGS += -L.
libsid_demo: LDLIBS += $(GTK_LIBS) -lsid
libsid_demo: libsid_demo.o libsid.so
