CXX=gcc

CXXFLAGS=-Wall -Wextra -Werror -pedantic -fstack-protector-all -pedantic -std=c99
SANITY_FLAGS=-Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

COMMON_DIR=src/common/

ifneq ($(OS),Windows_NT)	
	arch := $(shell uname -m)
	ifeq ($(arch), x86_64)
		SRC_DIR=src/x86/
		SOURCE += $(COMMON_DIR)main.c $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)udev.c $(SRC_DIR)uarch.c $(COMMON_DIR)printer.c $(COMMON_DIR)args.c $(COMMON_DIR)global.c
		HEADERS += $(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)udev.h $(SRC_DIR)uarch.h $(COMMON_DIR)ascii.h $(COMMON_DIR)printer.h $(COMMON_DIR)args.h $(COMMON_DIR)global.h
		CXXFLAGS += -D_ARCH_X86
	else
		SRC_DIR=src/arm/
		SOURCE += $(COMMON_DIR)main.c $(SRC_DIR)cpuid.c $(SRC_DIR)uarch.c $(COMMON_DIR)printer.c $(COMMON_DIR)args.c $(COMMON_DIR)global.c 
		HEADERS += $(COMMON_DIR)ascii.h $(SRC_DIR)uarch.h $(SRC_DIR)cpuid.h $(COMMON_DIR)printer.h $(COMMON_DIR)args.h $(COMMON_DIR)global.h 
		CXXFLAGS += -D_ARCH_ARM -Wno-unused-parameter
	endif
		
	OUTPUT=cpufetch	
else
	# Assume x86_64
	SRC_DIR=src/x86/
	SOURCE += $(COMMON_DIR)main.c $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)udev.c $(SRC_DIR)uarch.c $(COMMON_DIR)printer.c $(COMMON_DIR)args.c $(COMMON_DIR)global.c
	HEADERS += $(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)udev.h $(SRC_DIR)uarch.h $(COMMON_DIR)ascii.h $(COMMON_DIR)printer.h $(COMMON_DIR)args.h $(COMMON_DIR)global.h
	CXXFLAGS += -D_ARCH_X86
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
