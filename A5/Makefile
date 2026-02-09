GCC=gcc
GPP=g++
FLAGS=-Wall -pthread -g -Wextra -Wconversion
TITLE=a5_malloc
BUILD_DIR=build

.PHONY : all clean runall

SRCS_CPP=$(wildcard tests/*.cpp)
EXECUTABLES_CPP=$(SRCS_CPP:.cpp= )

SRCS_C=$(wildcard tests/*.c)
EXECUTABLES_C=$(SRCS_C:.c= )

# make
all: malloc new ${EXECUTABLES_CPP} ${EXECUTABLES_C}

malloc: util.cpp
	mkdir -p $(BUILD_DIR)
	$(GPP) -c -fno-exceptions $(FLAGS) util.cpp -o $(BUILD_DIR)/malloc.o
	$(GPP) -c -D NEW_ENABLED $(FLAGS) util.cpp -o $(BUILD_DIR)/malloc_new.o

new: new.cpp
	$(GPP) -c $(FLAGS) new.cpp -o $(BUILD_DIR)/new.o

${EXECUTABLES_CPP}:
	$(GPP) $(FLAGS) $(BUILD_DIR)/malloc_new.o $(BUILD_DIR)/new.o $(@).cpp -o $(subst tests/,$(BUILD_DIR)/,$(@))

${EXECUTABLES_C}:
	$(GCC) $(FLAGS) $(BUILD_DIR)/malloc.o $(@).c -o $(subst tests/,$(BUILD_DIR)/,$(@))

runall:
	./run_tests.sh

# make clean
clean :
	rm -r ./$(BUILD_DIR)
