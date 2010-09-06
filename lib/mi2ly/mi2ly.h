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



#ifndef mi2ly_h
#define mi2ly_h
#include "defs.h"
#include "midi.h"
#include <ctype.h>
#include <limits.h>
#include <paths.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ALL_CHANNELS 16
#define MAX_NOTE_BUFFER 128
#define QUARTER_DURATION 15360L
#define QUARTER QUARTER_DURATION
#define HALF 2 * QUARTER_DURATION
#define FULL QUARTER * 4
#define Q_TOL QUARTER / 16
#define SLUR_TOL QUARTER / 16
#define MEASURE_TOL QUARTER_DURATION / 16  /* Used to quantize measure ends */
#define MAX_NOTE_DURATION 4 * FULL  
                        /* Maximum note lenght accepted, remove cyphers */
#define MAXTRACKS 1000  /* No real limit on midi files known */
#define MAX_DIRECTIVES 512
#define RC_EOF 1
#define NBASIC_DURATION 8
/* music event flags */
#define ME_TIED  0x01
#define ME_NO_SLUR 0x02  /* Cannot be slurred because next note is the same */
#define ME_SLURRED 0x04
#define ME_STACCATO 0x08
enum  
{MEASURE_DURATION, MINIMUM_REST_DURATION, MINIMUM_REST_RELATIVE_DURATION,
REST_QUANTIZE, KEY_SIGN, STRETCH};
enum me_types
{
ME_NOTE = 0, ME_REST, TRIPLET_BEGIN, TRIPLET_END, DUIN_BEGIN, DUIN_END,
ME_TOTAL_REST
};
enum modifiers
{
   TRIPLET = 0, DUIN, NO_MODIFIER = 0xFF
};
typedef struct
{
   unsigned char type; /* ME_NOTE | ME_REST | TRIPLET_BEGIN | TRIPLET_END |
			* DUIN_BEGIN | DUIN_END */
   unsigned char note;
   unsigned char velocity;
   unsigned long total_duration; 
   /* These are not real times in triplets /duins etc. */
   unsigned long note_duration;
   unsigned char flags;
}
MUSIC_EVENT;

typedef struct mep
{
   struct mep *previous;
   struct mep *next;
   MUSIC_EVENT *ev;
}
MUSIC_EVENT_P;

typedef struct
{
   MUSIC_EVENT_P *first_measure_event;   /* These are linked list pointers */
   MUSIC_EVENT_P *last_measure_event;
   MUSIC_EVENT *previous_measure_event;  /* NULL if not significant */
   unsigned long duration;
   unsigned char time_num;
   unsigned char time_den;
   unsigned short no;
}
MEASURE;

typedef struct 
{
   uchar cmd;
   uchar measure;
   union 
     {
	double stretch_factor;
	struct
	  {
	     uchar num;
	     uchar den;
	  } time_sig;
	char sharps;
     }data;   
} DIRECTIVE;

typedef struct 
{
   unsigned long initial_skip;
   FILE *fmid;
   FILE *flog;
   FILE *fcmd;
   FILE *fout;
   STRETCH_DATA *str;
   unsigned short nmd;
   char dump_mode;
   char key;
   char transpose;
   char multimeasure_rests;
   unsigned char active_track;
   unsigned char active_channel;
   unsigned char time_num;
   unsigned char time_den;
}
GEN_INFO;
typedef struct
{
   FILE * fmly;
   unsigned char slur_open;
   unsigned char multimeasure_rests;
   unsigned short n_multi;
}
LY_PRINTER;
typedef struct
{
   MUSIC_EVENT_P *first_event;
   MUSIC_EVENT_P *last_event;
   unsigned long total_duration;
}
MUSIC_INTERVAL;
typedef struct
{
   unsigned long **times;
   unsigned char *natt;
   unsigned char ndiv;
   unsigned char modifier;
}
DIVISION_TABLE;
typedef struct
{
   unsigned char note;
   unsigned char velocity;
   unsigned long time;     /* note-on time, internal units */
   unsigned long duration; /* ULONG_MAX means duration undefined */
} NOTE_EVENT;
typedef struct nep
{
   struct nep *previous;
   struct nep *next;
   NOTE_EVENT *ev;
}NOTE_EVENT_P;
typedef struct
{
   NOTE_EVENT_P *first_event;
   NOTE_EVENT_P *last_event;
   unsigned long last_time;
} NOTE_QUEUE;
typedef struct
{
   unsigned long initial_skip_time;
   unsigned char end_of_track;
   unsigned char active_channel;
   SMF_SCANNER *smf_sc;
   NOTE_QUEUE *nq;
} 
NOTE_SCANNER;
typedef struct
{
   NOTE_SCANNER *ns;
   NOTE_EVENT nev_old;
   NOTE_EVENT nev;
   unsigned char end_of_track;   
   unsigned char first_time;
}
ME_SCANNER;
typedef struct
{
   ME_SCANNER *mesc;
   SMF_SCANNER *smf_sc;
   MUSIC_EVENT_P *first_queue_event;
   MUSIC_EVENT_P *last_measure_event;
   MUSIC_EVENT_P *last_event;
   MUSIC_EVENT *last_event_tied;
   unsigned long dur_belonging_events;
   unsigned long credit;
   unsigned char time_num;
   unsigned char time_den;
   unsigned char end_of_track;
}
MEAS_SCANNER;
#include "proto.h"
#endif
