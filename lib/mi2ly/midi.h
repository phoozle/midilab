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



#ifndef midi_h
#define midi_h
#include <stdio.h>
#include "defs.h"
#define TRACK_BEGIN  4

#define UNKNOWN_ERROR 0 /* FIXME make enum */
#define BEYOND_TRACK_LIMIT 1
#define WRONG_HEADER_SIGNATURE 2
#define VARLEN_GT_4_BYTES 3
#define WRONG_TRACK_SIGNATURE 4
#define WRONG_MIDI_DATA_BYTE 5
#define WRONG_LEADING_BYTE 6
enum smf_events
{
MIDI = 1, META, SYSEX
};
enum midi_commands
{
NOTE_OFF = 0, NOTE_ON, AFTER_TOUCH_K, CONTROL_CHANGE,
PROGRAM_CHANGE, AFTER_TOUCH_P, PITCH_WHEEL, SYSTEM_EXCLUS, MIDI_UNDEFINED = 127
};
enum meta_commands
{
SEQ_NO = 0, TEXT, COPYRIGHT, TRACK_NAME, INSTR_NAME, LYRIC, MARKER, CUE_POINT,
MIDI_CHANNEL_PREFIX, MIDI_PORT, M_END_OF_TRACK, SET_TEMPO, SMPTE_OFF, TIME_SIG,
KEY_SIG, SEQ_SPECIFIC, UNKNOWN
};
typedef struct
{
   void *p;
   unsigned char type;
   unsigned long time;
}
SMF_EVENT;
typedef struct {
    uchar cmnd;
    uchar channel;
    uchar ndat;
    uchar data[2];
    } MIDI_EVENT;
typedef struct {
    uchar cmnd;
    void *p;
    } META_EVENT;
typedef struct 
{
   unsigned char hour;
   unsigned char minute;
   unsigned char second;
   unsigned char frame;
   unsigned char subframe;
}
SMPTE_OFFSET;
typedef struct
{
   unsigned char nn;  /* log2(num) */
   unsigned char dd;  /* den */
   unsigned char cc;  /* Midi clocks per metronome tick */
   unsigned char bb;  /* 1/32 notes per  24 midi clocks */
}
TIME_SIGNATURE;
typedef struct
{   
   char sf;           /* >0 sharps, <0 flats  */
   unsigned char mi;  /* 0 = major, 1 = minor */
}
KEY_SIGNATURE;
typedef struct 
{
   unsigned long len;
   unsigned char *data;
}
VARLEN_DATA;
typedef struct
{
    VARLEN_DATA *p;
} 
SYSEX_EVENT;
typedef struct 
{
   unsigned short nstr;
   unsigned long *stretched;
   unsigned long *unstretched;
   double *factors;
}
STRETCH_DATA;
typedef struct 
{
   FILE *fmid;
   STRETCH_DATA *str;
   unsigned long cur_time;
   unsigned long hlenght;
   unsigned long format;
   unsigned long ntrack;
   unsigned long position;
   unsigned long left_track_bytes;
   unsigned long ticks_quarter_note;
   unsigned short cur_track;
   unsigned char running_status;
   unsigned char error;
}
SMF_SCANNER;
#endif
