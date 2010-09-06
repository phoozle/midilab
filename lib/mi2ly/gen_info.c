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
GEN_INFO *gen_info_ist(void)
{
   GEN_INFO *g;
   g = malloc(sizeof(GEN_INFO));
   if (!g)
     return g;
   /* set defaults */
  g->str = malloc(sizeof(STRETCH_DATA));
  g->active_track = 1;
  g->key = 0; /* c major */
  g->transpose = 0;
  g->multimeasure_rests = 1;
  return g;   
}
