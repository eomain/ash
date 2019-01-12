/* Copyright 2018 eomain
   this program is licensed under the 2-clause BSD license
   see LICENSE for the full license info
*/

#ifndef ASH_BUILTIN
#define ASH_BUILTIN

enum ash_builtin {
    ASH_BUILTIN_ASSERT = 0,
    ASH_BUILTIN_BUILTIN,
    ASH_BUILTIN_CD,
    ASH_BUILTIN_EXIT,
    ASH_BUILTIN_ECHO,
    ASH_BUILTIN_EXPORT,
    ASH_BUILTIN_HELP,
    ASH_BUILTIN_HISTORY,
    ASH_BUILTIN_SLEEP,
    ASH_BUILTIN_UNSET,

    /* number of ash builtin commands */
    ASH_BUILTIN_NO
};

extern void ash_builtin_exec(int, int, const char * const *);
extern int ash_builtin_find(const char *);

#endif
