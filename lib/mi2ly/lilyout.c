/*----------------------------------------------------------------------------
    This file is a part of mi2ly a program to convert midi files to
    the lilypond language.
    
    Copyright (C) 2004, 2005  Antonio Palama' (palama@inwind.it)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
----------------------------------------------------------------------------*/



#include <stdio.h>
#include <string.h>
#include "mi2ly.h"
#include <stdlib.h>
/* output note names, accidentals and apex/commas */
/* Note names to be used according to current key */
/* Major keys only */
static char *note_names[14][12] =
{
   /* g flat major */
  {"c", "des", "d", "ees", "e", "f", "ges", "g", "aes", "a", "bes", "ces"},
   /* d flat major */
  {"c", "des", "d", "ees", "e", "f", "ges", "g", "aes", "a", "bes", "b"},
   /* a flat major */
  {"c", "des", "d", "ees", "e", "f", "ges", "g", "aes", "a", "bes", "b"},
   /* e flat major */
  {"c", "des", "d", "ees", "e", "f", "ges", "g", "aes", "a", "bes", "b"},
   /* b flat major */
  {"c", "des", "d", "ees", "e", "f", "fis", "g", "aes", "a", "bes", "b"},
   /* f major */
  {"c", "cis", "d", "ees", "e", "f", "fis", "g", "aes", "a", "bes", "b"},
   /* c major */
  {"c", "cis", "d", "ees", "e", "f", "fis", "g", "gis", "a", "bes", "b"},
   /* g major */
  {"c", "cis", "d", "ees", "e", "f", "fis", "g", "gis", "a", "bes", "b"},
   /* d major */
  {"c", "cis", "d", "dis", "e", "f", "fis", "g", "gis", "a", "bes", "b"},
   /* a major */
  {"c", "cis", "d", "dis", "e", "f", "fis", "g", "gis", "a", "ais", "b"},
   /* e major */
  {"c", "cis", "d", "dis", "e", "f", "fis", "g", "gis", "a", "ais", "b"},
   /* b major */
  {"c", "cis", "d", "dis", "e", "f", "fis", "g", "gis", "a", "ais", "b"},
   /* f sharp major */
  {"c", "cis", "d", "dis", "e", "eis", "fis", "g", "gis", "a", "ais", "b"},
   /* c sharp major */
  {"bis", "cis", "d", "dis", "e", "eis", "fis", "g", "gis", "a", "ais", "b"}
};
static char *key_note_names[14][7] =
{
   /* g flat major */
  {"ces", "des", "ees", "f", "ges", "aes", "bes"},
   /* d flat major */
  {"c", "des", "ees", "f", "ges", "aes", "bes"},
   /* a flat major */
  {"c", "des", "ees", "f", "g", "aes", "bes"},
   /* e flat major */
  {"c", "d", "ees", "f", "g", "aes", "bes"},
   /* b flat major */
  {"c", "d", "ees", "f", "g", "a", "bes"},
   /* f major */
  {"c", "d", "e", "f", "g", "a", "bes"},
   /* c major */
  {"c", "d", "e", "f", "g", "a", "b"},
   /* g major */
  {"c", "d", "e", "fis", "g", "a", "b"},
   /* d major */
  {"cis", "d", "e", "fis", "g", "a", "b"},
   /* a major */
  {"cis", "dis", "e", "fis", "gis", "a", "b"},
   /* e major */
  {"cis", "dis", "e", "fis", "gis", "a", "b"},
   /* b major */
  {"cis", "dis", "e", "fis", "gis", "ais", "b"},
   /* f sharp major */
  {"cis", "dis", "eis", "fis", "gis", "ais", "b"},
   /* c sharp major */
  {"cis", "dis", "eis", "fis", "gis", "ais", "bis"},
};
static unsigned char nbuf1[MAX_NOTE_BUFFER], nbuf2[MAX_NOTE_BUFFER];
static unsigned char old_note_number;
static unsigned char *note_buffer, *old_note_buffer, inb, 
  lenght_old_note_buffer;
static GEN_INFO *gen;
void init_lily_out_note(GEN_INFO *g)
{
  gen = g;
  old_note_number = 48; /* c i.e. one octave below middle c */
  note_buffer = nbuf1;
  old_note_buffer = nbuf2;
}
void swap_note_buffers()
{   
  unsigned char *swp;
  lenght_old_note_buffer = inb;
  swp = old_note_buffer;
  old_note_buffer = note_buffer;
  note_buffer = swp;
}
char * dotted_duration(unsigned long measure_duration)
{
  if (measure_duration == QUARTER * 6)
      return "1.";
  if (measure_duration == QUARTER * 4)
      return "1";
  if (measure_duration == QUARTER * 3)
      return "2.";
  if (measure_duration == QUARTER * 2)
      return "2";
  if (measure_duration == QUARTER * 3 / 2)
      return "4.";
  if (measure_duration == QUARTER )
      return "4";
  fprintf (stderr, "Unknown measure duration in dotted(duration()\n");
  abort();
}
char inline *note_name(unsigned char note)
{
  return note_names[gen->key + 6][note];
}
int inline isinkey(char *n)
{
  int i;
  for (i = 0; i < 7; i++)
    if (strcmp(n, key_note_names[gen->key + 6][i]) == 0)
      return 1;
  return 0;
}
unsigned char note_index(unsigned char note)
{
   return note_name(note)[0] - 'a';
}
static unsigned char notes_this_measure[7]; 
/* FIXME 1 byte is enough for this pourpose */
void forget_notes(void)
{
  int i;
  inb = 0;
  for (i = 0; i < 7; i++)
    notes_this_measure[i] = 0;
}
void remember (char basic_name)
{
  notes_this_measure[basic_name - 'a'] = 1;
}
int first_time(char basic_name)
{
  return !notes_this_measure[basic_name - 'a'];
}
void lily_out_note(unsigned char note_number)
{
  int note, dn, i;
  /* Remeber notes of this measure to be used to decide if courtesy
   * accidentals are needed in next measure */
  if (inb >= MAX_NOTE_BUFFER)
    {
      fprintf(stderr, "Dimension of note buffer exceeded\n");
      abort();
    }  
  note_buffer[inb++] = note_number;
  note = note_number % 12;
  dn = note_number - old_note_number;
  fprintf (gen->fout, "%s", note_name(note));
  if (dn >= 0)
    {
      int napex, delta_basic;
       /* Must use basic names: lilypond goes to the closest basic name
	* disregarding accidentals 
	* something like napex = (dn + 6) / 12;
	*                if (some function of basic names)
	*                  napex --;
	* should work */
       /* yet simpler dn / 12 + diff(basic names) > 3 */
       napex = dn / 12;
       delta_basic = note_name(note)[0] - note_name(old_note_number % 12)[0];
       if (delta_basic < 0)
	 delta_basic += 7;
       if (delta_basic > 3)
	 napex++;
      while (napex--)
	fprintf(gen->fout, "'");
    }  
  else
    {
       /* yet simpler -dn / 12 - diff(basic names) > 3 */
       int ncomma, delta_basic;
       ncomma = -dn / 12;
       delta_basic = -note_name(note)[0] + note_name(old_note_number % 12)[0];
       if (delta_basic < 0)
	 delta_basic += 7;
       if (delta_basic > 3)
	 ncomma++;
      while (ncomma--)
	fprintf (gen->fout, ",");
    }
  /* Decide if a courtesy accidental is needed */
  /* Find last occurrence of this basic note name in last measure */
    {      
      unsigned char old_note;
      for (i = lenght_old_note_buffer -1; i >= 0; i--)
	{
	  old_note = old_note_buffer[i] % 12;
	  if (note_name(old_note)[0] == note_name(note)[0]) /* Same basic notes? */
	    break;
	}
      if (i >=0 ) /* Did break */
	{      
	  if (!isinkey(note_name(old_note)) && 
	      strcmp(note_name(note), note_name(old_note)) &&
	      first_time(note_name(note)[0]))
	    fprintf (gen->fout, "!");
	}
    }  
  /* Must remeber occurrence of notes in last measure
     and output exclamation if occurrence was modified by an accidental */
  remember(note_name(note)[0]);
  old_note_number = note_number;
}
