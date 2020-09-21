CXX=gcc

CXXFLAGS=-Wall -Wextra -Werror -pedantic -fstack-protector-all -pedantic -std=c99
SANITY_FLAGS=-Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

SRC_DIR=src/
SOURCE=$(SRC_DIR)main.c $(SRC_DIR)cpuid.c $(SRC_DIR)apic.c $(SRC_DIR)cpuid_asm.c $(SRC_DIR)printer.c $(SRC_DIR)args.c $(SRC_DIR)global.c $(SRC_DIR)uarch.c
HEADERS=$(SRC_DIR)cpuid.h $(SRC_DIR)apic.h $(SRC_DIR)cpuid_asm.h $(SRC_DIR)printer.h $(SRC_DIR)ascii.h $(SRC_DIR)args.h $(SRC_DIR)global.h $(SRC_DIR)uarch.h

ifneq ($(OS),Windows_NT)
	SOURCE += $(SRC_DIR)udev.c
	HEADERS += $(SRC_DIR)udev.h
	OUTPUT=cpufetch	
else
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

run:
	./$(OUTPUT)

clean:
	@rm $(OUTPUT)
	
install: $(OUTPUT)
	install -Dm755 "cpufetch"   "/usr/bin/cpufetch"
	install -Dm644 "LICENSE"    "/usr/share/licenses/cpufetch-git/LICENSE"
	install -Dm644 "cpufetch.8" "/usr/share/man/man8/cpufetch.8.gz"
