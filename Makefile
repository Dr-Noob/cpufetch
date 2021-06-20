CC ?= gcc

CFLAGS+=-D_FORTIFY_SOURCE=2 -Wall -Wextra -pedantic -fstack-protector-all -pedantic -std=c99
SANITY_FLAGS=-Wfloat-equal -Wshadow -Wpointer-arith

PREFIX ?= /usr

SRC_COMMON=src/common/

COMMON_SRC = $(SRC_COMMON)main.c $(SRC_COMMON)cpu.c $(SRC_COMMON)udev.c $(SRC_COMMON)printer.c $(SRC_COMMON)args.c $(SRC_COMMON)global.c
COMMON_HDR = $(SRC_COMMON)ascii.h $(SRC_COMMON)cpu.h $(SRC_COMMON)udev.h $(SRC_COMMON)printer.h $(SRC_COMMON)args.h $(SRC_COMMON)global.h

ifneq ($(OS),Windows_NT)
	arch := $(shell uname -m)
	ifeq ($(arch), $(filter $(arch), x86_64 amd64 i686))
		SRC_DIR=src/x86/
		SOURCE += $(COMMON_SRC) $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)uarch.c
		HEADERS += $(COMMON_HDR) $(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)uarch.h
		CFLAGS += -DARCH_X86
	else
		SRC_DIR=src/arm/
		SOURCE += $(COMMON_SRC) $(SRC_DIR)midr.c $(SRC_DIR)uarch.c $(SRC_DIR)soc.c $(SRC_DIR)udev.c
		HEADERS += $(COMMON_HDR) $(SRC_DIR)midr.h $(SRC_DIR)uarch.h  $(SRC_DIR)soc.h $(SRC_DIR)udev.c $(SRC_DIR)socs.h
		CFLAGS += -DARCH_ARM -Wno-unused-parameter
	endif

	OUTPUT=cpufetch
else
	# Assume x86_64
	SRC_DIR=src/x86/
	SOURCE += $(COMMON_SRC) $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)uarch.c
	HEADERS += $(COMMON_HDR) $(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)uarch.h
	CFLAGS += -DARCH_X86
	SANITY_FLAGS += -Wno-pedantic-ms-format
	OUTPUT=cpufetch.exe
endif

all: CFLAGS += -O3
all: $(OUTPUT)

debug: CFLAGS += -g -O0
debug: $(OUTPUT)

static: CFLAGS += -static -O3
static: $(OUTPUT)

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CC) $(CFLAGS) $(SANITY_FLAGS) $(SOURCE) -o $(OUTPUT)

run: $(OUTPUT)
	./$(OUTPUT)

clean:
	@rm -f $(OUTPUT)

install: $(OUTPUT)
	install -Dm755 "cpufetch"   "$(DESTDIR)$(PREFIX)/bin/cpufetch"
	install -Dm644 "LICENSE"    "$(DESTDIR)$(PREFIX)/share/licenses/cpufetch-git/LICENSE"
	install -Dm644 "cpufetch.1" "$(DESTDIR)$(PREFIX)/share/man/man1/cpufetch.1.gz"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/cpufetch"
	rm -f "$(DESTDIR)$(PREFIX)/share/licenses/cpufetch-git/LICENSE"
	rm -f "$(DESTDIR)$(PREFIX)/share/man/man1/cpufetch.1.gz"
