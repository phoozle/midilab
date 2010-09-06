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



#include "mi2ly.h"
extern DIRECTIVE directives[];
static GEN_INFO *gen;
void init_utils(GEN_INFO *g)
{
   gen = g;
}
void logmsg(char * str)
{
  fprintf(gen->flog,"%s\n",str);
}
char *keystring[] =
{
   "ges","des","aes","ees","bes","f","c","g","d","a","e","b","fis","cis"
};
int measure_duration_changed(GEN_INFO *g, unsigned short no)
{
   int i;
   for (i = 0; i < g->nmd; i++)/* FIXME Inefficient directives should be sorted */
     {
	if (directives[i].measure != no)
	  continue;
	if (directives[i].cmd == MEASURE_DURATION)
	  return 1;
     }
   return 0;
}
void update_directives(GEN_INFO *g, unsigned short no)
{ /* FIXME mettere un outfile in gen = foo.mly per dump -1 e foo.dmp
   * per gli altri dump e comunque uguale al file definito con l'opzione
   * -o */
   int i;
   
   for (i = 0; i < g->nmd; i++)/* FIXME Inefficient directives should be sorted */
     {
	if (directives[i].measure != no)
	  continue;
	if (directives[i].cmd == MEASURE_DURATION)
	  {
	     fprintf (g->flog, "MEASURE_DURATION, par1 = %d, par2 = %d\n",
		      directives[i].data.time_sig.num,
		      directives[i].data.time_sig.den);
	     fprintf (g->fout, "\\time %d/%d\n", 
		      directives[i].data.time_sig.num, 
		      directives[i].data.time_sig.den);
	     g->time_num = directives[i].data.time_sig.num;
	     g->time_den = directives[i].data.time_sig.den;
	  }
	else if (directives[i].cmd == KEY_SIGN)
	  {
	     g->key = directives[i].data.sharps;
	     fprintf (g->flog, "KEY_SIGNATURE, sharps = %d\n",
		      g->key);
	     fprintf (g->fout, "\\key %s \\major\n", 
		      keystring[g->key + 6]);
	  }
	else;
     }
}
