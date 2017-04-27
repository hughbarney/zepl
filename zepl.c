/* zepl.c,  Zep Emacs, Public Domain, Hugh Barney, 2017, Derived from: Anthony's Editor January 93 */

#include <stdlib.h>
#include <assert.h>
#include <curses.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define E_NAME          "zepl"
#define E_VERSION       "v0.9"
#define E_LABEL         "Zepl:"
#define E_NOT_BOUND	"<not bound>"
#define E_INITFILE      "zepl.rc"

#define B_MODIFIED	0x01		/* modified buffer */
#define MSGLINE         (LINES-1)
#define CHUNK           8096L
#define K_BUFFER_LENGTH 256
#define MAX_FNAME       256
#define TEMPBUF         512
#define MIN_GAP_EXPAND  512
#define NOMARK          -1
#define STRBUF_M        64
#define MAX_KNAME       12
#define MAX_KBYTES      12
#define MAX_KFUNC       30

typedef unsigned char char_t;
typedef long point_t;

typedef struct keymap_t {
	char k_name[MAX_KNAME + 1];       /* name of key eg "c-c c-f" */
	char k_bytes[MAX_KNAME + 1];      /* bytes of key sequence */
	char k_funcname[MAX_KFUNC + 1];   /* name of function, eg (forward-char) */
	void (*k_func)(void);             /* function pointer */
	struct keymap_t *k_next;         /* link to next keymap_t */
} keymap_t;

typedef struct buffer_t
{
	point_t b_mark;	     	  /* the mark */
	point_t b_point;          /* the point */
	point_t b_page;           /* start of page */
	point_t b_epage;          /* end of page */
	char_t *b_buf;            /* start of buffer */
	char_t *b_ebuf;           /* end of buffer */
	char_t *b_gap;            /* start of gap */
	char_t *b_egap;           /* end of gap */
	char w_top;	          /* Origin 0 top row of window */
	char w_rows;              /* no. of rows of text in window */
	int b_row;                /* cursor row */
	int b_col;                /* cursor col */
	char b_fname[MAX_FNAME + 1]; /* filename */
	char b_flags;             /* buffer flags */
} buffer_t;

/*
 * Some compilers define size_t as a unsigned 16 bit number while
 * point_t and off_t might be defined as a signed 32 bit number.  
 * malloc(), realloc(), fread(), and fwrite() take size_t parameters,
 * which means there will be some size limits because size_t is too
 * small of a type.
 */
#define MAX_SIZE_T      ((unsigned long) (size_t) ~0)

int done;
char_t *input;
int msgflag;
char msgline[TEMPBUF];
char temp[TEMPBUF];
keymap_t *key_return;
keymap_t *khead = NULL;
keymap_t *ktail = NULL;
buffer_t *curbp;
point_t nscrap = 0;
char_t *scrap = NULL;
char searchtext[STRBUF_M];

buffer_t* new_buffer()
{
	buffer_t *bp = (buffer_t *)malloc(sizeof(buffer_t));
	assert(bp != NULL);
	bp->b_point = 0;
	bp->b_mark = NOMARK;
	bp->b_page = 0;
	bp->b_epage = 0;
	bp->b_flags = 0;
	bp->b_buf = NULL;
	bp->b_ebuf = NULL;
	bp->b_gap = NULL;
	bp->b_egap = NULL;
	bp->b_fname[0] = '\0';
	bp->w_top = 0;	
	bp->w_rows = LINES - 2;
	return bp;
}

void fatal(char *msg)
{
	noraw();
	endwin();
	printf("\n%s %s:\n%s\n", E_NAME, E_VERSION, msg);
	exit(1);
}

int msg(char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	(void)vsprintf(msgline, msg, args);
	va_end(args);
	msgflag = TRUE;
	return FALSE;
}

/* Given a buffer offset, convert it to a pointer into the buffer */
char_t * ptr(buffer_t *bp, register point_t offset)
{
	if (offset < 0) return (bp->b_buf);
	return (bp->b_buf+offset + (bp->b_buf + offset < bp->b_gap ? 0 : bp->b_egap-bp->b_gap));
}

/* Given a pointer into the buffer, convert it to a buffer offset */
point_t pos(buffer_t *bp, register char_t *cp)
{
	assert(bp->b_buf <= cp && cp <= bp->b_ebuf);
	return (cp - bp->b_buf - (cp < bp->b_egap ? 0 : bp->b_egap - bp->b_gap));
}

/* Enlarge gap by n chars, position of gap cannot change */
int growgap(buffer_t *bp, point_t n)
{
	char_t *new;
	point_t buflen, newlen, xgap, xegap;
		
	assert(bp->b_buf <= bp->b_gap);
	assert(bp->b_gap <= bp->b_egap);
	assert(bp->b_egap <= bp->b_ebuf);

	xgap = bp->b_gap - bp->b_buf;
	xegap = bp->b_egap - bp->b_buf;
	buflen = bp->b_ebuf - bp->b_buf;
    
	/* reduce number of reallocs by growing by a minimum amount */
	n = (n < MIN_GAP_EXPAND ? MIN_GAP_EXPAND : n);
	newlen = buflen + n * sizeof (char_t);

	if (buflen == 0) {
		if (newlen < 0 || MAX_SIZE_T < newlen) fatal("Failed to allocate required memory.\n");
		new = (char_t*) malloc((size_t) newlen);
		if (new == NULL) fatal("Failed to allocate required memory.\n");
	} else {
		if (newlen < 0 || MAX_SIZE_T < newlen) return msg("Failed to allocate required memory");
		new = (char_t*) realloc(bp->b_buf, (size_t) newlen);
		if (new == NULL) return msg("Failed to allocate required memory");
	}

	/* Relocate pointers in new buffer and append the new
	 * extension to the end of the gap.
	 */
	bp->b_buf = new;
	bp->b_gap = bp->b_buf + xgap;      
	bp->b_ebuf = bp->b_buf + buflen;
	bp->b_egap = bp->b_buf + newlen;
	while (xegap < buflen--)
		*--bp->b_egap = *--bp->b_ebuf;
	bp->b_ebuf = bp->b_buf + newlen;

	assert(bp->b_buf < bp->b_ebuf);          /* Buffer must exist. */
	assert(bp->b_buf <= bp->b_gap);
	assert(bp->b_gap < bp->b_egap);          /* Gap must grow only. */
	assert(bp->b_egap <= bp->b_ebuf);
	return (TRUE);
}

point_t movegap(buffer_t *bp, point_t offset)
{
	char_t *p = ptr(bp, offset);
	while (p < bp->b_gap)
		*--bp->b_egap = *--bp->b_gap;
	while (bp->b_egap < p)
		*bp->b_gap++ = *bp->b_egap++;
	assert(bp->b_gap <= bp->b_egap);
	assert(bp->b_buf <= bp->b_gap);
	assert(bp->b_egap <= bp->b_ebuf);
	return (pos(bp, bp->b_egap));
}

void save_buffer()
{
	FILE *fp;
	point_t length;

	fp = fopen(curbp->b_fname, "w");
	if (fp == NULL) msg("Failed to open file \"%s\".", curbp->b_fname);
	(void) movegap(curbp, (point_t) 0);
	length = (point_t) (curbp->b_ebuf - curbp->b_egap);
	if (fwrite(curbp->b_egap, sizeof (char), (size_t) length, fp) != length) 
		msg("Failed to write file \"%s\".", curbp->b_fname);
	fclose(fp);
	curbp->b_flags &= ~B_MODIFIED;
	msg("File \"%s\" %ld bytes saved.", curbp->b_fname, pos(curbp, curbp->b_ebuf));
}

/* reads file into buffer at point */
int insert_file(char *fn, int modflag)
{
	FILE *fp;
	size_t len;
	struct stat sb;

	if (stat(fn, &sb) < 0) return msg("Failed to find file \"%s\".", fn);
	if (MAX_SIZE_T < sb.st_size) msg("File \"%s\" is too big to load.", fn);

	if (curbp->b_egap - curbp->b_gap < sb.st_size * sizeof (char_t) && !growgap(curbp, sb.st_size))
		return (FALSE);
	if ((fp = fopen(fn, "r")) == NULL) return msg("Failed to open file \"%s\".", fn);

	curbp->b_point = movegap(curbp, curbp->b_point);
	curbp->b_gap += len = fread(curbp->b_gap, sizeof (char), (size_t) sb.st_size, fp);

	if (fclose(fp) != 0) return msg("Failed to close file \"%s\".", fn);
	curbp->b_flags &= (modflag ? B_MODIFIED : ~B_MODIFIED);
	msg("File \"%s\" %ld bytes read.", fn, len);
	return (TRUE);
}

char_t *get_key(keymap_t *keys, keymap_t **key_return)
{
	keymap_t *k;
	int submatch;
	static char_t buffer[K_BUFFER_LENGTH];
	static char_t *record = buffer;

	*key_return = NULL;

	/* if recorded bytes remain, return next recorded byte. */
	if (*record != '\0') {
		*key_return = NULL;
		return record++;
	}
	/* reset record buffer. */
	record = buffer;

	do {
		assert(K_BUFFER_LENGTH > record - buffer);
		/* read and record one byte. */
		*record++ = (unsigned)getch();
		*record = '\0';

		/* if recorded bytes match any multi-byte sequence... */
		for (k = keys, submatch = 0; k != NULL; k = k->k_next) {
			char_t *p, *q;

			if (k->k_func == NULL) continue;
			assert(k->k_bytes != NULL);

			for (p = buffer, q = (char_t *)k->k_bytes; *p == *q; ++p, ++q) {
			        /* an exact match */
				if (*q == '\0' && *p == '\0') {
	    				record = buffer;
					*record = '\0';
					*key_return = k;
					return record; /* empty string */
				}
			}
			/* record bytes match part of a command sequence */
			if (*p == '\0' && *q != '\0') {
				submatch = 1;
			}
		}
	} while (submatch);
	/* nothing matched, return recorded bytes. */
	record = buffer;
	return (record++);
}

/* Reverse scan for start of logical line containing offset */
point_t lnstart(buffer_t *bp, register point_t off)
{
	register char_t *p;
	do
		p = ptr(bp, --off);
	while (bp->b_buf < p && *p != '\n');
	return (bp->b_buf < p ? ++off : 0);
}

/* Forward scan for start of logical line segment containing 'finish' */
point_t segstart(buffer_t *bp, point_t start, point_t finish)
{
	char_t *p;
	int c = 0;
	point_t scan = start;

	while (scan < finish) {
		p = ptr(bp, scan);
		if (*p == '\n') {
			c = 0;
			start = scan+1;
		} else if (COLS <= c) {
			c = 0;
			start = scan;
		}
		++scan;
		c += *p == '\t' ? 8 - (c & 7) : 1;
	}
	return (c < COLS ? start : finish);
}

/* Forward scan for start of logical line segment following 'finish' */
point_t segnext(buffer_t *bp, point_t start, point_t finish)
{
	char_t *p;
	int c = 0;

	point_t scan = segstart(bp, start, finish);
	for (;;) {
		p = ptr(bp, scan);
		if (bp->b_ebuf <= p || COLS <= c)
			break;
		++scan;
		if (*p == '\n')
			break;
		c += *p == '\t' ? 8 - (c & 7) : 1;
	}
	return (p < bp->b_ebuf ? scan : pos(bp, bp->b_ebuf));
}

/* Move up one screen line */
point_t upup(buffer_t *bp, point_t off)
{
	point_t curr = lnstart(bp, off);
	point_t seg = segstart(bp, curr, off);
	if (curr < seg)
		off = segstart(bp, curr, seg-1);
	else
		off = segstart(bp, lnstart(bp,curr-1), curr-1);
	return (off);
}

/* Move down one screen line */
point_t dndn(buffer_t *bp, point_t off)
{
	return (segnext(bp, lnstart(bp,off), off));
}

/* Return the offset of a column on the specified line */
point_t lncolumn(buffer_t *bp, point_t offset, int column)
{
	int c = 0;
	char_t *p;
	while ((p = ptr(bp, offset)) < bp->b_ebuf && *p != '\n' && c < column) {
		c += *p == '\t' ? 8 - (c & 7) : 1;
		++offset;
	}
	return (offset);
}

void modeline(buffer_t *bp)
{
	int i;
	char mch;
	
	standout();
	move(bp->w_top + bp->w_rows, 0);
	mch = ((bp->b_flags & B_MODIFIED) ? '*' : '=');
	sprintf(temp, "=%c " E_LABEL " == %s ", mch, bp->b_fname);
	addstr(temp);

	for (i = strlen(temp) + 1; i <= COLS; i++)
		addch('=');
	standend();
}

void dispmsg()
{
	move(MSGLINE, 0);
	if (msgflag) {
		addstr(msgline);
		msgflag = FALSE;
	}
	clrtoeol();
}

void display()
{
	char_t *p;
	int i, j, k;
	buffer_t *bp = curbp;
	
	/* find start of screen, handle scroll up off page or top of file  */
	/* point is always within b_page and b_epage */
	if (bp->b_point < bp->b_page)
		bp->b_page = segstart(bp, lnstart(bp,bp->b_point), bp->b_point);

	/* reframe when scrolled off bottom */
	if (bp->b_epage <= bp->b_point) {
		/* Find end of screen plus one. */
		bp->b_page = dndn(bp, bp->b_point);
		/* if we scoll to EOF we show 1 blank line at bottom of screen */
		if (pos(bp, bp->b_ebuf) <= bp->b_page) {
			bp->b_page = pos(bp, bp->b_ebuf);
			i = bp->w_rows - 1;
		} else {
			i = bp->w_rows - 0;
		}
		/* Scan backwards the required number of lines. */
		while (0 < i--)
			bp->b_page = upup(bp, bp->b_page);
	}

	move(bp->w_top, 0); /* start from top of window */
	i = bp->w_top;
	j = 0;
	bp->b_epage = bp->b_page;
	
	/* paint screen from top of page until we hit maxline */ 
	while (1) {
		/* reached point - store the cursor position */
		if (bp->b_point == bp->b_epage) {
			bp->b_row = i;
			bp->b_col = j;
		}
		p = ptr(bp, bp->b_epage);
		if (bp->w_top + bp->w_rows <= i || bp->b_ebuf <= p) /* maxline */
			break;
		if (*p != '\r') {
			if (isprint(*p) || *p == '\t' || *p == '\n') {
				j += *p == '\t' ? 8-(j&7) : 1;
				addch(*p);
			} else {
				const char *ctrl = unctrl(*p);
				j += (int) strlen(ctrl);
				addstr(ctrl);
			}
		}
		if (*p == '\n' || COLS <= j) {
			j -= COLS;
			if (j < 0)
				j = 0;
			++i;
		}
		++bp->b_epage;
	}

	/* replacement for clrtobot() to bottom of window */
	for (k=i; k < bp->w_top + bp->w_rows; k++) {
		move(k, j); /* clear from very last char not start of line */
		clrtoeol();
		j = 0; /* thereafter start of line */
	}

	modeline(bp);
	dispmsg();
	move(bp->b_row, bp->b_col); /* set cursor */
	refresh();
}

void display_prompt_and_response(char *prompt, char *response)
{
	mvaddstr(MSGLINE, 0, prompt);
	addstr(response);
	clrtoeol();
}

void top() { curbp->b_point = 0; }
void bottom() {	curbp->b_epage = curbp->b_point = pos(curbp, curbp->b_ebuf); }
void left() { if (0 < curbp->b_point) --curbp->b_point; }
void right() { if (curbp->b_point < pos(curbp, curbp->b_ebuf)) ++curbp->b_point; }
void up() { curbp->b_point = lncolumn(curbp, upup(curbp, curbp->b_point),curbp->b_col); }
void down() { curbp->b_point = lncolumn(curbp, dndn(curbp, curbp->b_point),curbp->b_col); }
void lnbegin() { curbp->b_point = segstart(curbp, lnstart(curbp,curbp->b_point), curbp->b_point); }
void quit() { done = 1; }
void resize_terminal() { curbp->w_rows = LINES - 2; }

void lnend()
{
	curbp->b_point = dndn(curbp, curbp->b_point);
	left();
}

void pgdown()
{
	curbp->b_page = curbp->b_point = upup(curbp, curbp->b_epage);
	while (0 < curbp->b_row--)
		down();
	curbp->b_epage = pos(curbp, curbp->b_ebuf);
}

void pgup()
{
	int i = curbp->w_rows;
	while (0 < --i) {
		curbp->b_page = upup(curbp, curbp->b_page);
		up();
	}
}

void insert()
{
	assert(curbp->b_gap <= curbp->b_egap);
	if (curbp->b_gap == curbp->b_egap && !growgap(curbp, CHUNK)) return;
	curbp->b_point = movegap(curbp, curbp->b_point);
	*curbp->b_gap++ = *input == '\r' ? '\n' : *input;
	curbp->b_point = pos(curbp, curbp->b_egap);
	curbp->b_flags |= B_MODIFIED;
}

void backspace()
{
	curbp->b_point = movegap(curbp, curbp->b_point);
	if (curbp->b_buf < curbp->b_gap) {
		--curbp->b_gap;
		curbp->b_flags |= B_MODIFIED;
	}
	curbp->b_point = pos(curbp, curbp->b_egap);
}

void delete()
{
	curbp->b_point = movegap(curbp, curbp->b_point);
	if (curbp->b_egap < curbp->b_ebuf) {
		curbp->b_point = pos(curbp, ++curbp->b_egap);
		curbp->b_flags |= B_MODIFIED;
	}
}

void set_mark()
{
	curbp->b_mark = (curbp->b_mark == curbp->b_point ? NOMARK : curbp->b_point);
	(curbp->b_mark != NOMARK) ? msg("Mark set") : msg("Mark cleared");
}

void copy_cut(int cut, int verbose)
{
	char_t *p;
	if (scrap != NULL) {
		free(scrap);
		scrap = NULL;
		nscrap = 0;
	}
	/* if no mark or point == marker, nothing doing */
	if (curbp->b_mark == NOMARK || curbp->b_point == curbp->b_mark) return;

	if (curbp->b_point < curbp->b_mark) {
		/* point above marker: move gap under point, region = marker - point */
		(void)movegap(curbp, curbp->b_point);
		p = ptr(curbp, curbp->b_point);
		nscrap = curbp->b_mark - curbp->b_point;
	} else {
		/* if point below marker: move gap under marker, region = point - marker */
		(void)movegap(curbp, curbp->b_mark);
		p = ptr(curbp, curbp->b_mark);
		nscrap = curbp->b_point - curbp->b_mark;
	}
	assert(nscrap > 0);
	if ((scrap = (char_t*) malloc(nscrap + 1)) == NULL) {
		msg("No more memory available.");
	} else {
		(void)memcpy(scrap, p, nscrap * sizeof (char_t));
		*(scrap + nscrap) = '\0';  /* null terminate for insert_string */
		if (cut) {
			curbp->b_egap += nscrap; /* if cut expand gap down */
			curbp->b_point = pos(curbp, curbp->b_egap); /* set point to after region */
			curbp->b_flags |= B_MODIFIED;
			if (verbose) msg("%ld bytes cut.", nscrap);
		} else {
			if (verbose) msg("%ld bytes copied.", nscrap);
		}
		curbp->b_mark = NOMARK;  /* unmark */
	}
}

void insert_string(char *str)
{
	int len = (str == NULL) ? 0 : strlen(str);

	if (len <= 0) {
		msg("nothing to insert");
	} else if (len < curbp->b_egap - curbp->b_gap || growgap(curbp, len)) {
		curbp->b_point = movegap(curbp, curbp->b_point);
		memcpy(curbp->b_gap, str, len * sizeof (char_t));
		curbp->b_gap += len;
		curbp->b_point = pos(curbp, curbp->b_egap);
		curbp->b_flags |= B_MODIFIED;
	}
}

void yank() { insert_string((char *)scrap); }
void copy_region() { copy_cut(FALSE, TRUE); }
void kill_region() { copy_cut(TRUE, TRUE); }

/* return char at current point */
char *get_char()
{
	static char ch[2] = "\0\0";
	ch[0] = (char)*(ptr(curbp, curbp->b_point));
	return ch;
}

/* wrapper to simplify call and dependancies in the interface code */
char *get_input_key() {	return (char *)get_key(khead, &key_return); }
/* the name of the bound function of this key */
char *get_key_funcname() { return (key_return != NULL ? key_return->k_funcname : ""); }
/* the name of the last key */
char *get_key_name() { return (key_return != NULL ? key_return->k_name : ""); }
/* return point in current buffer */
point_t get_point() { return curbp->b_point; }

void set_point(point_t p)
{
	if (p < 0 || p > pos(curbp, curbp->b_ebuf)) return;
	curbp->b_point = p;
}

point_t search_forward(buffer_t *bp, point_t start_p, char *stext)
{
	point_t end_p = pos(bp, bp->b_ebuf);
	point_t p,pp;
	char* s;

	if (0 == strlen(stext)) return start_p;

	for (p=start_p; p < end_p; p++) {
		for (s=stext, pp=p; *s == *(ptr(bp, pp)) && *s !='\0' && pp < end_p; s++, pp++)
			;
		if (*s == '\0') return pp;
	}
	return -1;
}

point_t search_forward_curbp(point_t start_p, char *stext) {
	return search_forward(curbp, start_p, stext);
}

void user_func(void);

keymap_t *new_key(char *name, char *bytes)
{
	keymap_t *kp = (keymap_t *)malloc(sizeof(keymap_t));
	assert(kp != NULL);

 	strncpy(kp->k_name, name, MAX_KNAME);
	strncpy(kp->k_bytes, bytes, MAX_KBYTES);
	kp->k_name[MAX_KNAME] ='\0';
	kp->k_bytes[MAX_KBYTES] ='\0';
	kp->k_func = user_func;
	strcpy(kp->k_funcname, E_NOT_BOUND);
	kp->k_next = NULL;
	return kp;
}

/* note, no check if name already exists */
void make_key(char *name, char *bytes)
{
	keymap_t *kp = new_key(name, bytes);
	ktail->k_next = kp;
	ktail = kp;
}

void create_keys()
{
	char ch;
	char ctrx_map[] = "c-x c-|";
	char ctrl_map[] = "c-|";
	char esc_map[] = "esc-|";
	char ctrx_bytes[] = "\x18\x01";
	char ctrl_bytes[] = "\x01";
	char esc_bytes[] = "\x1B\x61";

	assert(khead == NULL);
	khead = ktail = new_key("c-space", "\x00");

	/* control-a to z */
	for (ch = 1; ch <= 26; ch++) {
		if (ch == 9 || ch == 10 || ch == 24) continue;  /* skip tab, linefeed, ctrl-x */
		ctrl_map[2] = ch + 96;   /* ASCII a is 97 */
		ctrl_bytes[0] = ch;
		make_key(ctrl_map, ctrl_bytes);
	}

	/* esc-a to z */
	for (ch = 1; ch <= 26; ch++) {
		esc_map[4] = ch + 96;
		esc_bytes[1] = ch + 96;
		make_key(esc_map, esc_bytes);
	}

	/* control-x control-a to z */
	for (ch = 1; ch <= 26; ch++) {
		ctrx_map[6] = ch + 96;
		ctrx_bytes[1] = ch;
		make_key(ctrx_map, ctrx_bytes);
	}

	make_key("c-x ?", "\x18\x3F");
}

int set_key_internal(char *name, char *funcname, char *bytes, void (*func)(void))
{
	keymap_t *kp;
	
	for (kp = khead; kp->k_next != NULL; kp = kp->k_next) {
		if (0 == strcmp(kp->k_name, name)) {
			strncpy(kp->k_funcname, funcname, MAX_KFUNC);
			kp->k_funcname[MAX_KFUNC] ='\0';
			if (func != NULL)  /* dont set if its a user_func */
				kp->k_func = func;
			return 1;
		}
	}

	/* not found, create it and add onto the tail */
	if (func != NULL) {
		kp = new_key(name, bytes);
		strncpy(kp->k_funcname, funcname, MAX_KFUNC);
		kp->k_funcname[MAX_KFUNC] ='\0';
		kp->k_func = func;
		ktail->k_next = kp;
		ktail = kp;
		return 1;
	}
	return 0;
}

int set_key(char *name, char *funcname)
{
	return set_key_internal(name, funcname, "", NULL);
}

extern char *load_file(int);
extern char *call_lisp(char *);
extern void init_lisp(void);
extern void reset_output_stream();

void eval_block()
{
	char *output;

	if (curbp->b_mark == NOMARK || curbp->b_mark >= curbp->b_point) {
		msg("no block defined");
		return;
	}

	copy_cut(FALSE, FALSE);
	assert(scrap != NULL);
	assert(strlen(scrap) > 0);

	reset_output_stream();
	output = call_lisp((char *)scrap);
	insert_string("\n");
	insert_string(output);
	reset_output_stream();
}

void user_func()
{
	char *output;
	assert(key_return != NULL);
	if (0 == strcmp(key_return->k_funcname, E_NOT_BOUND)) {
		msg(E_NOT_BOUND);
		return;
	}

	reset_output_stream();
	output = call_lisp(key_return->k_funcname);

	/* show errors on message line */
	if (NULL != strstr(output, "error:")) {
		char buf[81];
		strncpy(buf, output, 80);
		buf[80] ='\0';
		msg(buf);
	}	
	reset_output_stream();
}

void load_config()
{
	char fname[300];
	char *output;
	int fd;

	reset_output_stream();
	(void)snprintf(fname, 300, "%s/%s", getenv("HOME"), E_INITFILE);

	if ((fd = open(fname, O_RDONLY)) == -1)
		fatal("failed to open " E_INITFILE " in HOME directory");

	reset_output_stream();
	output = load_file(fd);
	assert(output != NULL);
	close(fd);

	/* all exceptions start with the word error: */
	if (NULL != strstr(output, "error:"))
		fatal(output);
	reset_output_stream();
}

void setup_keys()
{
	create_keys();
        set_key_internal("c-a",     "(beginning-of-line)",   "\x01", lnbegin);
	set_key_internal("c-b",     "(backward-char)",       "\x02", left);
	set_key_internal("c-d",     "(delete)",              "\x04", delete);
	set_key_internal("c-e",     "(end-of-line)",         "\x05", lnend);
	set_key_internal("c-f",     "(forward-char)",        "\x06", right);
	set_key_internal("c-n",     "(next-line)",           "\x0E", down);
	set_key_internal("c-p",     "(previous-line)",       "\x10", up);
	set_key_internal("c-h",     "(backspace)",           "\x08", backspace);
	set_key_internal("c-v",     "(page-down)",           "\x16", pgdown);
	set_key_internal("c-w",     "(kill-region)",         "\x17", kill_region);
	set_key_internal("c-y",     "(yank)",                "\x19", yank);
	set_key_internal("esc-k",   "(kill-region)",         "\x1B\x6B", kill_region);
	set_key_internal("esc-v",   "(page-up)",             "\x1B\x76", pgup);
	set_key_internal("esc-w",   "(copy-region)",         "\x1B\x77", copy_region);
	set_key_internal("esc-<",   "(beginning-of-buffer)", "\x1B\x3C", top);
	set_key_internal("esc-<",   "(end-of-bufffer)",      "\x1B\x3E", bottom);
	set_key_internal("esc-]",   "(eval-block)",          "\x1B\x5D", eval_block);
	set_key_internal("up ",     "(previous-line)",       "\x1B\x5B\x41", up);
	set_key_internal("down",    "(next-line)",           "\x1B\x5B\x42", down);
	set_key_internal("left",    "(backward-character)",  "\x1B\x5B\x44", left);
	set_key_internal("right",   "(forward-character)",   "\x1B\x5B\x43", right);
	set_key_internal("home",    "(beginning-of-line)",   "\x1B\x4F\x48", lnbegin);
	set_key_internal("end",     "(end-of-line)",         "\x1B\x4F\x46", lnend);
	set_key_internal("del",     "(delete)",              "\x1B\x5B\x33\x7E", delete);
	set_key_internal("pgup",    "(page-up)",             "\x1B\x5B\x35\x7E",pgup);
	set_key_internal("pgdn",    "(page-down)",           "\x1B\x5B\x36\x7E", pgdown);
	set_key_internal("backspace","(backspace)",          "\x7f", backspace);
	set_key_internal("c-x c-s", "(save-buffer)",         "\x18\x13", save_buffer);  
	set_key_internal("c-x c-c", "(exit)",                "\x18\x03", quit);
	set_key_internal("c-space", "(set-mark)",            "\x00", set_mark);
	set_key_internal("resize",  "(resize)",              "\x9A", resize_terminal);
}

int main(int argc, char **argv)
{
	if (argc != 2) fatal("usage: " E_NAME " filename\n");

	setup_keys();
	(void)init_lisp();
	load_config();
	initscr();	
	raw();
	noecho();
	
	curbp = new_buffer();
	(void)insert_file(argv[1], FALSE);
	/* Save filename irregardless of load() success. */
	strncpy(curbp->b_fname, argv[1], MAX_FNAME);
	curbp->b_fname[MAX_FNAME] = '\0'; /* force truncation */

	if (!growgap(curbp, CHUNK)) fatal("Failed to allocate required memory.\n");

	while (!done) {
		display();
		input = get_key(khead, &key_return);

		if (key_return != NULL) {
			(key_return->k_func)();
		} else {
			/* allow TAB and NEWLINE, any other control char is 'Not Bound' */
			if (*input > 31 || *input == 10 || *input == 9)
				insert();
                        else {
				fflush(stdin);
				msg(E_NOT_BOUND);
			}
		}
	}

	noraw();
	endwin();
	return 0;
}
