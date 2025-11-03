target := bin/main
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
TESTS=$(wildcard $(TEST)/*.c)
TESTSBINS=$(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))

CC := gcc
CFLAGS := -Wall -O3 -lm
# CFLAGS := -Wall
LFLAGS := -lm
ZIPNAME := project.zip
BIN := bin
OBJ := obj
SRC := src
TEST := tests

# AddressSanitizer flags (used by the 'mem' target)
ASAN_CFLAGS := -fsanitize=address -g -O1 -fno-omit-frame-pointer
ASAN_LFLAGS := -fsanitize=address

$(shell mkdir -p obj bin tests/bin)

all: $(target) $(BIN) $(OBJ)
	@echo "Tip: use 'make -j' for faster builds"

debug: CFLAGS = -Wall -g
debug: clean
debug: $(target)

# Build+run with AddressSanitizer
# Appends ASAN flags (including -O1) so the final -O option used is -O1.
mem: CFLAGS += $(ASAN_CFLAGS)
mem: LFLAGS += $(ASAN_LFLAGS)
mem: clean $(target)
	./$(target)

$(target): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN)/* $(OBJ)/* $(TEST)/bin/*

zip: 
	rm -f $(ZIPNAME)
	zip $(ZIPNAME) $(SRC)/*


test: $(TESTSBINS)
	for test in $(TESTSBINS) ; do ./$$test --verbose ; done


$(TEST)/bin/%: $(TEST)/%.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(filter-out obj/main.o, $^) -lcriterion $(LFLAGS)


run: $(target)
	./$(target)

cleanrun: clean
cleanrun: run
