# Benedictum

**A Latin-themed esoteric programming language.**

Benedictum is a Turing-complete esoteric language inspired by [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck), replacing cryptic symbols with Latin commands. Every program is a *benediction* — a blessing whispered to the machine.

Every word that isn't a command is silently ignored, so your source code can be written as flowing Latin prose with instructions woven in.

---

[![CI](https://github.com/Beneking102/benedictum/actions/workflows/ci.yml/badge.svg)](https://github.com/Beneking102/benedictum/actions/workflows/ci.yml)

## Language Reference

### Core Commands (v1.0)

| Command | Latin Meaning | Brainfuck | Description |
|---------|--------------|-----------|-------------|
| `bene`  | *good*       | `+`       | Increment the current cell (wraps 255 → 0) |
| `male`  | *badly*      | `-`       | Decrement the current cell (wraps 0 → 255) |
| `dex`   | *right*      | `>`       | Move tape pointer right |
| `sin`   | *left*       | `<`       | Move tape pointer left |
| `dic`   | *speak*      | `.`       | Output current cell as ASCII character |
| `audi`  | *listen*     | `,`       | Read one byte from stdin into current cell |
| `ora`   | *pray*       | `[`       | Begin loop — skip to matching `amen` if cell is 0 |
| `amen`  | *so be it*   | `]`       | End loop — jump back to matching `ora` if cell is non-zero |

### Extended Commands (v1.1)

| Command   | Latin Meaning | Description |
|-----------|--------------|-------------|
| `lux`     | *light*      | Print current cell as a **decimal number** |
| `nox`     | *night*      | Print a **newline** character |
| `fatum`   | *fate*       | Set current cell to a **random value** (0–255) |
| `requiem` | *rest*       | **Halt** the program immediately |
| `sanctus` | *holy*       | **Zero** the current cell (shorthand for `[male]`) |
| `numerus` | *number*     | Read a **decimal integer** from stdin into the current cell |

---

## Build

```bash
# compile
make

# or manually
gcc -Wall -Wextra -O2 -std=c99 -o benedictum src/benedictum.c
```

On Windows, `make` produces `benedictum.exe`. Requires GCC (e.g. [WinLibs](https://winlibs.com)) or any C99-compatible compiler.

---

## Usage

```bash
benedictum [options] <file.ben>
```

### Options

| Flag        | Description |
|-------------|-------------|
| `--gloria`  | Verbose mode — dumps the full tape state after every instruction |
| `--lex`     | Tokenize only — prints the instruction list and exits (great for debugging) |
| `--version` | Print version and exit |
| `--help`    | Print help and exit |

### Examples

```bash
./benedictum benedictum/hello.ben
./benedictum benedictum/dice.ben
./benedictum --gloria benedictum/pax.ben
./benedictum --lex benedictum/fibonacci.ben
```

---

## Example Programs

All examples live in the [`benedictum/`](benedictum/) directory.

| File | Description |
|------|-------------|
| [`hello.ben`](benedictum/hello.ben) | Prints `Hello World!` |
| [`alphabet.ben`](benedictum/alphabet.ben) | Prints `ABCDEFGHIJKLMNOPQRSTUVWXYZ` |
| [`countdown.ben`](benedictum/countdown.ben) | Prints `9876543210` |
| [`echo.ben`](benedictum/echo.ben) | Reads stdin and echoes it back |
| [`pax.ben`](benedictum/pax.ben) | Counts down 9 → 0, one number per line |
| [`bottles.ben`](benedictum/bottles.ben) | Counts down 10 → 0 (ten bottles of beer) |
| [`fibonacci.ben`](benedictum/fibonacci.ben) | Prints the first 14 Fibonacci numbers |
| [`oracle.ben`](benedictum/oracle.ben) | Prints a random number (uses `fatum`) |
| [`three_fates.ben`](benedictum/three_fates.ben) | Three random fate dice — Clotho, Lachesis, Atropos |
| [`dice.ben`](benedictum/dice.ben) | Rolls a fair d6 (uses `fatum` + mod 6 algorithm) |
| [`sanctus.ben`](benedictum/sanctus.ben) | Demonstrates the `sanctus` command |
| [`domine.ben`](benedictum/domine.ben) | Reads a character, prints its ASCII value |
| [`numerus.ben`](benedictum/numerus.ben) | Reads a number and doubles it |

---

## How It Works

Benedictum is a **tape machine**:

- A tape of **30,000 cells**, each holding an unsigned value from 0–255
- A **pointer** that starts at cell 0
- Commands move the pointer, modify cells, perform I/O, or loop

The only control flow is `ora`/`amen` loops: if the current cell is zero, `ora` skips to the matching `amen`; if non-zero, `amen` jumps back to the matching `ora`. This is enough to be **Turing-complete**.

---

## Comments and Prose

Any text that is not a recognised command is silently ignored. This lets you write flowing prose around your code:

```
This is a prayer to the machine.
Let us begin with a blessing.

bene bene bene bene bene bene bene bene bene
ora dex bene bene bene bene bene bene bene male amen

The cell has been sanctified. Now speak.

dex dic
```

> ⚠️ **Gotcha:** some common English and Latin words are also Benedictum commands — `sin`, `male`, `amen`, `ora`, `fatum`, `lux`. If these appear in your prose at a word boundary they will be tokenized as instructions. Use `--lex` to inspect what the tokenizer actually sees.

---

## Converting from Brainfuck

Any Brainfuck program translates directly:

| Brainfuck | Benedictum |
|-----------|------------|
| `+` | `bene` |
| `-` | `male` |
| `>` | `dex` |
| `<` | `sin` |
| `.` | `dic` |
| `,` | `audi` |
| `[` | `ora` |
| `]` | `amen` |

---

## File Extension

Benedictum source files use the `.ben` extension.

---

## System Requirements

- A C99-compatible compiler: GCC, Clang, or MSVC
- Works on Linux, macOS, and Windows

---

## License

MIT — see [LICENSE](LICENSE)

---

## Author

**Bene** — [GitHub](https://github.com/Beneking102) · [Portfolio](https://benedikt-pkr.info)
