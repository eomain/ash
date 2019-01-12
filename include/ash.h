/* Copyright 2018 eomain
   this program is licensed under the 2-clause BSD license
   see LICENSE for the full license info
*/

#ifndef ASH_ASH
#define ASH_ASH

#define PNAME "ash"
#define ASH_VERSION "0.1.4"
#define ASH_VERSION_MAJOR "0"
#define ASH_VERSION_MINOR "1"
#define ASH_VERSION_MICRO "4"

extern void ash_set_status(int);
extern void ash_print_help(void);
extern void ash_logout(void);
extern int ash_check_login(void);
extern void ash_abort(const char *);

#endif
