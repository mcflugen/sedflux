#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

void
print_choices(int sig_num);

int
sedflux_set_signal_action(void)
{
    gint rtn_val;
    struct sigaction new_action, old_action;

    /* Set the new action */
    new_action.sa_handler = print_choices;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    /* If the signal has already been set to be ignored, respect that */
    rtn_val = sigaction(SIGINT, NULL, &old_action);

    if (old_action.sa_handler != SIG_IGN) {
        rtn_val = sigaction(SIGINT, &new_action, NULL);
    }

    return rtn_val;
}

static gboolean __quit_signal =
    FALSE; //< signal to indicate that the user wishes to quit.
static gboolean __dump_signal =
    FALSE; //< signal to indicate that the user wishes to dump output.
static gboolean __cpr_signal  =
    FALSE; //< signal to indicate that the user wishes to create a checkpoint.

gboolean
sedflux_signal_is_pending(Sedflux_sig_num sig)
{
    gboolean is_pending = FALSE;

    if (sig == SEDFLUX_SIG_QUIT && __quit_signal) {
        is_pending = TRUE;
    }

    if (sig == SEDFLUX_SIG_DUMP && __dump_signal) {
        is_pending = TRUE;
    }

    if (sig == SEDFLUX_SIG_CPR  && __cpr_signal) {
        is_pending = TRUE;
    }

    return is_pending;
}

void
sedflux_signal_reset(Sedflux_sig_num sig)
{
    if (sig & SEDFLUX_SIG_QUIT && __quit_signal) {
        __quit_signal = FALSE;
    }

    if (sig & SEDFLUX_SIG_DUMP && __dump_signal) {
        __dump_signal = FALSE;
    }

    if (sig & SEDFLUX_SIG_CPR  && __cpr_signal) {
        __cpr_signal  = FALSE;
    }

    return;
}

void
sedflux_signal_set(Sedflux_sig_num sig)
{
    if (sig & SEDFLUX_SIG_QUIT) {
        __quit_signal = TRUE;
    }

    if (sig & SEDFLUX_SIG_DUMP) {
        __dump_signal = TRUE;
    }

    if (sig & SEDFLUX_SIG_CPR) {
        __cpr_signal  = TRUE;
    }

    return;
}

void
print_choices(int sig_num)
{
    gboolean is_invalid = FALSE;
    char     ch;

    fprintf(stdout, "\n");
    fprintf(stdout, "-----------------------------------------------\n");
    fprintf(stdout, "  (1) End run after this time step.\n");
    fprintf(stdout, "  (2) Dump results and continue.\n");
    fprintf(stdout, "  (3) Create a checkpoint.\n");
    fprintf(stdout, "  (4) Continue.\n");
    fprintf(stdout, "  (5) Quit immediatly (without saving).\n");
    fprintf(stdout, "-----------------------------------------------\n");

    do {
        fprintf(stdout, "   Your choice [4]? ");

        is_invalid = FALSE;

        fscanf(stdin, "%s", &ch);

        if (g_ascii_strcasecmp(&ch, "1") == 0) {
            sedflux_signal_set(SEDFLUX_SIG_QUIT);
            fprintf(stdout, "   You have opted to quit early...\n\n");
        } else if (g_ascii_strcasecmp(&ch, "2") == 0) {
            sedflux_signal_set(SEDFLUX_SIG_DUMP);
            fprintf(stdout,
                "   Temporary output files will be dumped at the end of this time step...\n\n");
        } else if (g_ascii_strcasecmp(&ch, "3") == 0) {
            sedflux_signal_set(SEDFLUX_SIG_CPR);
            fprintf(stdout, "   Creating a checkpoint/reset file...\n\n");
        } else if (g_ascii_strcasecmp(&ch, "4") == 0
            || g_ascii_strcasecmp(&ch, "") == 0) {
            fprintf(stdout, "   Continuing...\n\n");
        } else if (g_ascii_strcasecmp(&ch, "5") == 0) {
            fprintf(stdout, "   Terminating run without saving...\n\n");
            eh_exit(EXIT_SUCCESS);
        } else {
            is_invalid = TRUE;
        }
    } while (is_invalid);

    return;
}


