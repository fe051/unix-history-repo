/*-
 * Copyright (c) 1994-1995 S�ren Schmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software withough specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef lint
static const char rcsid[] =
	"$Id$";
#endif /* not lint */

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <machine/console.h>
#include "path.h"
#include "lex.h"

char ctrl_names[32][4] = {
	"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
	"bs ", "ht ", "nl ", "vt ", "ff ", "cr ", "so ", "si ",
	"dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
	"can", "em ", "sub", "esc", "fs ", "gs ", "rs ", "ns "
	};

char fkey_table[96][MAXFK] = {
/* 01-04 */	"\033[M", "\033[N", "\033[O", "\033[P",
/* 05-08 */	"\033[Q", "\033[R", "\033[S", "\033[T",
/* 09-12 */	"\033[U", "\033[V", "\033[W", "\033[X",
/* 13-16 */	"\033[Y", "\033[Z", "\033[a", "\033[b",
/* 17-20 */	"\033[c", "\033[d", "\033[e", "\033[f",
/* 21-24 */	"\033[g", "\033[h", "\033[i", "\033[j",
/* 25-28 */	"\033[k", "\033[l", "\033[m", "\033[n",
/* 29-32 */	"\033[o", "\033[p", "\033[q", "\033[r",
/* 33-36 */	"\033[s", "\033[t", "\033[u", "\033[v",
/* 37-40 */	"\033[w", "\033[x", "\033[y", "\033[z",
/* 41-44 */	"\033[@", "\033[[", "\033[\\","\033[]",
/* 45-48 */     "\033[^", "\033[_", "\033[`", "\033[{",
/* 49-52 */	"\033[H", "\033[A", "\033[I", "-"     ,
/* 53-56 */	"\033[D", "\033[E", "\033[C", "+"     ,
/* 57-60 */	"\033[F", "\033[B", "\033[G", "\033[L",
/* 61-64 */     "\177",   "\033[J", "\033[~", "\033[}",
/* 65-68 */	""      , ""      , ""      , ""      ,
/* 69-72 */	""      , ""      , ""      , ""      ,
/* 73-76 */	""      , ""      , ""      , ""      ,
/* 77-80 */	""      , ""      , ""      , ""      ,
/* 81-84 */	""      , ""      , ""      , ""      ,
/* 85-88 */	""      , ""      , ""      , ""      ,
/* 89-92 */	""      , ""      , ""      , ""      ,
/* 93-96 */	""      , ""      , ""      , ""      ,
	};

const int	delays[]  = {250, 500, 750, 1000};
const int	repeats[] = { 34,  38,  42,  46,  50,  55,  59,  63,
			      68,  76,  84,  92, 100, 110, 118, 126,
			     136, 152, 168, 184, 200, 220, 236, 252,
			     272, 304, 336, 368, 400, 440, 472, 504};
const int	ndelays = (sizeof(delays) / sizeof(int));
const int	nrepeats = (sizeof(repeats) / sizeof(int));
int 		hex = 0;
int 		number;
char 		letter;

static void usage __P((void));

char *
nextarg(int ac, char **av, int *indp, int oc)
{
	if (*indp < ac)
		return(av[(*indp)++]);
	warnx("option requires two arguments -- %c", oc);
	usage();
	return("");
}


char *
mkfullname(const char *s1, const char *s2, const char *s3)
{
	static char	*buf = NULL;
	static int	bufl = 0;
	int		f;

	f = strlen(s1) + strlen(s2) + strlen(s3) + 1;
	if (f > bufl)
		if (buf)
			buf = (char *)realloc(buf, f);
		else
			buf = (char *)malloc(f);
	if (!buf) {
		bufl = 0;
		return(NULL);
	}

	bufl = f;
	strcpy(buf, s1);
	strcat(buf, s2);
	strcat(buf, s3);
	return(buf);
}


int
get_entry()
{
	switch (yylex()) {
	case TNOP:
		return NOP | 0x100;
	case TLSH:
		return LSH | 0x100;
	case TRSH:
		return RSH | 0x100;
	case TCLK:
		return CLK | 0x100;
	case TNLK:
		return NLK | 0x100;
	case TSLK:
		return SLK | 0x100;
	case TBTAB:
		return BTAB | 0x100;
	case TLALT:
		return LALT | 0x100;
	case TLCTR:
		return LCTR | 0x100;
	case TNEXT:
		return NEXT | 0x100;
	case TRCTR:
		return RCTR | 0x100;
	case TRALT:
		return RALT | 0x100;
	case TALK:
		return ALK | 0x100;
	case TASH:
		return ASH | 0x100;
	case TMETA:
		return META | 0x100;
	case TRBT:
		return RBT | 0x100;
	case TDBG:
		return DBG | 0x100;
	case TSUSP:
		return SUSP | 0x100;
	case TFUNC:
		if (F(number) > L_FN)
			return -1;
		return F(number) | 0x100;
	case TSCRN:
		if (S(number) > L_SCR)
			return -1;
		return S(number) | 0x100;
	case TLET:
		return (unsigned char)letter;
	case TNUM:
		if (number < 0 || number > 255)
			return -1;
		return number;
	default:
		return -1;
	}
}


int
get_key_definition_line(FILE* fd, keymap_t *map)
{
	int i, def, scancode;

	yyin = fd;

	/* get scancode number */
	if (yylex() != TNUM)
		return -1;
	if (number < 0 || number >= NUM_KEYS)
		return -1;
	scancode = number;

	/* get key definitions */
	map->key[scancode].spcl = 0;
	for (i=0; i<NUM_STATES; i++) {
		if ((def = get_entry()) == -1)
			return -1;
		if (def & 0x100)
			map->key[scancode].spcl |= (0x80 >> i);
		map->key[scancode].map[i] = def & 0xFF;
	}
	/* get lock state key def */
	if (yylex() != TFLAG)
		return -1;
	map->key[scancode].flgs = number;
		return scancode;
}


void
print_entry(FILE *fp, int value)
{
	int val = value & 0xFF;

	switch (value) {
	case NOP | 0x100:
		fprintf(fp, " nop   ");
		break;
	case LSH | 0x100:
		fprintf(fp, " lshift");
		break;
	case RSH | 0x100:
		fprintf(fp, " rshift");
		break;
	case CLK | 0x100:
		fprintf(fp, " clock ");
		break;
	case NLK | 0x100:
		fprintf(fp, " nlock ");
		break;
	case SLK | 0x100:
		fprintf(fp, " slock ");
		break;
	case BTAB | 0x100:
		fprintf(fp, " btab  ");
		break;
	case LALT | 0x100:
		fprintf(fp, " lalt  ");
		break;
	case LCTR | 0x100:
		fprintf(fp, " lctrl ");
		break;
	case NEXT | 0x100:
		fprintf(fp, " nscr  ");
		break;
	case RCTR | 0x100:
		fprintf(fp, " rctrl ");
		break;
	case RALT | 0x100:
		fprintf(fp, " ralt  ");
		break;
	case ALK | 0x100:
		fprintf(fp, " alock ");
		break;
	case ASH | 0x100:
		fprintf(fp, " ashift");
		break;
	case META | 0x100:
		fprintf(fp, " meta  ");
		break;
	case RBT | 0x100:
		fprintf(fp, " boot  ");
		break;
	case DBG | 0x100:
		fprintf(fp, " debug ");
		break;
	default:
		if (value & 0x100) {
		 	if (val >= F_FN && val <= L_FN)
				fprintf(fp, " fkey%02d", val - F_FN + 1);
		 	else if (val >= F_SCR && val <= L_SCR)
				fprintf(fp, " scr%02d ", val - F_SCR + 1);
			else if (hex)
				fprintf(fp, " 0x%02x  ", val);
			else
				fprintf(fp, "  %3d  ", val);
		}
		else {
			if (val < ' ')
				fprintf(fp, " %s   ", ctrl_names[val]);
			else if (val == 127)
				fprintf(fp, " del   ");
			else if (isascii(val) && isprint(val))
				fprintf(fp, " '%c'   ", val);
			else if (hex)
				fprintf(fp, " 0x%02x  ", val);
			else
				fprintf(fp, " %3d   ", val);
		}
	}
}


void
print_key_definition_line(FILE *fp, int scancode, struct key_t *key)
{
	int i;

	/* print scancode number */
	if (hex)
		fprintf(fp, " 0x%02x  ", scancode);
	else
		fprintf(fp, "  %03d  ", scancode);

	/* print key definitions */
	for (i=0; i<NUM_STATES; i++) {
		if (key->spcl & (0x80 >> i))
			print_entry(fp, key->map[i] | 0x100);
		else
			print_entry(fp, key->map[i]);
	}

	/* print lock state key def */
	switch (key->flgs) {
	case 0:
		fprintf(fp, "  O\n");
		break;
	case 1:
		fprintf(fp, "  C\n");
		break;
	case 2:
		fprintf(fp, "  N\n");
		break;
	case 3:
		fprintf(fp, "  B\n");
		break;
	}
}


void
load_keymap(char *opt, int dumponly)
{
	keymap_t map;
	FILE	*fd;
	int	scancode, i, j;
	struct key_t *kp;
	char	*name, *cp;
	char	*prefix[]  = {"", "", KEYMAP_PATH, NULL};
	char	*postfix[] = {"", ".kbd", ".kbd"};

	for (i=0; prefix[i]; i++) {
		name = mkfullname(prefix[i], opt, postfix[i]);
		if ((fd = fopen(name, "r")))
			break;
	}
	if (fd == NULL) {
		warn("keymap file not found");
		return;
	}
	memset(&map, 0, sizeof(map));
	while (1) {
		if ((scancode = get_key_definition_line(fd, &map)) < 0)
			break;
		if (scancode > map.n_keys) map.n_keys = scancode;
    	}
	if (dumponly) {
		/* fix up the filename to make it a valid C identifier */
		for (cp = opt; *cp; cp++)
			if (!isalpha(*cp) && !isdigit(*cp)) *cp = '_';
		printf("static const struct keymap keymap_%s = {\n"
		       "/*\n * Automatically generated from %s.\n"
		       " * DO NOT EDIT!\n */\n\n\t%u,\n{\n",
		       opt, name, (unsigned)map.n_keys);
		for (i = 0; i < map.n_keys; i++) {
			kp = &map.key[i];

			printf("\t{{ ");
			for (j = 0; j < NUM_STATES; j++)
				printf("0x%02x%c", (unsigned)kp->map[j],
					j == NUM_STATES-1? '}': ',');
			printf(", 0x%x, 0x%x },\n",
			       (unsigned)kp->spcl, (unsigned)kp->flgs);
		}
		printf("}\n};\n\n");
		return;
	}
	if (ioctl(0, PIO_KEYMAP, &map) < 0) {
		warn("setting keymap");
		fclose(fd);
		return;
	}
}


void
print_keymap()
{
	keymap_t map;
	int i;

	if (ioctl(0, GIO_KEYMAP, &map) < 0)
		err(1, "getting keymap");
    	printf(
"#                                                         alt\n"
"# scan                       cntrl          alt    alt   cntrl lock\n"
"# code  base   shift  cntrl  shift  alt    shift  cntrl  shift state\n"
"# ------------------------------------------------------------------\n"
    	);
	for (i=0; i<map.n_keys; i++)
		print_key_definition_line(stdout, i, &map.key[i]);
}


void
load_default_functionkeys()
{
	fkeyarg_t fkey;
	int i;

	for (i=0; i<NUM_FKEYS; i++) {
		fkey.keynum = i;
		strcpy(fkey.keydef, fkey_table[i]);
		fkey.flen = strlen(fkey_table[i]);
		if (ioctl(0, SETFKEY, &fkey) < 0)
			warn("setting function key");
	}
}

void
set_functionkey(char *keynumstr, char *string)
{
	fkeyarg_t fkey;

	if (!strcmp(keynumstr, "load") && !strcmp(string, "default")) {
		load_default_functionkeys();
		return;
	}
	fkey.keynum = atoi(keynumstr);
	if (fkey.keynum < 1 || fkey.keynum > NUM_FKEYS) {
		warnx("function key number must be between 1 and %d",
			NUM_FKEYS);
		return;
	}
	if ((fkey.flen = strlen(string)) > MAXFK) {
		warnx("function key string too long (%d > %d)",
			fkey.flen, MAXFK);
		return;
	}
	strcpy(fkey.keydef, string);
	fkey.keynum -= 1;
	if (ioctl(0, SETFKEY, &fkey) < 0)
		warn("setting function key");
}


void
set_bell_values(char *opt)
{
	int bell, duration, pitch;

	if (!strcmp(opt, "visual"))
		bell = 1, duration = 1, pitch = 800;
	else if (!strcmp(opt, "normal"))
		bell = 0, duration = 1, pitch = 800;
	else {
		char		*v1;

		bell = 0;
		duration = strtol(opt, &v1, 0);
		if ((duration < 0) || (*v1 != '.'))
			goto badopt;
		opt = ++v1;
		pitch = strtol(opt, &v1, 0);
		if ((pitch < 0) || (*opt == '\0') || (*v1 != '\0')) {
badopt:
			warnx("argument to -b must be DURATION.PITCH");
			return;
		}
	}

	ioctl(0, CONS_BELLTYPE, &bell);
	if (!bell)
		fprintf(stderr, "[=%d;%dB", pitch, duration);
}


void
set_keyrates(char *opt)
{
struct	{
	int	rep:5;
	int	del:2;
	int	pad:1;
	}rate;

	if (!strcmp(opt, "slow"))
		rate.del = 3, rate.rep = 31;
	else if (!strcmp(opt, "normal"))
		rate.del = 1, rate.rep = 15;
	else if (!strcmp(opt, "fast"))
		rate.del = rate.rep = 0;
	else {
		int		n;
		int		delay, repeat;
		char		*v1;

		delay = strtol(opt, &v1, 0);
		if ((delay < 0) || (*v1 != '.'))
			goto badopt;
		opt = ++v1;
		repeat = strtol(opt, &v1, 0);
		if ((repeat < 0) || (*opt == '\0') || (*v1 != '\0')) {
badopt:
			warnx("argument to -r must be delay.repeat");
			return;
		}
		for (n = 0; n < ndelays - 1; n++)
			if (delay <= delays[n])
				break;
		rate.del = n;
		for (n = 0; n < nrepeats - 1; n++)
			if (repeat <= repeats[n])
				break;
		rate.rep = n;
	}

	if (ioctl(0, KDSETRAD, rate) < 0)
		warn("setting keyboard rate");
}


void
set_history(char *opt)
{
	int size;

	size = atoi(opt);
	if ((*opt == '\0') || size < 0) {
		warnx("argument must be a positive number");
		return;
	}
	if (ioctl(0, CONS_HISTORY, &size) == -1)
		warn("setting history buffer size");
}


static void
usage()
{
	fprintf(stderr, "%s\n%s\n%s\n",
"usage: kbdcontrol [-dFx] [-b  duration.pitch | belltype]",
"                  [-r delay.repeat | speed] [-l mapfile] [-f # string]",
"                  [-h size] [-L mapfile]");
	exit(1);
}


void
main(int argc, char **argv)
{
	int		opt;

	while((opt = getopt(argc, argv, "b:df:h:Fl:L:r:x")) != -1)
		switch(opt) {
			case 'b':
				set_bell_values(optarg);
				break;
			case 'd':
				print_keymap();
				break;
			case 'l':
				load_keymap(optarg, 0);
				break;
			case 'L':
				load_keymap(optarg, 1);
				break;
			case 'f':
				set_functionkey(optarg,
					nextarg(argc, argv, &optind, 'f'));
				break;
			case 'F':
				load_default_functionkeys();
				break;
			case 'h':
				set_history(optarg);
				break;
			case 'r':
				set_keyrates(optarg);
				break;
			case 'x':
				hex = 1;
				break;
			default:
				usage();
		}
	if ((optind != argc) || (argc == 1))
		usage();
	exit(0);
}


