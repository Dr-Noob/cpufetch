CXX=gcc

CXXFLAGS=-g

MAIN=main.c

OUTPUT=cpufetch

$(OUTPUT): Makefile $(MAIN) 02h.c 02h.h
	$(CXX) $(CXXFLAGS) $(MAIN) 02h.c -o $(OUTPUT)

clean:
	@rm $(OUTPUT1) $(OUTPUT2)
