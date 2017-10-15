/****************************************************************************
 * Copyright (c) 2007-2013,2017 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/*
 * Author: Thomas E. Dickey - 2007
 *
 * $Id: dots_mvcur.c,v 1.15 2017/10/11 08:15:46 tom Exp $
 *
 * A simple demo of the terminfo interface, and mvcur.
 */
#define USE_TINFO
#include <test.priv.h>

#if HAVE_SETUPTERM

#include <time.h>

static bool interrupted = FALSE;
static long total_chars = 0;
static time_t started;

static
TPUTS_PROTO(outc, c)
{
    int rc = c;

    if (interrupted) {
	char tmp = (char) c;
	if (write(STDOUT_FILENO, &tmp, (size_t) 1) == -1)
	    rc = EOF;
    } else {
	if (putc(c, stdout) == EOF)
	    rc = EOF;
    }
    TPUTS_RETURN(rc);
}

static bool
outs(const char *s)
{
    if (VALID_STRING(s)) {
	tputs(s, 1, outc);
	return TRUE;
    }
    return FALSE;
}

static void
cleanup(void)
{
    outs(exit_attribute_mode);
    if (!outs(orig_colors))
	outs(orig_pair);
    outs(clear_screen);
    outs(cursor_normal);

    printf("\n\n%ld total cells, rate %.2f/sec\n",
	   total_chars,
	   ((double) (total_chars) / (double) (time((time_t *) 0) - started)));
}

static void
onsig(int n GCC_UNUSED)
{
    interrupted = TRUE;
}

static double
ranf(void)
{
    long r = (rand() & 077777);
    return ((double) r / 32768.);
}

int
main(int argc GCC_UNUSED,
     char *argv[]GCC_UNUSED)
{
    int x0 = 1, y0 = 1;
    int x, y, z, p;
    double r;
    double c;
    SCREEN *sp;
    int my_colors;

    InitAndCatch((sp = newterm((char *) 0, stdout, stdin)), onsig);
    refresh();			/* needed with Solaris curses to cancel endwin */

    if (sp == 0) {
	fprintf(stderr, "Cannot initialize terminal\n");
	ExitProgram(EXIT_FAILURE);
    }

    srand((unsigned) time(0));

    outs(clear_screen);
    outs(cursor_home);
    outs(cursor_invisible);
    my_colors = max_colors;
    if (my_colors > 1) {
	if (!VALID_STRING(set_a_foreground)
	    || !VALID_STRING(set_a_background)
	    || (!VALID_STRING(orig_colors) && !VALID_STRING(orig_pair)))
	    my_colors = -1;
    }

    r = (double) (lines - 4);
    c = (double) (columns - 4);
    started = time((time_t *) 0);

    while (!interrupted) {
	x = (int) (c * ranf()) + 2;
	y = (int) (r * ranf()) + 2;
	p = (ranf() > 0.9) ? '*' : ' ';

	if (mvcur(y0, x0, y, x) != ERR) {
	    x0 = x;
	    y0 = y;
	}

	if (my_colors > 0) {
	    z = (int) (ranf() * my_colors);
	    if (ranf() > 0.01) {
		tputs(tparm2(set_a_foreground, z), 1, outc);
	    } else {
		tputs(tparm2(set_a_background, z), 1, outc);
		napms(1);
	    }
	} else if (VALID_STRING(exit_attribute_mode)
		   && VALID_STRING(enter_reverse_mode)) {
	    if (ranf() <= 0.01) {
		outs((ranf() > 0.6)
		     ? enter_reverse_mode
		     : exit_attribute_mode);
		napms(1);
	    }
	}
	outc(p);
	++x0;
	fflush(stdout);
	++total_chars;
    }
    cleanup();
    endwin();
    delscreen(sp);
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(int argc GCC_UNUSED,
     char *argv[]GCC_UNUSED)
{
    fprintf(stderr, "This program requires terminfo\n");
    exit(EXIT_FAILURE);
}
#endif
