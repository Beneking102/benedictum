CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SRC = src/benedictum.c
BIN = benedictum-cli
ifeq ($(OS),Windows_NT)
    EXE = $(BIN).exe
else
    EXE = ./$(BIN)
endif

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

debug: $(SRC)
	$(CC) -Wall -Wextra -g -std=c99 -o $(BIN) $(SRC)

install: $(BIN)
	cp $(BIN) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(BIN)

clean:
	rm -f $(BIN)

test: $(BIN)
	@echo "=== hello.ben ==="
	@$(EXE) benedictum/hello.ben
	@echo ""
	@echo "=== alphabet.ben ==="
	@$(EXE) benedictum/alphabet.ben
	@echo ""
	@echo "=== countdown.ben ==="
	@$(EXE) benedictum/countdown.ben
	@echo ""
	@echo "=== pax.ben ==="
	@$(EXE) benedictum/pax.ben
	@echo ""
	@echo "=== fibonacci.ben ==="
	@$(EXE) benedictum/fibonacci.ben
	@echo ""
	@echo "=== oracle.ben ==="
	@$(EXE) benedictum/oracle.ben
	@echo ""
	@echo "=== three_fates.ben ==="
	@$(EXE) benedictum/three_fates.ben
	@echo ""
	@echo "=== bottles.ben ==="
	@$(EXE) benedictum/bottles.ben
	@echo ""
	@echo "=== sanctus.ben ==="
	@$(EXE) benedictum/sanctus.ben
	@echo ""
	@echo "=== dice.ben ==="
	@$(EXE) benedictum/dice.ben
	@echo ""
	@echo "=== All tests passed. Deo gratias. ==="

.PHONY: all debug install uninstall clean test