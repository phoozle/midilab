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
static SMF_SCANNER *scn;
static uchar nextbyte(void);
void smf_scn_rls(SMF_SCANNER *s)
{   /* FIXME must free s->str */
   free(s);
}
SMF_SCANNER *smf_scn_inst(FILE *f, STRETCH_DATA *str)
{
   SMF_SCANNER * s;
   s = malloc(sizeof(SMF_SCANNER));
   if (!s)
     return NULL;
   s->fmid = f;
   s->str = str;
   rewind(f);
   return s;
}
/* Compute delta time
 * 1 to 4 digits in base 128; last byte has a 0 high order bit
 * update cur_time */
static long varlen(void)
{
   int  used_bytes = 0;
   unsigned long n = 0;
   unsigned char b;
   while (1)
     {
        b = nextbyte();
        scn->left_track_bytes--;
        used_bytes++;
        n = n * 128 + (b & 0x7f);
        if (!(b & 0x80))
	  break;
        else if (used_bytes >= 4)
	  {  /* trying to use more than 4 bytes */
	     n = -1;
	     scn->error = VARLEN_GT_4_BYTES;
	     break;
	  }
	else if (scn->left_track_bytes <= 0)
	  { /* Trying to decode beyond track data limit */
	     scn->error = BEYOND_TRACK_LIMIT;
	     n = -1;
	     break;
	  }
        else;
     }
   return n;
}
static int numdat(uchar cmnd)
{
    if (cmnd >= PROGRAM_CHANGE && cmnd <= AFTER_TOUCH_P)
        return 1;
    else
        return 2;
}
static uchar nextbyte(void)
{
  uchar byte;
  byte = (uchar) getc(scn->fmid);
  scn->position++;
  return byte;
}
static unsigned long track_length(void)
{
  unsigned long tlenght = 0;
  int i;
  for (i = 0; i < 4; i++)
    tlenght = (unsigned long) nextbyte() + tlenght * 256;
  return tlenght;
}
int smf_scn_init(SMF_SCANNER *s)
{
  static char mthd[] = {'M','T','h','d'};
  int i;
  unsigned char b;
  scn = s;
  rewind(scn->fmid);
  for (i = 0; i < 4; i++)
    {
      b = nextbyte();
      if (b != mthd[i])
	break;
    }
  if (i < 4)
    {  /* Header signature missing or incorrect */
      scn->error = WRONG_HEADER_SIGNATURE;
      return NO_MATCH;
    }           /* HEADER SIGNATURE */
  scn->hlenght = 0;  /* Lenght of header data, bytes */
  for (i = 0; i < 4; i++)
    scn->hlenght = (unsigned long) nextbyte() + scn->hlenght * 256;
  scn->format = 256 * nextbyte();
  scn->format  += nextbyte();
  scn->ntrack = 256 * nextbyte();
  scn->ntrack  += nextbyte();  /* Number of tracks */
  b = nextbyte();
  if (b & 0x80)
     b = nextbyte();  /* SMPTE Format - Sorry no decoding - Ignore it */
  else
     scn->ticks_quarter_note = b * 256 + nextbyte();
  scn->cur_track = 0;
  return MATCH;
}
/* Read Track chunk */
int init_next_track(SMF_SCANNER *s)
{
  static char mtrk[] = {'M','T','r','k'};
  int i;
  unsigned char b;
  scn = s;
  for (i = 0; i < 4; i++)  /* Check if 1st 4 chars are "MTrk" */
    {
      b = nextbyte();
      if (b != mtrk[i])
	break;
    }
  if (i < 4)
    {  /* Track signature missing or incorrect */
      scn->error = WRONG_TRACK_SIGNATURE;
      return NO_MATCH;
    }
  (scn->cur_track)++;
  if ((scn->cur_track) > scn->ntrack)
     return EOF;
  scn->left_track_bytes = track_length();
  scn->cur_time = 0;
  return MATCH;
}
void smf_event_rls(SMF_EVENT * eve)
{
   if (eve->type == MIDI)
     free(eve->p);
   else if (eve->type == META)
     {
	META_EVENT *ev = eve->p;
	switch(ev->cmnd)
	  {
	   case SEQ_NO:      /* p was usigned short * */
	   case TEXT:        /* p was char * */
	   case COPYRIGHT:
	   case TRACK_NAME:
	   case INSTR_NAME:
	   case LYRIC:
	   case MARKER:
	   case CUE_POINT:   /* up to here char * */
	   case MIDI_CHANNEL_PREFIX:  /* char * */
	   case SET_TEMPO:   /* unsigned long * */
	   case SMPTE_OFF:   /* SMPTE_OFFSET* */
	   case TIME_SIG:    /* TIME_SIGNATURE* */
	   case KEY_SIG:     /* KEY_SIGNATURE* */
	   case SEQ_SPECIFIC:/* unsigned char* */
	     free(ev->p);
	     break;
	   case UNKNOWN:     /* NULL */
	   case M_END_OF_TRACK:  /* p was NULL */
	   default:
	     break;    /* do nothing */
	  }
	free(ev);
     }
   else if(eve->type == SYSEX)
     free(eve->p);
   else;
   free(eve);
}
unsigned long stretch(SMF_SCANNER *scn, unsigned long t)
{
   int i;
   unsigned short nstr = scn->str->nstr;
   unsigned long *stretched = scn->str->stretched;
   unsigned long *unstretched = scn->str->unstretched;
   double *factors = scn->str->factors;
   unsigned long ticks_qn = scn->ticks_quarter_note;
   unsigned long ti = t * QUARTER / ticks_qn;
   i = 0;
   while(1)
     {
	if (i + 1 == nstr)
	  break;
	if (ti >= unstretched[i] && ti < unstretched[i + 1])
	  break;
	i++;
     }
   t = (stretched[i] + (double)(ti - unstretched[i]) * factors[i]) * 
       ticks_qn / QUARTER + 0.5;
   return t;
}	       
SMF_EVENT *smf_event(SMF_SCANNER *s)
{
   SMF_EVENT *eve;
   unsigned char b;
   scn = s;
   eve = malloc(sizeof(SMF_EVENT));
   scn->cur_time += varlen();
   eve->time = stretch(scn, scn->cur_time);
   b = nextbyte(); 
   if (b < 0xF0)
     {           /* Midi event */
	MIDI_EVENT *ev;
	eve->type = MIDI;
	ev = malloc(sizeof(MIDI_EVENT));
	if (b >= 0x80)
	  {       /* Midi status byte */
	     scn->running_status = b;
	     b = nextbyte();
	  }
	else; /* FIXME what if running status was never defined? */
	ev->cmnd = (scn->running_status >> 4) & 0x07;
	ev->channel = scn->running_status & 0xf;
        if (b & 0x80) /* if (b > 0x7F) */
	  {  /* Not a legal midi data byte */
	     scn->error = WRONG_MIDI_DATA_BYTE;
	     return NULL;
	  }
	ev->ndat = numdat(ev->cmnd);
	ev->data[0] = b;
	if (ev->ndat == 2)
	  {
	     b = nextbyte();
	     if (b > 0x7F)
	       {  /* Not a legal midi datum byte */
		  scn->error = WRONG_MIDI_DATA_BYTE;
		  return NULL;
	       }
	     ev->data[1] = b;
	  }
	if (ev->cmnd == NOTE_ON && ev->data[1] == 0)
	  ev->cmnd = NOTE_OFF; /* Special case of note on with velocity = 0 */
	eve->p = ev;
     }
   else if (b == 0xFF)  /* Meta event */
     {
	META_EVENT *ev;
	eve->type = META;
	ev = malloc(sizeof(META_EVENT));
	b = nextbyte(); /* Byte defining meta event */
	if (b == 0x00)  /* Sequence number */
	  {
	     ev->cmnd = SEQ_NO;
	     ev->p = malloc(sizeof(unsigned short));
	     nextbyte();  /* Shoud be always 2 */
	     *((unsigned short *)(ev->p)) = 
	       nextbyte() * 256;
	     *((unsigned short *)(ev->p)) += nextbyte();
	  }
	else if (b > 0x00 && b < 0x08)
	  {  
	     unsigned long text_len;
	     int i;
	     switch(b)
	       {
		case 0x01:
		  ev->cmnd = TEXT;
		  break;
		case 0x02:
		  ev->cmnd = COPYRIGHT;
		  break;
		case 0x03:
		  ev->cmnd = TRACK_NAME;
		  break;
		case 0x04:
		  ev->cmnd = INSTR_NAME;
		  break;
		case 0x05:
		  ev->cmnd = LYRIC;
		  break;
		case 0x06:
		  ev->cmnd = MARKER;
		  break;
		case 0x07:
		  ev->cmnd = CUE_POINT;
		  break;
	       }	     
	     text_len = varlen();
	     ev->p = malloc((text_len + 1) * sizeof(char));
	     for (i = 0; i < text_len; i++)
	       ((char *)ev->p)[i] = nextbyte();
	     ((char *)ev->p)[text_len] = 0;
	  }
	else if (b == 0x20)  /* Midi Channel Prefix */
	  {
	     ev->cmnd = MIDI_CHANNEL_PREFIX;
	     nextbyte();  /* always = 1 */
	     ev->p = malloc(sizeof(char));
	     *((unsigned char *)(ev->p)) =  nextbyte();
	  }
	else if (b == 0x21)  /* Midi Channel Prefix */
	  {
	     ev->cmnd = MIDI_PORT;
	     nextbyte();  /* always = 1 */
	     ev->p = malloc(sizeof(char));
	     *((unsigned char *)(ev->p)) =  nextbyte();
	  }
	else if (b == 0x2F)  /* End of track */
	  {
	     nextbyte();  /* always = 0 */
	     ev->cmnd = M_END_OF_TRACK;
	     ev->p = NULL;
	  }
	else if (b == 0x51) /* Set tempo micro secs / quarter note */
	  {
	     int i;
	     nextbyte(); /* always = 3 */
	     ev->cmnd = SET_TEMPO;
	     ev->p = malloc(sizeof(unsigned long));
	     *((unsigned long *)(ev->p)) = 0;
	     for (i = 0; i < 3; i++)
	       *((unsigned long *)(ev->p)) =
	       *((unsigned long *)(ev->p)) * 256 + nextbyte();
	  }
	else if (b == 0x54) /* SMPTE Offset hh/mm/ss/fr/subfr */
	  {
	     nextbyte();  /* skip fixed lenght = 5 bytes*/
	     ev->cmnd = SMPTE_OFF;
	     ev->p = malloc(sizeof(SMPTE_OFFSET));
	     ((SMPTE_OFFSET *)ev->p)->hour = nextbyte();
	     ((SMPTE_OFFSET *)ev->p)->minute = nextbyte();
	     ((SMPTE_OFFSET *)ev->p)->second = nextbyte();
	     ((SMPTE_OFFSET *)ev->p)->frame = nextbyte();
	     ((SMPTE_OFFSET *)ev->p)->subframe = nextbyte();
	  }
	else if (b == 0x58)  /* Time signature */
	  {
	     nextbyte(); /* skip fixed lenght = 4 bytes*/
	     ev->cmnd = TIME_SIG;
	     ev->p = malloc(sizeof(TIME_SIGNATURE));
	     ((TIME_SIGNATURE *)ev->p)->nn = nextbyte();  
	                    /* Time Signature nn / 2^dd */
	     ((TIME_SIGNATURE *)ev->p)->dd = nextbyte();
	     ((TIME_SIGNATURE *)ev->p)->cc = nextbyte();  
	                    /* Midi clocks per Metronome tick */
	     ((TIME_SIGNATURE *)ev->p)->bb = nextbyte();  
	                    /* 1/32 notes per 24 Midi clocks */
	  }
	else if (b == 0x59)  /* Key Signature */
	  {
	     nextbyte();  /* skip fixed lenght = 2 byte */
	     ev->cmnd = KEY_SIG;
	     ev->p = malloc(sizeof(KEY_SIGNATURE));
	     ((KEY_SIGNATURE *)ev->p)->sf = nextbyte(); 
	                  /* >0 sharps <0 flats */
	     ((KEY_SIGNATURE *)ev->p)->mi = nextbyte(); 
	                  /* 0 = major 1 = minor */
	  }
	else if (b == 0x7F) /* var bytes of sequencer specific data */
	  {
	     int i;
	     VARLEN_DATA *vd;
	     ev->cmnd = SEQ_SPECIFIC;
	     ev->p = vd = malloc(sizeof(VARLEN_DATA));
	     vd->len = varlen();
	     vd->data = malloc(vd->len * sizeof(unsigned char));
	     for (i = 0; i < vd->len; i++)
	       (vd->data)[i] = nextbyte();
	  }
	else  /* Unsupported meta event FF XX l bytes long */
	  {
	     unsigned long len;
	     int i;
	     len = varlen();
	     ev->cmnd = UNKNOWN;
	     ev->p = NULL;
	     for (i = 0; i < len; i++)
		  nextbyte(); /* skip data */
	  }
	eve->p = ev;
     }
   else if (b == 0xF0)
     {
	SYSEX_EVENT *ev;
	VARLEN_DATA *vd;
	int i;
	ev = malloc(sizeof(SYSEX_EVENT));
	ev->p = vd = malloc(sizeof(VARLEN_DATA));
	vd->len = varlen();
	vd->data = malloc(vd->len * sizeof(unsigned char));
	for (i = 0; i < vd->len; i++)
	  (vd->data)[i] = nextbyte();
	if (vd->data[i - 1] != 0xF7)
	  {
	     fprintf (stderr, "Wrong sysex data termination\n");
	     abort();
	  }	
	eve->type = SYSEX;
	eve->p = ev;
     }
   else
     {
	scn->error = WRONG_LEADING_BYTE;
	return NULL;
     }
   return eve;
}

