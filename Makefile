CXX=gcc

CXXFLAGS=-g

SOURCE=main.c 01h.c 02h.c extended.c cpuid.c
HEADERS=01h.c 02h.h extended.h cpuid.h

OUTPUT=cpufetch

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(OUTPUT)

clean:
	@rm $(OUTPUT1) $(OUTPUT2)
