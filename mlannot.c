/* Simple OCaml annotation extractor. */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SZ          4097

struct annot {
    char * tag;
    char * value;
    struct annot * next;
};

static void die(const char *);
static void usage(int);
static inline struct annot * cons_annot(struct annot ***);
static struct annot * find_annot(FILE *, unsigned int);
static void print_free_annot(struct annot *, FILE *);
static char * strapp(char * restrict, char * restrict);
static char * strdup(const char *);
static bool strsuff(const char * restrict, const char * restrict);

static void
die(const char * msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    abort();
}

static char *
strapp(char * restrict s1, char * restrict s2)
{
    const size_t len = (s1 ? strlen(s1) + 1 : 0) + strlen(s2) + 1;
    char * out = malloc(len);

    if (!out)
        die("Out of memory.");
    if (s1) {
        strcpy(out, s1);
        strcat(out, " ");
        free(s1);
    }
    return strcat(out, s2);
}

static char *
strdup(const char * s)
{
    char * str = malloc(strlen(s) + 1);

    if (str == NULL)
        die("Out of memory.");
    return strcpy(str, s);
}

static bool
strsuff(const char * s, const char * suff)
{
    const int off = strlen(s) - strlen(suff);

    if (off < 0)
        return false;
    s = &s[off];
    while (*s && (*s == *suff))
        s++, suff++;
    return !*s;
}

static inline struct annot *
cons_annot(struct annot *** pppa)
{
    struct annot * pa;

    if ((pa = **pppa = malloc(sizeof(struct annot))) == NULL)
        die("Out of memory.");
    *pa = (struct annot) { NULL, NULL, NULL };
    *pppa = &pa->next;
    return pa;
}

static struct annot *
find_annot(FILE * f, unsigned int pos)
{
    enum { TOP, ANNOT, FTOP, FANNOT } state = TOP;
    static char buf[BUF_SZ];
    struct annot * ret = NULL;
    struct annot ** cur_annot = &ret;
    struct annot * ann = NULL;

    while (!feof(f) && !ferror(f)) {
        size_t len;

        if (fgets(buf, BUF_SZ, f) == NULL) {
            if (ferror(f)) {
                perror("fgets");
                continue;
            }
            return ret;
        }

        len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            len--;
            buf[len] = 0;
        }

        switch (state) {
        case TOP: {
            if (strchr(buf, '('))
                state = ANNOT;
            else {
                unsigned int cstart, cend;
                int ss;

                ss = sscanf(buf, "\"%*[^\"]\" %*d %*d %u"
                                 " \"%*[^\"]\" %*d %*d %u",
                            &cstart, &cend);
                if (ss == 2 && (pos >= cstart && pos <= cend))
                    state = FTOP;
                continue;
            }
        }
        case ANNOT: {
            if (strcmp(buf, ")") == 0)
                state = TOP;
            continue;
        }
        case FTOP: {
            if (strchr(buf, '(')) {
                char * p = buf;

                while (!isspace(*p) && *p && *p != '(')
                    p++;
                *p = 0;
                ann = cons_annot(&cur_annot);
                ann->tag = strdup(buf);
                state = FANNOT;
                continue;
            } else
                return ret;
        }
        case FANNOT: {
            if (strcmp(buf, ")") == 0)
                state = FTOP;
            else {
                char *pstart = buf;

                while (isspace(*pstart))
                    pstart++;
                ann->value = strapp(ann->value, pstart);
            }
            continue;
        }
        }
    }

    return ret;
}

static void
print_free_annot(struct annot * ann, FILE * f)
{
    bool sep = false;

    while (ann) {
        struct annot * const next = ann->next;
        if (ann->tag && ann->value) {
            fprintf(f, "%s%s: %s", sep ? "  |  " : "",
                    ann->tag, ann->value);
            free(ann->tag);
            free(ann->value);
            sep = true;
        }
        free(ann);
        ann = next;
    }
    fputc('\n', f);
}

static void
usage(int ret)
{
    fputs("usage: mlannot FILE CHAR_NUMBER\n", ret ? stderr : stdout);
    exit(ret);
}

int
main(int argc, char ** argv)
{
    char * fname;
    FILE * f;

    if (argc > 1 && strcmp(argv[1], "-h") == 0)
        usage(0);
    if (argc < 3)
        usage(1);

    if (strsuff(fname = argv[1], ".ml")) {
        const size_t flen = strlen(argv[1]) + 4;

        if ((fname = malloc(flen)) == NULL)
            die("Out of memory.");
        argv[1][flen - 7] = 0;
        strcpy(fname, argv[1]);
        strcat(fname, ".annot");
    }

    if ((f = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "Cannot open %s for reading.\n", fname);
    } else {
        struct annot * ann;

        if ((ann = find_annot(f, atoi(argv[2]))))
            print_free_annot(ann, stdout);
        else
            puts("No matching annotation.");
        fclose(f);
    }

    if (fname != argv[1])
        free(fname);
    return 0;
}
