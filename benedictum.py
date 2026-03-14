#!/usr/bin/env python3
"""Benedictum v1.1.0 — Latin-themed esoteric programming language."""

import sys
import re
import random

VERSION = "1.1.0"

KEYWORDS = [
    # longer keywords first to avoid prefix collisions
    ("requiem", "REQUIEM"),
    ("sanctus", "SANCTUS"),
    ("numerus", "NUMERUS"),
    ("fatum",   "FATUM"),
    ("bene",    "BENE"),
    ("male",    "MALE"),
    ("audi",    "AUDI"),
    ("amen",    "AMEN"),
    ("dex",     "DEX"),
    ("sin",     "SIN"),
    ("dic",     "DIC"),
    ("lux",     "LUX"),
    ("nox",     "NOX"),
    ("ora",     "ORA"),
]

TAPE_SIZE = 30000

_pattern  = "|".join(rf"\b{kw}\b" for kw, _ in KEYWORDS)
_kw_map   = {kw: tok for kw, tok in KEYWORDS}
_TOKEN_RE = re.compile(_pattern)

BLESSINGS = [
    "Benedictum completum est. Deo gratias.",
    "Ita fiat. Machina benedicta est.",
    "Dixi. Finis coronat opus.",
    "Computatio sancta est. Pax vobiscum.",
    "Laus Deo. Programma in pace quiescit.",
    "Consummatum est. Vale, lector.",
    "Ora et labora. Et labora iterum.",
    "Machina oravit. Machina audivit. Machina dixit.",
    "Omnia in numeris posita sunt. Et numeravimus.",
    "Errare humanum est. Sed machina non erravit.",
]


def tokenize(source):
    tokens = [_kw_map[m.group()] for m in _TOKEN_RE.finditer(source)]

    depth = 0
    for i, tok in enumerate(tokens):
        if tok == "ORA":
            depth += 1
        elif tok == "AMEN":
            depth -= 1
            if depth < 0:
                print(
                    f"HAERESIS! Amen sine ora dictum est! "
                    f"(Unexpected 'amen' at token {i} — no matching 'ora')",
                    file=sys.stderr,
                )
                return None
    if depth != 0:
        print(
            f"SCHISMA! Oratio sine fine incipit! "
            f"({depth} 'ora' missing a matching 'amen')",
            file=sys.stderr,
        )
        return None

    return tokens


def gloria_dump(tape, ptr, pc, tok):
    start = max(ptr - 3, 0)
    end   = min(ptr + 4, TAPE_SIZE)
    cells = ""
    if start > 0:
        cells += "... "
    for i in range(start, end):
        if i == ptr:
            cells += f"[{tape[i]:3d}] "
        else:
            cells += f" {tape[i]:3d}  "
    if end < TAPE_SIZE:
        cells += "..."
    print(f"  [{tok:<8s}] pc={pc:<5d} ptr={ptr:<5d} | {cells}", file=sys.stderr)


def execute(tokens, gloria=False):
    tape = bytearray(TAPE_SIZE)
    ptr  = 0
    pc   = 0
    n    = len(tokens)

    while pc < n:
        tok = tokens[pc]

        if gloria:
            gloria_dump(tape, ptr, pc, tok)

        if tok == "BENE":
            tape[ptr] = (tape[ptr] + 1) & 0xFF

        elif tok == "MALE":
            tape[ptr] = (tape[ptr] - 1) & 0xFF

        elif tok == "DEX":
            ptr += 1
            if ptr >= TAPE_SIZE:
                print(
                    f"CALAMITAS! Poeta ad finem mundi pervenit! "
                    f"(Tape overflow at instruction {pc})",
                    file=sys.stderr,
                )
                return 1

        elif tok == "SIN":
            ptr -= 1
            if ptr < 0:
                print(
                    f"CALAMITAS! Ante initium mundi nemo habitat! "
                    f"(Tape underflow at instruction {pc})",
                    file=sys.stderr,
                )
                return 1

        elif tok == "DIC":
            sys.stdout.write(chr(tape[ptr]))
            sys.stdout.flush()

        elif tok == "AUDI":
            c = sys.stdin.read(1)
            tape[ptr] = 0 if c == "" else ord(c) & 0xFF

        elif tok == "ORA":
            if tape[ptr] == 0:
                depth = 1
                while depth:
                    pc += 1
                    if tokens[pc] == "ORA":  depth += 1
                    elif tokens[pc] == "AMEN": depth -= 1

        elif tok == "AMEN":
            if tape[ptr] != 0:
                depth = 1
                while depth:
                    pc -= 1
                    if tokens[pc] == "AMEN": depth += 1
                    elif tokens[pc] == "ORA":  depth -= 1

        elif tok == "LUX":
            sys.stdout.write(str(tape[ptr]))
            sys.stdout.flush()

        elif tok == "NOX":
            sys.stdout.write("\n")
            sys.stdout.flush()

        elif tok == "FATUM":
            tape[ptr] = random.randint(0, 255)

        elif tok == "REQUIEM":
            break

        elif tok == "SANCTUS":
            tape[ptr] = 0

        elif tok == "NUMERUS":
            try:
                line = sys.stdin.readline().strip()
                tape[ptr] = int(line) & 0xFF
            except (ValueError, EOFError):
                tape[ptr] = 0

        pc += 1

    return 0


def print_usage():
    print(f"Benedictum v{VERSION}")
    print("A Latin-themed esoteric programming language.\n")
    print("Usage: python benedictum.py [options] <file.ben>\n")
    print("Options:")
    print("  --gloria   Verbose mode: dump tape state after every instruction")
    print("  --lex      Tokenize only; print token list and exit")
    print("  --version  Show version")
    print("  --help     Show this help\n")
    print("Commands:")
    print("  bene     increment cell             (Latin: good)")
    print("  male     decrement cell             (Latin: badly)")
    print("  dex      move pointer right         (Latin: right)")
    print("  sin      move pointer left          (Latin: left)")
    print("  dic      output cell as ASCII char  (Latin: speak)")
    print("  audi     read one byte from stdin   (Latin: listen)")
    print("  ora      begin loop                 (Latin: pray)")
    print("  amen     end loop                   (Latin: so be it)")
    print("  lux      print cell as decimal      (Latin: light)")
    print("  nox      print a newline            (Latin: night)")
    print("  fatum    set cell to random 0-255   (Latin: fate)")
    print("  requiem  halt immediately           (Latin: rest)")
    print("  sanctus  zero the current cell      (Latin: holy)")
    print("  numerus  read decimal from stdin    (Latin: number)")


def main():
    gloria = False
    lex    = False
    file   = None

    for arg in sys.argv[1:]:
        if arg in ("--version", "-v"):
            print(f"benedictum {VERSION}")
            return 0
        elif arg in ("--help", "-h"):
            print_usage()
            return 0
        elif arg == "--gloria":
            gloria = True
        elif arg == "--lex":
            lex = True
        elif not arg.startswith("-"):
            file = arg
        else:
            print(f"Unknown option: {arg}", file=sys.stderr)
            return 1

    if not file:
        print_usage()
        return 0

    try:
        with open(file, "r", encoding="utf-8") as f:
            source = f.read()
    except OSError as e:
        print(f"Error: cannot open '{file}': {e}", file=sys.stderr)
        return 1

    tokens = tokenize(source)
    if tokens is None:
        return 1

    if lex:
        print(f"Benedictum v{VERSION} -- Lexed '{file}' ({len(tokens)} tokens):\n")
        for i, tok in enumerate(tokens):
            print(f"  {i:4d}  {tok.lower()}")
        return 0

    if gloria:
        print("=== GLORIA MODE: In nomine machinae ===\n", file=sys.stderr)

    result = execute(tokens, gloria=gloria)

    if result == 0:
        print(f"\n*** {random.choice(BLESSINGS)}", file=sys.stderr)

    return result


if __name__ == "__main__":
    sys.exit(main())
