CXX=gcc

CXXFLAGS=-g

SOURCE=main.c standart.c extended.c cpuid.c udev.c printer.c
HEADERS=standart.h extended.h cpuid.h udev.h printer.h ascii.h

OUTPUT=cpufetch

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(OUTPUT)

run:
	./$(OUTPUT)
clean:
	@rm $(OUTPUT)
