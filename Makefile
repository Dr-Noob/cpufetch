CXX=gcc

CXXFLAGS=-g

SOURCE=main.c 01h.c 02h.c extended.c cpuid.c udev.c
HEADERS=01h.c 02h.h extended.h cpuid.h udev.h printer.h

OUTPUT=cpufetch

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(OUTPUT)

run:
	./$(OUTPUT)
clean:
	@rm $(OUTPUT)
