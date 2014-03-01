/*
 This file is part of pybliographer
 
 Copyright (C) 1998-1999 Frederic GOBRY
 Email : gobry@idiap.ch
 	   
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2 
 of the License, or (at your option) any later version.
   
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details. 
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 $Id: accents.c,v 1.1.2.2 2003/09/02 14:35:33 fredgo Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <string.h>

#include "bibtex.h"

typedef struct  {
    gchar c;
    gchar m;
}
CharMapping;

typedef struct  {
    gchar * c;
    gchar * m;
}
StringMapping;

CharMapping acute [] = {
    {'A', '\xC1'},
    {'E', '\xC9'},
    {'I', '\xCD'},
    {'O', '\xD3'},
    {'U', '\xDA'},
    {'Y', '\xDD'},
    {'a', '\xE1'},
    {'e', '\xE9'},
    {'i', '\xED'},
    {'o', '\xF3'},
    {'u', '\xFA'},
    {'y', '\xFD'},
    {0, 0}
};

CharMapping grave [] = {
    {'A', '\xC0'},
    {'E', '\xC8'},
    {'I', '\xCC'},
    {'O', '\xD2'},
    {'U', '\xD9'},
    {'a', '\xE0'},
    {'e', '\xE8'},
    {'i', '\xEC'},
    {'o', '\xF2'},
    {'u', '\xF9'},
    {0, 0}
};

CharMapping hat [] = {
    {'A', '\xC2'},
    {'E', '\xCA'},
    {'I', '\xCE'},
    {'O', '\xD4'},
    {'U', '\xDB'},
    {'a', '\xE2'},
    {'e', '\xEA'},
    {'i', '\xEE'},
    {'o', '\xF4'},
    {'u', '\xFB'},
    {0, 0}
};

CharMapping trema [] = {
    {'A', '\xC4'},
    {'E', '\xCB'},
    {'I', '\xCF'},
    {'O', '\xD6'},
    {'U', '\xDC'},
    {'a', '\xE4'},
    {'e', '\xEB'},
    {'i', '\xEF'},
    {'o', '\xF6'},
    {'u', '\xFC'},
    {'y', '\xFF'},
    {0, 0}
};

CharMapping cedilla [] = {
    {'C', '\xC7'},
    {'c', '\xE7'},
    {0, 0}
};

CharMapping tilda [] = {
    {'A', '\xC3'},
    {'O', '\xD5'},
    {'a', '\xE3'},
    {'o', '\xF5'},
    {'n', '\xF1'},
    {'N', '\xD1'},
    {0, 0}
};

StringMapping commands [] = {
    {"backslash", "\\"},
    {"S",  "\xA7"},
    {"ss", "\xDF"},
    {"DH", "\xD0"},
    {"dh", "\xF0"},
    {"AE", "\xC6"},
    {"ae", "\xE6"},
    {"O",  "\xD8"},
    {"o",  "\xF8"},
    {"TH", "\xDE"},
    {"th", "\xFE"},
    {"aa", "\xE5"},
    {"AA", "\xC5"},
    {"guillemotleft",    "\xAB"},
    {"guillemotright",   "\xBB"},
    {"flqq",             "\xAB"},
    {"frqq",             "\xBB"},
    {"guilsingleft",     "<"},
    {"guilsingright",    ">"},
    {"textquestiondown", "\xBF"},
    {"textexclamdown",   "\xA1"},
    {"copyright",        "\xA9"},
    {"pound",            "\xA3"},
    {"neg",              "\xAC"},
    {"-",                "\xAD"},
    {"cdotp",            "\xB7"},
    {",",                "\xB8"},
    {NULL, NULL}
};

static gchar *
initialize_table (CharMapping * map, char empty) {
    gchar * table;

    g_return_val_if_fail (map != NULL, NULL);

    table = g_new0 (gchar, 256);

    while (map->c != '\0') {
	table [(int) map->c] = map->m;
	map ++;
    }

    table [0] = empty;

    return table;
}

static GHashTable *
initialize_mapping (StringMapping * map) {
    GHashTable * dico;
    
    dico = g_hash_table_new (g_str_hash, g_str_equal);
    
    while (map->c != NULL) {
	g_hash_table_insert (dico, map->c, map->m);
	map ++;
    }

    return dico;
}

static gchar *
eat_as_string (GList ** flow,
	       gint qtt,
	       gboolean * loss) {

    BibtexStruct * tmp_s;
    gchar * tmp, * text = g_strdup ("");

    g_return_val_if_fail (qtt > 0, text);

    if (flow == NULL) {
	return text;
    }

    while (qtt > 0 && (* flow)) {
	tmp = text;
	tmp_s = (BibtexStruct *) ((* flow)->data);
	* flow = (* flow)->next;

	if (tmp_s->type == BIBTEX_STRUCT_SPACE) continue;

	qtt --;

	text = g_strconcat (text, 
			    bibtex_struct_as_string (tmp_s, BIBTEX_OTHER,
						     NULL, loss),
			    NULL);
	g_free (tmp);
    }

    return text;
}

gchar * 
bibtex_accent_string (BibtexStruct * s, 
		      GList ** flow,
		      gboolean * loss) {
    
    static gchar * acute_table = NULL;
    static gchar * grave_table = NULL;
    static gchar * hat_table = NULL;
    static gchar * trema_table = NULL;
    static gchar * cedilla_table = NULL;
    static gchar * tilda_table = NULL;

    static GHashTable * commands_table = NULL;

    gchar * text, * tmp, accent;

    g_return_val_if_fail (s != NULL, NULL);
    g_return_val_if_fail (s->type == BIBTEX_STRUCT_COMMAND, NULL);

    if (acute_table == NULL) {
	/* Initialize accent table if necessary */

	acute_table    = initialize_table   (acute,   '´');
	grave_table    = initialize_table   (grave,   '\0');
	hat_table      = initialize_table   (hat,     '\0');
	trema_table    = initialize_table   (trema,   '¨');
	cedilla_table  = initialize_table   (cedilla, '\0');
	tilda_table    = initialize_table   (tilda,   '\0');

	commands_table = initialize_mapping (commands);
    }

    /* traiter les codes de 1 de long */
    if (strlen (s->value.com) == 1) {
	accent = s->value.com [0];

	if (accent == 'i') {
	    return g_strdup ("i");
	}

	/* Is it a known accent ? */
	if (accent == '\'' ||
	    accent == '^'  ||
	    accent == '`'  ||
	    accent == '"'  ||
	    accent == '~'  ||
	    accent == 'c') {
	    
	    text = eat_as_string (flow, 1, loss);

	    tmp  = NULL;

	    switch (accent) {
	    case '\'':
		tmp = acute_table;
		break;
	    case '`':
		tmp = grave_table;
		break;
	    case '^':
		tmp = hat_table;
		break;
	    case '"':
		tmp = trema_table;
		break;
	    case 'c':
		tmp = cedilla_table;
		break;
	    case '~':
		tmp = tilda_table;
		break;
		
	    default:
		g_assert_not_reached ();
		break;
	    }
	    
	    /* We know how to convert */
	    if (tmp [(int) text [0]] != 0) {
		if (text [0]) {
		    text [0] = tmp [(int) text [0]];
		}
		else {
		    tmp = g_strdup_printf ("%c", tmp [(int)text [0]]);
		    g_free (text);
		    text = tmp;
		}
	    }
	    else {
		if (loss) * loss = TRUE;
	    }

	    return text;
	}
	else {
	    /* return the single symbol */
	    if (! isalnum (s->value.com [0])) {
		return g_strdup (s->value.com);
	    }
	}
    }

    /* if not found, use dictionnary to eventually map */
    text = g_hash_table_lookup (commands_table, s->value.com);

    if (text) {
      return g_strdup (text);
    }

    if (loss) * loss = TRUE;
    bibtex_warning ("unable to convert `\\%s'", s->value.com);

    return g_strdup (s->value.com);
}


void
bibtex_capitalize (gchar * text,
		   gboolean is_noun,
		   gboolean at_start) {
    gboolean begin_of_sentence;
    gchar * current;

    g_return_if_fail (text != NULL);

    /* Put everything lowercase */
    if (is_noun) {
	gchar* c = text;
	for (; *c != '\0'; *c = g_ascii_tolower(*c), c++);
    }

    current = text;
    begin_of_sentence = at_start;

    /* Parse the whole text */
    while (* current) {
	switch (* current) {
	    
	case ' ':
	    /* Skip whitespace */
	    break;

	case '-':
	    /* Composed names */
	    if (is_noun) {
		begin_of_sentence = TRUE;
	    }
	    break;

	case '.':
	    /* New sentence */
	    begin_of_sentence = TRUE;
	    break;

	default:
	    if (isalpha (* current) && begin_of_sentence) {
		* current = toupper (* current);
		begin_of_sentence = FALSE;
	    }
	    break;
	}
	
	current ++;
    }
}
