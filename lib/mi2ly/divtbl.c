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
static unsigned char natt4_4s[] = {1,1,2};
static unsigned long times4_4s0[] = {QUARTER * 4 / 3};
static unsigned long times4_4s1[] = {QUARTER * 8 / 3};
static unsigned long times4_4s2[] = {QUARTER * 4 / 3, QUARTER * 8 / 3};
static unsigned long *times4_4s[] = {times4_4s0, times4_4s1, times4_4s2};
static DIVISION_TABLE table4_4s = 
{times4_4s, natt4_4s, 3, TRIPLET};
static unsigned char natt2_4s[] = {1,1,2};
static unsigned long times2_4s0[] = {QUARTER * 2 / 3};
static unsigned long times2_4s1[] = {QUARTER * 4 / 3};
static unsigned long times2_4s2[] = {QUARTER * 2 / 3, QUARTER * 4 / 3};
static unsigned long *times2_4s[] = {times2_4s0, times2_4s1, times2_4s2};
static DIVISION_TABLE table2_4s = 
{times2_4s, natt2_4s, 3, TRIPLET};
static unsigned char natt1_4s[] = {1,1,2};
static unsigned long times1_4s0[] = {QUARTER / 3};
static unsigned long times1_4s1[] = {QUARTER * 2 / 3};
static unsigned long times1_4s2[] = {QUARTER / 3, QUARTER * 2 / 3};
static unsigned long *times1_4s[] = {times1_4s0, times1_4s1, times1_4s2};
static DIVISION_TABLE table1_4s = 
{times1_4s, natt1_4s, 3, TRIPLET};
static unsigned char natt3_4s[] = {1};
static unsigned long times3_4s0[] = {QUARTER * 3 / 2};
static unsigned long *times3_4s[] = {times3_4s0};
static DIVISION_TABLE table3_4s = 
{times3_4s, natt3_4s, 1, DUIN};
static unsigned char natt3_2s[] = {1};
static unsigned long times3_2s0[] = {HALF * 3 / 2};
static unsigned long *times3_2s[] = {times3_2s0};
static DIVISION_TABLE table3_2s = 
{times3_2s, natt3_2s, 1, DUIN};
	
DIVISION_TABLE *sec_div_tbl(MEASURE *mea)
{
   if (mea->time_num == 4 && mea->time_den == 4)
     return &table4_4s;
   else if (mea->time_num == 3 && mea->time_den == 4)
     return &table3_4s;
   else if (mea->time_num == 3 && mea->time_den == 2)
     return &table3_2s;
   else
     return NULL;
}
int pri_div(MEASURE *mea)
{
   if (mea->time_num == 4 && mea->time_den == 4)
     return 2;
   else if (mea->time_num == 3 && mea->time_den == 4)
     return 3;
   else if (mea->time_num == 3 && mea->time_den == 2)
     return 3;
   else if (mea->time_num == 6 && mea->time_den == 8)
     return 2;
   else
     return RC_FAIL ;
}
DIVISION_TABLE *in_sec_div_tbl(MUSIC_INTERVAL *in)
{
   if (in->total_duration == FULL)
     return &table4_4s;
   else if (in->total_duration == HALF)
     return &table2_4s;
   else if (in->total_duration == QUARTER)
     return &table1_4s;
   else
     return NULL;
}
int in_pri_div(MUSIC_INTERVAL *in)
{
   if (in->total_duration == FULL)
     return 2;
   else if (in->total_duration == HALF)
     return 2;
   else if (in->total_duration == QUARTER)
     return 2;
   else if (in->total_duration == QUARTER / 2)
     return 2;
   else if (in->total_duration == 3 * QUARTER / 2) /* 3/8 */
     return 3;
   else if (in->total_duration == QUARTER / 4)
     return 2;
   else if (in->total_duration >= QUARTER / 16)
     return 2;
   else
     return RC_FAIL;
}
unsigned long strong_duration(MEASURE *mea)
{
   if (mea->time_num == 4 && mea->time_den == 4)
     return QUARTER;
   else if (mea->time_num == 3 && mea->time_den == 4)
     return QUARTER;
   else if (mea->time_num == 3 && mea->time_den == 2)
     return 2 * QUARTER;
   else if (mea->time_num == 6 && mea->time_den == 8)
     return QUARTER / 2;
   else
     return QUARTER;
}
