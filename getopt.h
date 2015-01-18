#ifndef GETOPT_H
#define GETOPT_H

#ifdef __cplusplus
extern "C"{
#endif

extern int optopt;
extern char *optarg;

extern int
opt_error(
    int __optopt,
    int __err,
    int __showerr);

extern getopt(
    int __argc,
    char *const *__argv,
    const char *__shortopts);

#ifdef __cplusplus
}
#endif

#endif