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
static GEN_INFO *gen;
static char * midi_mess_text[] =
              {
              "Note Off",
              "Note On ",
              "After Touch (key)",
              "Control Change",
              "Program Change",
              "After Touch (poly)",
              "Pitch Wheel",
              "System Exclusive"
              };
/* Dunmp of raw midifile */
int dump_track(SMF_SCANNER *s, FILE *fdump, unsigned char active_channel)
{
   /* In this routine times are in midi ticks */
  SMF_EVENT *ev;
   unsigned char allch = active_channel == ALL_CHANNELS;
   while(1)
     {
        ev = smf_event(s);
        if (allch || ev->type != MIDI ||
	    ((MIDI_EVENT *)ev->p)->channel == active_channel)
          {
	     fprintf(fdump,"%8.2lf ", 
		     (double)s->cur_time / (double)s->ticks_quarter_note);
	  }	
        if (ev->type == MIDI)
          {
             MIDI_EVENT *p = ev->p;
	     if (active_channel == ALL_CHANNELS || 
		 p->channel == active_channel)
	       {		  
		  int i;
		  fprintf(fdump, "%2d  ", p->channel);
		  fprintf(fdump,"%s",midi_mess_text[p->cmnd]);
		  for (i = 0; i < p->ndat; i++)
		    fprintf(fdump," %4d",p->data[i]);
		  fprintf(fdump,"\n");
	       }	     
          }       
        else if (ev->type == META)
          {
             META_EVENT *p = ev->p;
             if (p->cmnd == M_END_OF_TRACK)
               {
                  fprintf(fdump,"End of track\n");
                  smf_event_rls(ev);
                  break;
               }             
             if (p->cmnd >= 1 && p->cmnd <= 7)
               {                 
                  switch (p->cmnd)
                    {
                     case TEXT:
                       fprintf(fdump,"Text Event");
                       break;
                     case COPYRIGHT:
                       fprintf(fdump,"Copyright Notice");
                       break;
                     case TRACK_NAME:
                       fprintf(fdump,"Sequence/Track Name");
                       break;
                     case INSTR_NAME:
                       fprintf(fdump,"Instrument Name");
                       break;
                     case LYRIC:
                       fprintf(fdump,"Lyric");
                       break;
                     case MARKER:
                       fprintf(fdump,"Marker");
                       break;
                     case CUE_POINT:
                       fprintf(fdump,"Cue Point");
                       break;
                    }
                  fprintf(fdump,": %s\n",(char *)p->p);
               }             
             else if (p->cmnd == MIDI_CHANNEL_PREFIX)
                  fprintf(fdump,"Midi Channel Prefix %d\n",
                          *(unsigned char*)p->p);
             else if (p->cmnd == MIDI_PORT)
	       {		  
                  fprintf(fdump,"Midi Port No %d\n",
                          *(unsigned char*)p->p);
		  free(p->p);
	       }	     
             else if (p->cmnd == SET_TEMPO)
               {
                  unsigned long micro_secs = *(unsigned long *)p->p;
                  fprintf(fdump,"Set Tempo %lu microseconds / quarter note\n",
                          micro_secs);
                  fprintf(fdump,"             i.e. %lg quarter notes per minute\n", 
                          60e6/micro_secs);
               }
             else if (p->cmnd == SMPTE_OFF)
               {
                  SMPTE_OFFSET *pp = (SMPTE_OFFSET *)p->p;
                  fprintf(fdump,"SMPTE Offset:\n");
                  fprintf(fdump,"hour     = %d\n", pp->hour);
                  fprintf(fdump,"minute   = %d\n", pp->minute);
                  fprintf(fdump,"second   = %d\n", pp->second);
                  fprintf(fdump,"frame    = %d\n", pp->frame);
                  fprintf(fdump,"subframe = %d\n", pp->subframe);
               }             
             else if (p->cmnd == TIME_SIG)
               {
                  TIME_SIGNATURE *pp = (TIME_SIGNATURE *)p->p;
                  unsigned long denom = 1;
                  int i;
                  for(i = 0; i < pp->dd; i++)
                    denom *= 2;
                  fprintf(fdump,"Time Signature %d / %ld\n",pp->nn,denom);
                  fprintf(fdump,
                 "            %d Midi clocks per Metronome tick\n",pp->cc);
                  fprintf(fdump,
                 "            %d 1/32 notes per 24 Midi clocks\n",pp->bb);
               }             
             else if (p->cmnd == KEY_SIG)
               {
                  KEY_SIGNATURE *pp = (KEY_SIGNATURE *)p->p;
                  fprintf(fdump,"Key Signature ");
                  if (pp->sf >= 0)
                    fprintf(fdump,"%d sharps ",pp->sf);
                  else
                    fprintf(fdump,"%d flats ",-pp->sf);
                  if (pp->mi)
                    fprintf(fdump,"minor\n");
                  else
                    fprintf(fdump,"major\n");
               }             
             else if (p->cmnd == SEQ_SPECIFIC)
               {
                  fprintf(fdump,
                    "Sequencer specific data - Sorry no decoding\n");
               }             
          }
        else if (ev->type == SYSEX)
          ;  /* Ignore it */
        smf_event_rls(ev);
     }
   return MATCH;
}
int skip_track(SMF_SCANNER *s)
{
  SMF_EVENT *ev;
   while(1)
     {
        ev = smf_event(s);
        if (ev->type == META)
          {
             META_EVENT *p = ev->p;
             if (p->cmnd == M_END_OF_TRACK)
               {
                  smf_event_rls(ev);
                  break;
               }
	     if (p->cmnd == MIDI_PORT)
	       free(p->p);
          }     
        smf_event_rls(ev);
     }
   return MATCH;
}
int dump_smf(GEN_INFO *g)
{   
  int i, rc;
  SMF_SCANNER *scn;
  gen = g;
  scn = smf_scn_inst(g->fmid, g->str);
  if (!scn)
     {  
        fprintf(stderr, "Unsuccesful scanner creation\n");
        abort();
     }
  rc = smf_scn_init(scn);
  if (rc == NO_MATCH)
     {
	smf_scn_rls(scn);
	return rc;
     }   
  else
    {
       rewind(g->fout);
       fprintf(g->fout,"HEADER SIGNATURE\n");
       fprintf(g->fout,"Lenght of header data = %ld bytes\n",scn->hlenght);
       fprintf(g->fout,"Format = %ld\n",scn->format);
       fprintf(g->fout,"Number of tracks = %ld\n",scn->ntrack);
       fprintf(g->fout,"%ld Ticks per quarter note\n", 
               scn->ticks_quarter_note);
       for (i = 0; i < scn->ntrack; i++)
         {
            rc = init_next_track(scn);
            if (rc == NO_MATCH)
              {
                fprintf(stderr, "no match in init_next_track(); aborting...\n");
                abort();
              }
            fprintf(g->fout,"\nDump of Track %d:\n",i);
            fprintf(g->fout,"TRACK SIGNATURE\n");
            fprintf(g->fout,"Lenght of track data = %ld bytes\n",
                    scn->left_track_bytes);
            fprintf(g->flog,"\nParsing Track %d:\n",i);
            rc = dump_track(scn, g->fout, g->active_channel);
            fflush(g->fout);
            if (rc == NO_MATCH)
              break;
         }
    }
  smf_scn_rls(scn);
  return rc;
}
char *timestring(unsigned short meas_no, unsigned long t)
{
/* In this routine times are in internal units defined in midi2ly.h */
   static char buf[12];
   unsigned long q, r;
   q = t / QUARTER_DURATION;
   r = (t % QUARTER_DURATION) * 100 / QUARTER_DURATION;
   sprintf(buf, "%3u:%02lu:%02lu", meas_no, q, r);
   return buf;
}
char *timestring1(unsigned long t)
{
/* In this routine times are in internal units defined in midi2ly.h */
   static char buf[12];
   unsigned long q, r;
   q = t / QUARTER_DURATION;
   r = (t % QUARTER_DURATION) * 100 / QUARTER_DURATION;
   sprintf(buf, "%04lu:%02lu", q, r);
   return buf;
}
char *durstring(unsigned long t)
{
/* In this routine times are in internal units defined in midi2ly.h */
   static char buf[8];
   unsigned long q, r;
   q = t / QUARTER_DURATION;
   r = (t % QUARTER_DURATION) * 100 / QUARTER_DURATION;
   sprintf(buf, "%3lu:%02lu", q, r);
   return buf;
}
static char *notenames[] =
{
   "C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B "
};
char *notestring(unsigned char n)
{
   static char buf[5];
   int i;
   if (n)
     {  
        i = n % 12;
        sprintf(buf, "%2s %1d", notenames[i], n / 12);
     }
   else
     strcpy(buf,"REST");
   return buf;
}
int dump_track_1(GEN_INFO *g)
{
   /* In this routine times are in internal units defined in midi2ly.h */
  int i, rc;
  SMF_SCANNER *scn;
  NOTE_EVENT nev;
  gen = g;
  scn = smf_scn_inst(g->fmid, g->str);
  if (!scn)
     {  
        fprintf(stderr, "Unsuccesful scanner creation\n");
        abort();
     }
  rc = smf_scn_init(scn);
  if (rc == NO_MATCH)
     {
	smf_scn_rls(scn);	
	return rc;
     }   
  else
    {
       rewind(g->fout);
       for (i = 0; i < scn->ntrack; i++)
         {
            rc = init_next_track(scn);
            if (i == g->active_track)
              {
                 NOTE_SCANNER *nes;
                 fprintf(g->fout,"\nDump of Note Events in Track %d:\n",i);
                 nes = note_scanner_ist(scn, g);
                 while (1)
                   {  
                      rc = get_note_event(nes, &nev);
                      if (rc == END_OF_TRACK)
                        break;
                      fprintf(g->fout,"%s ",timestring1(nev.time));
                      fprintf(g->fout,"%s %s\n", notestring(nev.note), 
			      durstring(nev.duration));
                   }
                 note_scanner_rls(nes);
              }
           else
              {
                 fprintf(g->flog,"\nSkipping Track %d:\n",i);
                 rc = skip_track(scn);  /* skip track */
                 if (rc == NO_MATCH)
                   break;
              }    
         }
    }
  smf_scn_rls(scn);
  return rc;
}
int dump_track_2(GEN_INFO *g)
{
   int i, rc;
   MUSIC_EVENT me;
   SMF_SCANNER *scn;
   gen = g;
   scn = smf_scn_inst(g->fmid, g->str);
   if (!scn)
     {  
        fprintf(stderr, "Unsuccesful scanner creation\n");
        abort();
     }
   rc = smf_scn_init(scn);
   if (rc == NO_MATCH)
     {
	smf_scn_rls(scn);	
	return rc;
     }   
   else
     {
        rewind(g->fout);
        for (i = 0; i < scn->ntrack; i++)
         {
            rc = init_next_track(scn);
            if (i == g->active_track)
              {
                 ME_SCANNER *mesc;
                 unsigned long cur_time = 0;
		 unsigned short no = 0;
		 unsigned long old_cur_time = 0;
		 unsigned long measure_duration;
		 unsigned long delta_t;
                 fprintf(g->fout,"\nDump of Note Events in Track %d:\n",i);
                 fprintf(g->fout,
               "  Attack  Note  Note Dur  Rest Dur Tot.Dur\n");
                 mesc = me_scanner_ist(scn, g);
		 update_directives(g, no);
		 measure_duration = FULL * g->time_num / g->time_den;
		 delta_t = 0;
                 while (1)
                   {
		      unsigned long t;
                      rc = get_music_event(mesc, &me);
                      if (rc == END_OF_TRACK)
                        break;
                      t = delta_t % measure_duration;
                      fprintf(g->fout,"%s ", timestring(no + 1, t));
                      fprintf(g->fout,"%s  %s", notestring(me.note), 
                              durstring(me.note_duration));
                      fprintf(g->fout,"     %s",
                              durstring(me.total_duration - me.note_duration));
                      fprintf(g->fout," %s",durstring(me.total_duration));
                      fprintf(g->fout,"   %3.2lf\n",
                              (double)me.note_duration / me.total_duration);
                      cur_time += me.total_duration;
		      delta_t = cur_time - old_cur_time;
		      while (delta_t >= measure_duration)
			{
			   no += 1;
			   fprintf(g->fout,"\n");
			   delta_t -= measure_duration;
			   old_cur_time += measure_duration;
			   update_directives(g, no);
			   measure_duration = FULL * g->time_num / g->time_den;
			}			   
                   }
                 me_scanner_rls(mesc);
              }
           else
              {
                 fprintf(g->flog,"\nSkipping Track %d:\n",i);
                 rc = skip_track(scn);  /* skip track */
                 if (rc == NO_MATCH)
                   break;
              }    
         }
    }
  smf_scn_rls(scn);
  return rc;
}
void dump_measure(MEASURE *mea, FILE *fdump)
{
   MUSIC_EVENT_P *p;
   unsigned long cur_time = 0;
   if (mea->previous_measure_event)
     {                                               
        MUSIC_EVENT me = *(mea->previous_measure_event);
        fprintf(fdump,"PREV. ME  ");
        fprintf(fdump,"%s  %s", notestring(me.note), 
                durstring(me.note_duration));
        fprintf(fdump,"     %s",
           durstring(me.total_duration - me.note_duration));
        fprintf(fdump," %s",durstring(me.total_duration));
        fprintf(fdump,"   %3.2lf\n",
           (double)me.note_duration / me.total_duration);
     }                     
   p = mea->first_measure_event;
   while (1)
     {
        MUSIC_EVENT me = *(p->ev);
        fprintf(fdump,"%s ",
           timestring(mea->no + 1, cur_time));
        fprintf(fdump,"%s  %s", notestring(me.note), 
                durstring(me.note_duration));
        fprintf(fdump,"     %s",
           durstring(me.total_duration - me.note_duration));
        fprintf(fdump," %s",durstring(me.total_duration));
        cur_time += me.total_duration;
        fprintf(fdump,"   %3.2lf",
           (double)me.note_duration / me.total_duration);
        if (me.flags & ME_TIED)
          fprintf(fdump,"  TIED\n");
        else if (me.flags & ME_NO_SLUR)
          fprintf(fdump,"  NO SLUR\n");                      
        else
          fprintf(fdump,"\n");
        if (p == mea->last_measure_event)
          break;
        p = p->next;
     }
   fprintf(fdump,"\n");
}
void dump_measure_1(MEASURE *mea, FILE *fdump)
{
   MUSIC_EVENT_P *p;
   unsigned long cur_time = 0;
   p = mea->first_measure_event;
   while (1)
     {
        MUSIC_EVENT me = *(p->ev);
        fprintf(fdump,"%s ",
           timestring(mea->no + 1, cur_time));
	if (me.type == TRIPLET_BEGIN)
	  fprintf (fdump, "TRIPLET_BEGIN\n");
	else if (me.type == TRIPLET_END)
	  fprintf (fdump, "TRIPLET_END\n");
	else if (me.type == DUIN_END)
	  fprintf (fdump, "DUIN_END\n");
	else if (me.type == DUIN_BEGIN)
	  fprintf (fdump, "DUIN_BEGIN\n");
	else
	  {	     
	     fprintf(fdump,"%s  %s", notestring(me.note), 
		     durstring(me.note_duration));
	     fprintf(fdump,"     %s",
		     durstring(me.total_duration - me.note_duration));
	     fprintf(fdump," %s",durstring(me.total_duration));
	     cur_time += me.total_duration;
	     fprintf(fdump,"   %3.2lf",
		     (double)me.note_duration / me.total_duration);
	     if (me.flags & ME_TIED)
	       fprintf(fdump,"  TIED");
	     if (me.flags & ME_NO_SLUR)
	       fprintf(fdump,"  NO SLUR");                      
	     if (me.flags & ME_SLURRED)
	       fprintf(fdump,"  SLURRED");                      
	     if (me.flags & ME_STACCATO)
	       fprintf(fdump,"  STACCATO");                      
	     fprintf(fdump,"\n");
	  }	
	if (p == mea->last_measure_event)
	  break;
	p = p->next;
     }
   fprintf(fdump,"\n");
}
int dump_track_3(GEN_INFO *g)
{
   int i, rc;
   MEASURE mea;
   SMF_SCANNER *scn;
   gen = g;
   scn = smf_scn_inst(g->fmid, g->str);
   if (!scn)
     {  
        fprintf(stderr, "Unsuccesful scanner creation\n");
        abort();
     }
   rc = smf_scn_init(scn);
   if (rc == NO_MATCH)
     {	
	smf_scn_rls(scn);
	return rc;
     }   
   else
     {
        rewind(g->fout);
        for (i = 0; i < scn->ntrack; i++)
         {
            rc = init_next_track(scn);
            if (i == g->active_track)
              {
                 MEAS_SCANNER *meas;
		 unsigned long measure_no;
                 fprintf(g->fout,"\nDump of Note Events in Track %d:\n",i);
                 fprintf(g->fout,
               "  Attack  Note  Note Dur  Rest Dur Tot.Dur\n");
                 measure_no = 0; /* Internal numbering is base 0 */
		 update_directives(g, measure_no);
                 meas = meas_scanner_ist(scn, g);
                 while (1)
                   {
                      rc = get_measure(meas, &mea);
                      mea.no = measure_no++;
                      mea.duration = meas->time_num * FULL / meas->time_den;
                      if (rc)
                        break;
                      dump_measure(&mea, g->fout);
		      fflush(g->fout);
                      clear_measure(&mea);
		      update_directives(g, measure_no);
		      update_meas(meas, g);
                   }
                 meas_scanner_rls(meas);
              }
           else
              {
                 fprintf(g->flog,"\nSkipping Track %d:\n",i);
                 rc = skip_track(scn);  /* skip track */
                 if (rc == NO_MATCH)
                   break;
              }    
         }
    }
  smf_scn_rls(scn);
  return rc;
}
int dump_track_4_5(GEN_INFO *g)
{
   int i, rc;
   unsigned short measure_no;
   MEASURE mea;
   SMF_SCANNER *scn;
   gen = g;
   scn = smf_scn_inst(g->fmid, g->str);
   if (!scn)
     {  
        fprintf(stderr, "Unsuccesful scanner creation\n");
        abort();
     }
   rc = smf_scn_init(scn);
   if (rc == NO_MATCH)
     {	
	smf_scn_rls(scn);
	return rc;
     }   
   else
     {
        for (i = 0; i < scn->ntrack; i++)
         {
            rc = init_next_track(scn);
            if (i == g->active_track)
              {
                 MEAS_SCANNER *meas;
                 fprintf (gen->fout, "%% Decoding Track %d\n", i);
                 measure_no = 0; /* Internal numbering is base 0 */
		 update_directives(g, measure_no);
                 meas = meas_scanner_ist(scn, g);
                 while (1)
                   {
                      rc = get_measure(meas, &mea);                   
                      if (rc)
                        break;
                      mea.no = measure_no++;
                      mea.duration = meas->time_num * FULL / meas->time_den;
                      if (g->dump_mode == 4)
			{			   
			   dump_measure(&mea, g->fout);
			   fflush(g->fout);
			}		      
                      rc = quantize_measure(&mea);
                      dump_measure_1(&mea, g->fout);
		      fflush(g->fout);
                      clear_measure(&mea);
		      update_directives(g, measure_no);
		      update_meas(meas, g);
                   }
                 meas_scanner_rls(meas);
              }
           else
              {
                 fprintf(g->flog,"\nSkipping Track %d:\n",i);
                 rc = skip_track(scn);  /* skip track */
                 if (rc == NO_MATCH)
                   break;
              }    
         }
    }
  smf_scn_rls(scn);
  return rc;
}

