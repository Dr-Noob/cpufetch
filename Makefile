CXX=gcc

CXXFLAGS=-g -Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused
SANITY_FLAGS=-Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

SOURCE=main.c standart.c extended.c cpuid.c udev.c printer.c args.c global.c
HEADERS=standart.h extended.h cpuid.h udev.h printer.h ascii.h args.h global.h

OUTPUT=cpufetch

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SANITY_FLAGS) $(SOURCE) -o $(OUTPUT)

run:
	./$(OUTPUT)
clean:
	@rm $(OUTPUT)
