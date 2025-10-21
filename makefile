target := bin/main
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
TESTS=$(wildcard $(TEST)/*.c)
TESTSBINS=$(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))

CC := gcc
# CFLAGS := -Wall -O3
CFLAGS := -Wall
LFLAGS :=
ZIPNAME := project.zip
BIN := bin
OBJ := obj
SRC := src
TEST := tests

$(shell mkdir -p obj bin)

all: $(target) $(BIN) $(OBJ)

debug: CFLAGS = -Wall -g
debug: clean
debug: $(target)

$(target): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN)/* $(OBJ)/* $(TEST)/bin/*

zip: 
	rm -f $(ZIPNAME)
	zip $(ZIPNAME) $(SRC)/*


test: $(target) $(TESTSBINS)
	for test in $(TESTSBINS) ; do ./$$test --verbose ; done


$(TEST)/bin/%: $(TEST)/%.c obj/file.o
	$(CC) $(CFLAGS) $< obj/file.o -o $@ -lcriterion


run: $(target)
	./$(target)

cleanrun: clean
cleanrun: run
