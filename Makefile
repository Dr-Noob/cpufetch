CXX=gcc

CXXFLAGS=-g -Wall -Werror -fstack-protector-all -pedantic -Wno-unused

SOURCE=main.c standart.c extended.c cpuid.c udev.c printer.c args.c global.c
HEADERS=standart.h extended.h cpuid.h udev.h printer.h ascii.h args.h global.h

OUTPUT=cpufetch

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(OUTPUT)

run:
	./$(OUTPUT)
clean:
	@rm $(OUTPUT)
