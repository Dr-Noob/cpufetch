CXX=gcc

CXXFLAGS=-Wall -Wextra -Werror -pedantic -fstack-protector-all -pedantic -std=c99
SANITY_FLAGS=-Wfloat-equal -Wshadow -Wpointer-arith

SRC_COMMON=src/common/

COMMON_SRC = $(SRC_COMMON)main.c $(SRC_COMMON)cpu.c $(SRC_COMMON)udev.c $(SRC_COMMON)printer.c $(SRC_COMMON)args.c $(SRC_COMMON)global.c
COMMON_HDR = $(SRC_COMMON)ascii.h $(SRC_COMMON)cpu.h $(SRC_COMMON)udev.h $(SRC_COMMON)printer.h $(SRC_COMMON)args.h $(SRC_COMMON)global.h

ifneq ($(OS),Windows_NT)
	arch := $(shell uname -m)
	ifeq ($(arch), x86_64)
		SRC_DIR=src/x86/
		SOURCE += $(COMMON_SRC) $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)uarch.c
		HEADERS += $(COMMON_HDR) $(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)uarch.h 
		CXXFLAGS += -DARCH_X86
	else
		SRC_DIR=src/arm/
		SOURCE += $(COMMON_SRC) $(SRC_DIR)midr.c $(SRC_DIR)uarch.c $(SRC_DIR)soc.c $(SRC_DIR)udev.c
		HEADERS += $(COMMON_HDR) $(SRC_DIR)midr.h $(SRC_DIR)uarch.h  $(SRC_DIR)soc.h $(SRC_DIR)udev.c $(SRC_DIR)socs.h
		CXXFLAGS += -DARCH_ARM -Wno-unused-parameter
	endif

	OUTPUT=cpufetch
else
	# Assume x86_64
	SRC_DIR=src/x86/
	SOURCE += $(COMMON_SRC) $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)uarch.c
	HEADERS += $(COMMON_HDR) $(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)uarch.h 
	CXXFLAGS += -DARCH_X86
	SANITY_FLAGS += -Wno-pedantic-ms-format
	OUTPUT=cpufetch.exe
endif

all: $(OUTPUT)

debug: CXXFLAGS += -g -O0
debug: $(OUTPUT)

release: CXXFLAGS += -static -O3
release: $(OUTPUT)

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SANITY_FLAGS) $(SOURCE) -o $(OUTPUT)

run: $(OUTPUT)
	./$(OUTPUT)

clean:
	@rm $(OUTPUT)

install: $(OUTPUT)
	install -Dm755 "cpufetch"   "/usr/bin/cpufetch"
	install -Dm644 "LICENSE"    "/usr/share/licenses/cpufetch-git/LICENSE"
	install -Dm644 "cpufetch.8" "/usr/share/man/man8/cpufetch.8.gz"
