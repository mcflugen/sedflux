//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#ifndef __EH_GET_OPT_H__
#define __EH_GET_OPT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <glib.h>
#include <utils/eh_types.h>
#include <utils/eh_symbol_table.h>

typedef struct {
    Eh_symbol_table args;
    Eh_symbol_table defaults;
}
Eh_args;

// create a new Eh_args.
Eh_args* eh_create_args(void);

// destroy a Eh_args.
// args        : the Eh_args to destroy (args can be NULL).
void eh_destroy_args(Eh_args* args);

// parse the command line options and initialize an arg table.
// argc        : the number of command line arguments.
// argv        : a list of the command line arguments.
Eh_args* eh_opts_init(int argc, char* argv[]);

// read the default values from the eh resource file.  first read values
// from ~/.ehrc and the ./.ehrc.  values in ./.ehrc take precedence over
// values in ~/.ehrc.
// prog_name   : the name of the current program.  the .ehrc files are
//             : scanned for records labeled with prog_name.
Eh_symbol_table eh_get_opt_defaults(const char* prog_name);

// lookup the value of a command line option.
// args   : an argument table returned by eh_opts_init.
// key         : the name of the option to look for.
char* eh_args_lookup(Eh_args* t, char* key);

// insert a value into an arg table.
// args   : an argument table returned by eh_opts_init.
// key         : the name of the command line option.
// value       : the value of the option.
void eh_args_insert(Eh_args* t, char* key, char* value);

// insert a default value into an arg table.
// args   : an argument table returned by eh_opts_init.
// key    : the name of the command line option.
// value  : the value of the option.
void eh_args_insert_default(Eh_args* t, char* key, char* value);

// check the command line options.
// args   : an argument table returned by eh_opts_init.
// required    : a null terminated list of required options or NULL if none required.
// possible    : a null terminated list of all the possible option of NULL to ignore
//             : the check.
// help_mesage : a help message to print to stderr if an error is encountered, or if
//             : help=yes is supplied on the command line.  null if no error message.
gboolean eh_check_opts(Eh_args* args, char** required, char** possible,
    const char** help_message);

// get the value (expresed as a string) given by the command line option, label or
// NULL if not given.
// args        : an argument table returned by eh_opts_init.
// label       : the command line option.
char* eh_get_opt(Eh_args* args, char* label);

// get the value (expresed as a string) given as a default value from a .ehrc file or
// NULL if not given.
// args        : an argument table returned by eh_opts_init.
// label       : the default option.
char* eh_get_opt_default(Eh_args* args, char* label);

// get the n-th argument on the command line.  an argument is different from an option
// as it is not of the form label=value; instead it is just a value.
// args   : an argument table returned by eh_opts_init.
// n      : the argument to return.
char* eh_get_arg_n(Eh_args* args, int n);

// print the current and default value of a command line option.
// args   : an argument table returned by eh_opts_init.
// label  : the command line option.
void eh_print_opt(Eh_args* args, char* label);

// get the string value given by the command line option, label.
// args   : an argument table returned by eh_opts_init.
// label       : the command line option.
// default_val : the default value
char* eh_get_opt_str(Eh_args* args, char* label, char* default_val);

// get the int value given by the command line option, label.
// args   : an argument table returned by eh_opts_init.
// label       : the command line option.
// default_val : the default value
int eh_get_opt_int(Eh_args* args, char* label, int default_val);

// get the boolean value given by the command line option, label.  possible values
// are yes or no.
// args   : an argument table returned by eh_opts_init.
// label       : the command line option.
// default_val : the default value
gboolean eh_get_opt_bool(Eh_args* args, char* label, gboolean default_val);

// get the key given by the command line option, label.
// args   : an argument table returned by eh_opts_init.
// label       : the command line option.
// default_val : the default value
// keys        : the possible key that can be given for the option.
int eh_get_opt_key(Eh_args* args, char* label, int default_val, char* keys[]);

// get the double value given by the command line option, label.
// args   : an argument table returned by eh_opts_init.
// label       : the command line option.
// default_val : the default value
double eh_get_opt_dbl(Eh_args* args, char* label, double default_val);

// print a message to the file pointer, fp
// fp          : a file pointer.
// message     : a null terminated list of text line to print.  a new line character is
//             : added to the end of each line weather there is already one there or not.
gint eh_print_message(FILE* fp, const char* message[]);

void eh_print_all_opts(Eh_args* args, char* prog_name, FILE* fp);

#ifdef __cplusplus
}
#endif

#endif

