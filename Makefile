CXX=gcc

CXXFLAGS=-g -Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused
SANITY_FLAGS=-Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

SRC_DIR=src/
SOURCE=$(SRC_DIR)main.c $(SRC_DIR)standart.c $(SRC_DIR)extended.c $(SRC_DIR)cpuid.c $(SRC_DIR)udev.c $(SRC_DIR)printer.c $(SRC_DIR)args.c $(SRC_DIR)global.c
HEADERS=$(SRC_DIR)standart.h $(SRC_DIR)extended.h $(SRC_DIR)cpuid.h $(SRC_DIR)udev.h $(SRC_DIR)printer.h $(SRC_DIR)ascii.h $(SRC_DIR)args.h $(SRC_DIR)global.h

OUTPUT=cpufetch

$(OUTPUT): Makefile $(SOURCE) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SANITY_FLAGS) $(SOURCE) -o $(OUTPUT)

run:
	./$(OUTPUT)
clean:
	@rm $(OUTPUT)
