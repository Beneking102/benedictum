#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define TAPE_SIZE 30000
#define VERSION "1.1.0"

/* ── Token types ─────────────────────────────────────────────────────────── */

typedef enum {
    TOK_BENE,
    TOK_MALE,
    TOK_DEX,
    TOK_SIN,
    TOK_DIC,
    TOK_AUDI,
    TOK_ORA,
    TOK_AMEN,
    /* v1.1 extensions */
    TOK_LUX,      /* print cell as decimal number  */
    TOK_NOX,      /* print newline                 */
    TOK_FATUM,    /* set cell to random 0-255      */
    TOK_REQUIEM,  /* halt immediately              */
    TOK_SANCTUS,  /* zero the current cell         */
    TOK_NUMERUS   /* read decimal number from stdin */
} TokenType;

static const char *token_name(TokenType t)
{
    switch (t) {
        case TOK_BENE:    return "bene";
        case TOK_MALE:    return "male";
        case TOK_DEX:     return "dex";
        case TOK_SIN:     return "sin";
        case TOK_DIC:     return "dic";
        case TOK_AUDI:    return "audi";
        case TOK_ORA:     return "ora";
        case TOK_AMEN:    return "amen";
        case TOK_LUX:     return "lux";
        case TOK_NOX:     return "nox";
        case TOK_FATUM:   return "fatum";
        case TOK_REQUIEM: return "requiem";
        case TOK_SANCTUS: return "sanctus";
        case TOK_NUMERUS: return "numerus";
        default:          return "???";
    }
}

/* ── Program (dynamic token array) ──────────────────────────────────────── */

typedef struct {
    TokenType *tokens;
    int count;
    int capacity;
} Program;

static void program_init(Program *prog)
{
    prog->capacity = 1024;
    prog->tokens = malloc(sizeof(TokenType) * prog->capacity);
    if (!prog->tokens) {
        fprintf(stderr, "CALAMITAS MAGNA! Memoria exhausta est!\n");
        exit(1);
    }
    prog->count = 0;
}

static void program_add(Program *prog, TokenType tok)
{
    if (prog->count >= prog->capacity) {
        prog->capacity *= 2;
        prog->tokens = realloc(prog->tokens, sizeof(TokenType) * prog->capacity);
        if (!prog->tokens) {
            fprintf(stderr, "CALAMITAS MAGNA! Memoria exhausta est!\n");
            exit(1);
        }
    }
    prog->tokens[prog->count++] = tok;
}

static void program_free(Program *prog)
{
    free(prog->tokens);
    prog->tokens = NULL;
    prog->count = 0;
}

/* ── Lexer ───────────────────────────────────────────────────────────────── */

static int is_word_boundary(char c)
{
    return !isalpha((unsigned char)c);
}

static int try_match(const char *src, int pos, int len, const char *kw, int kwlen)
{
    if (pos + kwlen > len) return 0;
    if (strncmp(&src[pos], kw, kwlen) != 0) return 0;
    if (pos + kwlen < len && !is_word_boundary(src[pos + kwlen])) return 0;
    return 1;
}

/* Keyword table — longer keywords listed before shorter ones to avoid
   prefix collisions (e.g. "sanctus" before "sin"). */
static const struct { const char *word; int len; TokenType tok; } KWDS[] = {
    { "requiem", 7, TOK_REQUIEM },
    { "sanctus", 7, TOK_SANCTUS },
    { "numerus", 7, TOK_NUMERUS },
    { "fatum",   5, TOK_FATUM   },
    { "bene",    4, TOK_BENE    },
    { "male",    4, TOK_MALE    },
    { "audi",    4, TOK_AUDI    },
    { "amen",    4, TOK_AMEN    },
    { "dex",     3, TOK_DEX     },
    { "sin",     3, TOK_SIN     },
    { "dic",     3, TOK_DIC     },
    { "lux",     3, TOK_LUX     },
    { "nox",     3, TOK_NOX     },
    { "ora",     3, TOK_ORA     },
};
#define N_KWDS (int)(sizeof(KWDS) / sizeof(KWDS[0]))

static int tokenize(const char *source, Program *prog)
{
    int i = 0;
    int len = (int)strlen(source);

    while (i < len) {
        if (isspace((unsigned char)source[i])) { i++; continue; }

        int matched = 0;
        for (int k = 0; k < N_KWDS; k++) {
            if (try_match(source, i, len, KWDS[k].word, KWDS[k].len)) {
                program_add(prog, KWDS[k].tok);
                i += KWDS[k].len;
                matched = 1;
                break;
            }
        }
        if (!matched) i++;
    }

    /* validate brackets */
    int depth = 0;
    for (int j = 0; j < prog->count; j++) {
        if (prog->tokens[j] == TOK_ORA)  depth++;
        if (prog->tokens[j] == TOK_AMEN) depth--;
        if (depth < 0) {
            fprintf(stderr,
                "HAERESIS! Amen sine ora dictum est! "
                "(Unexpected 'amen' at token %d — no matching 'ora')\n", j);
            return 1;
        }
    }
    if (depth != 0) {
        fprintf(stderr,
            "SCHISMA! Oratio sine fine incipit! "
            "(%d 'ora' missing a matching 'amen')\n", depth);
        return 1;
    }

    return 0;
}

/* ── Gloria (verbose) tape dump ──────────────────────────────────────────── */

static void gloria_dump(const unsigned char *tape, int ptr, int pc, TokenType tok)
{
    fprintf(stderr, "  [%-8s] pc=%-5d ptr=%-5d | ", token_name(tok), pc, ptr);
    int start = ptr - 3; if (start < 0) start = 0;
    int end   = ptr + 4; if (end > TAPE_SIZE) end = TAPE_SIZE;
    if (start > 0) fprintf(stderr, "... ");
    for (int i = start; i < end; i++) {
        if (i == ptr) fprintf(stderr, "[%3d] ", tape[i]);
        else          fprintf(stderr, " %3d  ", tape[i]);
    }
    if (end < TAPE_SIZE) fprintf(stderr, "...");
    fprintf(stderr, "\n");
}

/* ── Executor ────────────────────────────────────────────────────────────── */

static int execute(Program *prog, int gloria)
{
    unsigned char tape[TAPE_SIZE];
    memset(tape, 0, sizeof(tape));
    int ptr = 0;
    int pc  = 0;

    while (pc < prog->count) {
        TokenType tok = prog->tokens[pc];

        if (gloria) gloria_dump(tape, ptr, pc, tok);

        switch (tok) {

        case TOK_BENE:
            tape[ptr]++;
            break;

        case TOK_MALE:
            tape[ptr]--;
            break;

        case TOK_DEX:
            if (++ptr >= TAPE_SIZE) {
                fprintf(stderr,
                    "CALAMITAS! Poeta ad finem mundi pervenit! "
                    "(Tape overflow at instruction %d)\n", pc);
                return 1;
            }
            break;

        case TOK_SIN:
            if (--ptr < 0) {
                fprintf(stderr,
                    "CALAMITAS! Ante initium mundi nemo habitat! "
                    "(Tape underflow at instruction %d)\n", pc);
                return 1;
            }
            break;

        case TOK_DIC:
            putchar(tape[ptr]);
            fflush(stdout);
            break;

        case TOK_AUDI:
            {
                int c = getchar();
                tape[ptr] = (c == EOF) ? 0 : (unsigned char)c;
            }
            break;

        case TOK_ORA:
            if (tape[ptr] == 0) {
                int depth = 1;
                while (depth > 0) {
                    pc++;
                    if (prog->tokens[pc] == TOK_ORA)  depth++;
                    if (prog->tokens[pc] == TOK_AMEN) depth--;
                }
            }
            break;

        case TOK_AMEN:
            if (tape[ptr] != 0) {
                int depth = 1;
                while (depth > 0) {
                    pc--;
                    if (prog->tokens[pc] == TOK_AMEN) depth++;
                    if (prog->tokens[pc] == TOK_ORA)  depth--;
                }
            }
            break;

        case TOK_LUX:
            fprintf(stdout, "%d", tape[ptr]);
            fflush(stdout);
            break;

        case TOK_NOX:
            putchar('\n');
            fflush(stdout);
            break;

        case TOK_FATUM:
            tape[ptr] = (unsigned char)(rand() % 256);
            break;

        case TOK_REQUIEM:
            goto done;

        case TOK_SANCTUS:
            tape[ptr] = 0;
            break;

        case TOK_NUMERUS:
            {
                int n = 0;
                if (scanf("%d", &n) != 1) n = 0;
                tape[ptr] = (unsigned char)(n & 0xFF);
            }
            break;
        }

        pc++;
    }

done:
    return 0;
}

/* ── File reader ─────────────────────────────────────────────────────────── */

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) {
        fprintf(stderr, "Error: cannot determine size of '%s'\n", path);
        fclose(f);
        return NULL;
    }
    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fprintf(stderr, "CALAMITAS MAGNA! Memoria exhausta est!\n");
        fclose(f);
        return NULL;
    }
    size_t read = fread(buf, 1, (size_t)size, f);
    buf[read] = '\0';
    fclose(f);
    return buf;
}

/* ── Usage ───────────────────────────────────────────────────────────────── */

static void print_usage(void)
{
    printf("Benedictum v%s\n", VERSION);
    printf("A Latin-themed esoteric programming language.\n\n");
    printf("Usage: benedictum [options] <file.ben>\n\n");
    printf("Options:\n");
    printf("  --gloria   Verbose mode: dump tape state after every instruction\n");
    printf("  --lex      Tokenize only; print token list and exit\n");
    printf("  --version  Show version\n");
    printf("  --help     Show this help\n\n");
    printf("Commands:\n");
    printf("  bene     increment cell             (Latin: good)\n");
    printf("  male     decrement cell             (Latin: badly)\n");
    printf("  dex      move pointer right         (Latin: right)\n");
    printf("  sin      move pointer left          (Latin: left)\n");
    printf("  dic      output cell as ASCII char  (Latin: speak)\n");
    printf("  audi     read one byte from stdin   (Latin: listen)\n");
    printf("  ora      begin loop                 (Latin: pray)\n");
    printf("  amen     end loop                   (Latin: so be it)\n");
    printf("  lux      print cell as decimal      (Latin: light)\n");
    printf("  nox      print a newline            (Latin: night)\n");
    printf("  fatum    set cell to random 0-255   (Latin: fate)\n");
    printf("  requiem  halt immediately           (Latin: rest)\n");
    printf("  sanctus  zero the current cell      (Latin: holy)\n");
    printf("  numerus  read decimal from stdin    (Latin: number)\n");
}

/* ── Exit blessings ──────────────────────────────────────────────────────── */

static const char *BLESSINGS[] = {
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
};
#define N_BLESSINGS (int)(sizeof(BLESSINGS) / sizeof(BLESSINGS[0]))

/* ── Main ────────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

    int gloria = 0;
    int lex    = 0;
    const char *file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("benedictum %s\n", VERSION);
            return 0;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        } else if (strcmp(argv[i], "--gloria") == 0) {
            gloria = 1;
        } else if (strcmp(argv[i], "--lex") == 0) {
            lex = 1;
        } else if (argv[i][0] != '-') {
            file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (!file) {
        print_usage();
        return 0;
    }

    char *source = read_file(file);
    if (!source) return 1;

    Program prog;
    program_init(&prog);

    if (tokenize(source, &prog) != 0) {
        free(source);
        program_free(&prog);
        return 1;
    }

    if (lex) {
        printf("Benedictum v%s -- Lexed '%s' (%d tokens):\n\n", VERSION, file, prog.count);
        for (int i = 0; i < prog.count; i++)
            printf("  %4d  %s\n", i, token_name(prog.tokens[i]));
        free(source);
        program_free(&prog);
        return 0;
    }

    if (gloria)
        fprintf(stderr, "=== GLORIA MODE: In nomine machinae ===\n\n");

    int result = execute(&prog, gloria);

    free(source);
    program_free(&prog);

    if (result == 0)
        fprintf(stderr, "\n*** %s\n", BLESSINGS[rand() % N_BLESSINGS]);

    return result;
}
