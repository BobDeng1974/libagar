/*	$Csoft: keycodes.c,v 1.32 2003/08/31 11:58:10 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <engine/engine.h>
#include <engine/input.h>
#include <engine/prop.h>
#include <engine/config.h>

#include <ctype.h>
#include <string.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

static void key_bspace(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_delete(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_home(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_end(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_kill(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_left(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_right(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_character(struct textbox *, SDLKey, int, const char *, Uint32);

const struct keycode keycodes[] = {
	{ SDLK_BACKSPACE,	0,		key_bspace,	NULL, 1 },
	{ SDLK_DELETE,		0,		key_delete,	NULL, 1 },
	{ SDLK_HOME,		0,		key_home,	NULL, 1 },
	{ SDLK_END,		0,		key_end,	NULL, 1 },
	{ SDLK_a,		KMOD_CTRL,	key_home,	NULL, 1 },
	{ SDLK_e,		KMOD_CTRL,	key_end,	NULL, 1 },
	{ SDLK_k,		KMOD_CTRL,	key_kill,	NULL, 1 },
	{ SDLK_LEFT,		0,		key_left,	NULL, 1 },
	{ SDLK_RIGHT,		0,		key_right,	NULL, 1 },
	{ SDLK_LAST,		0,		key_character,	NULL, 0 },
};

static __inline__ Uint32
key_apply_mod(Uint32 key, int kmod)
{
	if (kmod & KMOD_CAPS) {
		if (kmod & KMOD_SHIFT) {
			return (tolower(key));
		} else {
			return (toupper(key));
		}
	} else {
		if (kmod & KMOD_SHIFT) {
			return (toupper(key));
		} else {
			return (key);
		}
	}
}

static struct {
	Uint32 comp, key, res;
} compose[] = {
	{ 0x0060, 0x0020, 0x0060 },  /* GRAVE ACCENT */
	{ 0x0060, 0x0061, 0x00e0 },  /* LATIN SMALL LETTER A */
	{ 0x0060, 0x0041, 0x00c0 },  /* LATIN CAPITAL LETTER A */
	{ 0x0060, 0x0065, 0x00e8 },  /* LATIN SMALL LETTER E */
	{ 0x0060, 0x0045, 0x00c8 },  /* LATIN CAPITAL LETTER E */
	{ 0x0060, 0x0069, 0x00ec },  /* LATIN SMALL LETTER I */
	{ 0x0060, 0x0049, 0x00cc },  /* LATIN CAPITAL LETTER I */
	{ 0x0060, 0x006f, 0x00f2 },  /* LATIN SMALL LETTER O */
	{ 0x0060, 0x004f, 0x00d2 },  /* LATIN CAPITAL LETTER O */
	{ 0x0060, 0x0075, 0x00f9 },  /* LATIN SMALL LETTER U */
	{ 0x0060, 0x0055, 0x00d9 },  /* LATIN CAPITAL LETTER U */
	
	{ 0x00b4, 0x0020, 0x0060 },  /* ACUTE ACCENT */
	{ 0x00b4, 0x0065, 0x00e9 },  /* LATIN SMALL LETTER E */
	{ 0x00b4, 0x0045, 0x00c9 },  /* LATIN CAPITAL LETTER E */
	
	{ 0x02db, 0x0020, 0x02db },  /* OGONEK */
	{ 0x02db, 0x0061, 0x0105 },  /* LATIN SMALL LETTER C */
	{ 0x02db, 0x0041, 0x0104 },  /* LATIN CAPITAL LETTER C */
	{ 0x02db, 0x0075, 0x0173 },  /* LATIN SMALL LETTER U */
	{ 0x02db, 0x0055, 0x0172 },  /* LATIN CAPITAL LETTER U */

	{ 0x00b8, 0x0020, 0x00b8 },  /* CEDILLA */
	{ 0x00b8, 0x0063, 0x00e7 },  /* LATIN SMALL LETTER C */
	{ 0x00b8, 0x0043, 0x00c7 },  /* LATIN CAPITAL LETTER C */
	{ 0x00b8, 0x0067, 0x0123 },  /* LATIN SMALL LETTER G */
	{ 0x00b8, 0x0047, 0x0122 },  /* LATIN CAPITAL LETTER G */
	{ 0x00b8, 0x006e, 0x0146 },  /* LATIN SMALL LETTER N */
	{ 0x00b8, 0x004e, 0x0145 },  /* LATIN CAPITAL LETTER N */
	{ 0x00b8, 0x006b, 0x0137 },  /* LATIN SMALL LETTER K */
	{ 0x00b8, 0x004b, 0x0136 },  /* LATIN CAPITAL LETTER K */
	{ 0x00b8, 0x0072, 0x0157 },  /* LATIN SMALL LETTER R */
	{ 0x00b8, 0x0052, 0x0156 },  /* LATIN CAPITAL LETTER R */
	{ 0x00b8, 0x0074, 0x0163 },  /* LATIN SMALL LETTER T */
	{ 0x00b8, 0x0054, 0x0162 },  /* LATIN CAPITAL LETTER T */
	{ 0x00b8, 0x0073, 0x015f },  /* LATIN SMALL LETTER S */
	{ 0x00b8, 0x0053, 0x015e },  /* LATIN CAPITAL LETTER S */
	
	{ 0x00a8, 0x0020, 0x00a8 },  /* DIAERESIS */
	{ 0x00a8, 0x0061, 0x00e4 },  /* LATIN SMALL LETTER A */
	{ 0x00a8, 0x0041, 0x00c4 },  /* LATIN CAPITAL LETTER A */
	{ 0x00a8, 0x0065, 0x00eb },  /* LATIN SMALL LETTER E */
	{ 0x00a8, 0x0045, 0x00cb },  /* LATIN CAPITAL LETTER E */
	{ 0x00a8, 0x0069, 0x00ef },  /* LATIN SMALL LETTER I */
	{ 0x00a8, 0x0049, 0x00cf },  /* LATIN CAPITAL LETTER I */
	{ 0x00a8, 0x006f, 0x00f6 },  /* LATIN SMALL LETTER O */
	{ 0x00a8, 0x004f, 0x00d6 },  /* LATIN CAPITAL LETTER O */
	{ 0x00a8, 0x0079, 0x00ff },  /* LATIN SMALL LETTER Y */
	{ 0x00a8, 0x0059, 0x0178 },  /* LATIN CAPITAL LETTER Y */
	{ 0x00a8, 0x0075, 0x00fc },  /* LATIN SMALL LETTER U */
	{ 0x00a8, 0x0055, 0x00dc },  /* LATIN CAPITAL LETTER U */
	
	{ 0x005e, 0x0020, 0x005e },  /* CIRCUMFLEX ACCENT */
	{ 0x005e, 0x0061, 0x00e2 },  /* LATIN SMALL LETTER A */
	{ 0x005e, 0x0041, 0x00c2 },  /* LATIN CAPITAL LETTER A */
	{ 0x005e, 0x0063, 0x0109 },  /* LATIN SMALL LETTER C */
	{ 0x005e, 0x0043, 0x0108 },  /* LATIN CAPITAL LETTER C */
	{ 0x005e, 0x0065, 0x00ea },  /* LATIN SMALL LETTER E */
	{ 0x005e, 0x0045, 0x00ca },  /* LATIN CAPITAL LETTER E */
	{ 0x005e, 0x0067, 0x011d },  /* LATIN SMALL LETTER G */
	{ 0x005e, 0x0047, 0x011c },  /* LATIN CAPITAL LETTER G */
	{ 0x005e, 0x0069, 0x00ee },  /* LATIN SMALL LETTER I */
	{ 0x005e, 0x0049, 0x00ce },  /* LATIN CAPITAL LETTER I */
	{ 0x005e, 0x006f, 0x00f4 },  /* LATIN SMALL LETTER O */
	{ 0x005e, 0x004f, 0x00d4 },  /* LATIN CAPITAL LETTER O */
	{ 0x005e, 0x0073, 0x015d },  /* LATIN SMALL LETTER S */
	{ 0x005e, 0x0053, 0x015c },  /* LATIN CAPITAL LETTER S */
	{ 0x005e, 0x0079, 0x0177 },  /* LATIN SMALL LETTER Y */
	{ 0x005e, 0x0059, 0x0176 },  /* LATIN CAPITAL LETTER Y */
	{ 0x005e, 0x0075, 0x00fb },  /* LATIN SMALL LETTER U */
	{ 0x005e, 0x0055, 0x00db },  /* LATIN CAPITAL LETTER U */
	{ 0x005e, 0x0077, 0x0175 },  /* LATIN SMALL LETTER W */
	{ 0x005e, 0x0057, 0x0174 },  /* LATIN CAPITAL LETTER W */
};
static const int ncompose = sizeof(compose) / sizeof(compose[0]);

static int
key_compose(struct textbox *tbox, Uint32 key, Uint32 *ins)
{
	int i;
		
	if (tbox->compose != 0) {
		for (i = 0; i < ncompose; i++) {
			if (compose[i].comp == tbox->compose &&
			    compose[i].key == key)
				break;
		}
		if (i < ncompose) {
			ins[0] = compose[i].res;
			tbox->compose = 0;
			return (1);
		} else {
			ins[0] = tbox->compose;
			ins[1] = key;
			tbox->compose = 0;
			return (2);
		}
	} else {
		for (i = 0; i < ncompose; i++) {
			if (compose[i].comp == key)
				break;
		}
		if (i < ncompose) {
			tbox->compose = key;
			return (0);
		} else {
			ins[0] = key;
			return (1);
		}
	}
}

static void
key_character(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	int trans;
	Uint32 ins[2];
	int i, nins;

	trans = prop_get_bool(config, "input.unicode");

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	if (trans) {
		if (uch == 0) {
			goto skip;
		}
		nins = key_compose(tbox, uch, ins);
	} else {
		ins[0] = key_apply_mod((Uint32)keysym, keymod);
		nins = 1;
	}

	/* Ensure the new character(s) fit inside the buffer. */
	if (len+nins >= stringb->size/sizeof(Uint32))
		goto skip;

	if (tbox->pos == len) {
		/* Append to the end of string */
		if (trans) {
			if (uch != 0) {
				for (i = 0; i < nins; i++)
					ucs[len+i] = ins[i];
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			for (i = 0; i < nins; i++)
				ucs[len+i] = ins[i];
		} else {
			goto skip;
		}
	} else {
		Uint32 *p = ucs + tbox->pos;

		/* Insert at the cursor position in the string. */
		memcpy(p+nins, p, (len - tbox->pos)*sizeof(Uint32));
		if (trans) {
			if (uch != 0) {
				for (i = 0; i < nins; i++)
					ucs[tbox->pos+i] = ins[i];
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			for (i = 0; i < nins; i++)
				ucs[len+i] = ins[i];
		} else {
			goto skip;
		}
	}
out:
	ucs[len+nins] = '\0';
	tbox->pos += nins;
	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
skip:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Destroy the character before the cursor. */
static void
key_bspace(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	int i;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	if (tbox->pos == 0 || len == 0)
		goto out;

	if (tbox->pos == len) {
		ucs[--tbox->pos] = '\0';
	} else if (tbox->pos > 0) {
		/* XXX use memmove */
		for (i = tbox->pos-1; i < len; i++) {
			ucs[i] = ucs[i+1];
		}
		tbox->pos--;
	}
	if (tbox->pos == tbox->offs) {
		if ((tbox->offs -= 4) < 1)
			tbox->offs = 0;
	}
	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
out:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Eliminate the character at the cursor position. */
static void
key_delete(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	int i;
	
	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	if (tbox->pos == len && len > 0) {		/* End of string */
		ucs[--tbox->pos] = '\0';
	} else if (tbox->pos >= 0) {			/* Middle of string */
		/* XXX use memmove */
		for (i = tbox->pos; i < len; i++)
			ucs[i] = ucs[i+1];
	}

	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Move the cursor to the start of the string. */
static void
key_home(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	if (tbox->offs > 0) {
		tbox->offs = 0;
	}
	tbox->pos = 0;
}

/* Move the cursor to the end of the string. */
static void
key_end(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	Uint32 *ucs;
	char *utf8;
	
	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	tbox->pos = ucs4_len(ucs);
	tbox->offs = 0;
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Kill the text after the cursor. */
/* XXX save to a kill buffer, etc */
static void
key_kill(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	Uint32 *ucs;
	char *utf8;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);

	ucs[tbox->pos] = '\0';

	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Move the cursor to the left. */
static void
key_left(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	if (--tbox->pos < 1) {
		tbox->pos = 0;
	}
	if (tbox->pos == tbox->offs) {
		if (--tbox->offs < 0)
			tbox->offs = 0;
	}
}

/* Move the cursor to the right. */
static void
key_right(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	Uint32 *ucs;
	char *utf8;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);

	if (tbox->pos < ucs4_len(ucs)) {
		tbox->pos++;
	}

	widget_binding_unlock(stringb);
	free(ucs);
}

