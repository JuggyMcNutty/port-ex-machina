#define _GNU_SOURCE
#include "core/relaunch.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char **dxl_argv_build(const char *exe, const char *flags) {
    size_t cap = 8, n = 0;
    char **argv = dxl_xmalloc(cap * sizeof *argv);
    argv[n++] = dxl_xstrdup(exe);

    if (flags && *flags) {
        /* Shell-like word splitting: unquoted whitespace separates, and a
         * quote may open anywhere inside a word, so -map="Deus Ex.dx" stays
         * one argument. Paths with spaces are normal on an SD card. */
        dxl_buf word;
        dxl_buf_init(&word);
        char quote = 0;
        int in_word = 0;

        for (const char *p = flags; ; p++) {
            if (*p && (quote ? *p != quote : !isspace((unsigned char)*p))) {
                if (!quote && (*p == '"' || *p == '\'')) { quote = *p; in_word = 1; continue; }
                dxl_buf_add(&word, p, 1);
                in_word = 1;
                continue;
            }
            if (quote && *p == quote) { quote = 0; continue; }

            if (in_word) {
                if (n + 2 > cap) {
                    cap *= 2;
                    argv = dxl_xrealloc(argv, cap * sizeof *argv);
                }
                argv[n++] = dxl_xstrdup(word.data ? word.data : "");
                word.len = 0;
                if (word.data) word.data[0] = '\0';
                in_word = 0;
            }
            if (!*p) break;
        }
        dxl_buf_free(&word);
    }
    if (n + 1 > cap) argv = dxl_xrealloc(argv, (cap = n + 1) * sizeof *argv);
    argv[n] = NULL;
    return argv;
}

void dxl_argv_free(char **argv) {
    if (!argv) return;
    for (char **p = argv; *p; p++) free(*p);
    free(argv);
}

int dxl_relaunch(const char *exe, const char *flags, const char *workdir,
                 dxl_err *err) {
    if (workdir && *workdir && chdir(workdir) != 0) {
        dxl_err_set(err, "cannot enter %s: %s", workdir, strerror(errno));
        return -1;
    }
    char **argv = dxl_argv_build(exe, flags);
    execv(exe, argv);

    /* Only reached on failure -- exec does not return on success. */
    dxl_err_set(err, "cannot exec %s: %s", exe, strerror(errno));
    dxl_argv_free(argv);
    return -1;
}
