#include "getopt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int optind = 1;
int opterr = 1;
int optopt;
char *optarg;

#define AXIS2_OPT_ERR_NO_ARG         1
#define AXIS2_OPT_ERR_INVALID_OPTION   2
#define AXIS2_OPT_ERR_BAD_ARG         3

int
opt_error(
    int __optopt,
    int __err,
    int __showerr)
{
    switch (__err)
    {
    case AXIS2_OPT_ERR_NO_ARG:
        if (__showerr)
            fprintf(stderr, " option requires an argument -- %c\n", __optopt);
        break;
    case AXIS2_OPT_ERR_INVALID_OPTION:
        if (__showerr)
            fprintf(stderr, " illegal option -- %c\n", __optopt);
        break;
    case AXIS2_OPT_ERR_BAD_ARG:
        return (int) ':';
    default:
        if (__showerr)
            fprintf(stderr, "unknown\n");
    }
    return (int) '?';
}

int
getopt(
    int __argc,
    char *const *__argv,
    const char *__shortopts)
{
    static char *pos = "";
    char *olstindex = NULL;

    if (!*pos)
    {
        /** no option or invalid option */
        if (optind >= __argc || *(pos = __argv[optind]) != '-')
        {
            pos = "";
            return -1;
        }

        /**-- option*/
        if (pos[1] && *++pos == '-')
        {
            ++optind;
            pos = "";
            return -1;
        }
    }

    if ((optopt = (int) *pos++) == (int) ':')
    {
        if (optopt == (int) '-')
            return -1;
        if (!*pos)
            ++optind;
        if (*__shortopts != ':')
            return opt_error(optopt, AXIS2_OPT_ERR_BAD_ARG, opterr);
        opt_error(optopt, AXIS2_OPT_ERR_INVALID_OPTION, opterr);
    }
    else
    {
        olstindex = strchr(__shortopts, optopt);
        if (!olstindex)
        {
            if (optopt == (int) '-')
                return -1;
            if (!*pos)
                ++optind;
            if (*__shortopts != ':')
                return opt_error(optopt, AXIS2_OPT_ERR_BAD_ARG, opterr);
            opt_error(optopt, AXIS2_OPT_ERR_INVALID_OPTION, opterr);
        }
    }

    if (!olstindex || *++olstindex != ':')
    {
        optarg = NULL;
        if (!*pos)
            ++optind;
    }
    else
    {
        if (*pos)
            optarg = pos;
        else if (__argc <= ++optind)
        {
            pos = "";
            if (*__shortopts == ':')
                return opt_error(-1, AXIS2_OPT_ERR_BAD_ARG, opterr);
            return opt_error(optopt, AXIS2_OPT_ERR_NO_ARG, opterr);
        }
        else
            optarg = __argv[optind];
        pos = "";
        ++optind;
    }
    return optopt;
}