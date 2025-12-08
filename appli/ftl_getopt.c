
#include <stdio.h>
#include <string.h>

int optind = 1;
char *optarg = NULL;
int opterr = 1;
int optopt = 0;

int ftl_getopt(int argc, char * const argv[], const char *optstring) {
    if (optind >= argc) return -1;

    char *arg = argv[optind];
    if (arg[0] != '-' || arg[1] == '\0') return -1; // オプションでない

    char opt = arg[1];
    const char *p = strchr(optstring, opt);
    if (!p) {
        optopt = opt;
        if (opterr) fprintf(stderr, "Unknown option '-%c'\n", opt);
        optind++;
        return '?';
    }

    if (*(p + 1) == ':') { // 引数必須
        if (optind + 1 < argc) {
            optarg = argv[++optind];
        } else {
            optopt = opt;
            if (opterr) fprintf(stderr, "Option '-%c' requires an argument\n", opt);
            optind++;
            return '?';
        }
    }

    optind++;
    return opt;
}
