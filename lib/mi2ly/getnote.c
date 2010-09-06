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
#include <limits.h>
static inline void complete_all_notes(NOTE_SCANNER *ns, SMF_EVENT *s)
{
   NOTE_EVENT_P *p;
   p = ns->nq->first_event;
   while (p)
     {
        if (p->ev->duration == ULONG_MAX)
          p->ev->duration = s->time * QUARTER_DURATION / 
          ns->smf_sc->ticks_quarter_note
          - p->ev->time;
        p = p->next;
     }
}
/* FIXME What if NOTE ON on same note twice before  NOTE OFF ? */
static inline void complete_note_event(NOTE_SCANNER *ns, SMF_EVENT *s)
{
   NOTE_QUEUE *nq = ns->nq;
   NOTE_EVENT_P *p;
   MIDI_EVENT *mip = s->p;
   p = nq->first_event;
   nq->last_time = s->time * QUARTER_DURATION / ns->smf_sc->ticks_quarter_note;
   while (p)
     {
        if (p->ev->note == mip->data[0] && p->ev->duration == ULONG_MAX)
	  {	     
	     p->ev->duration = nq->last_time - p->ev->time;
	     break;  /* complete only one event FIFO style */
	  }	
        p = p->next;
     }
}
static inline int get_note_event_from_queue(NOTE_QUEUE *nq, NOTE_EVENT *nev)
{
   if (!nq->first_event)
     return QUEUE_EMPTY;
   /* force note off for notes which have been on for too long a time */
   if (nq->last_time -
       nq->first_event->ev->time > MAX_NOTE_DURATION)
     nq->first_event->ev->time = MAX_NOTE_DURATION;
   /* return first note in queue if it was shut down, clear position in queue */
   if (nq->first_event->ev->duration != ULONG_MAX)
     {
        NOTE_EVENT_P *p;
        *nev = *(nq->first_event->ev);
        free (nq->first_event->ev);
        p = nq->first_event;
        nq->first_event = nq->first_event->next;
        if (nq->first_event)
          nq->first_event->previous = NULL;
        free(p);
        return RC_SUCCESS;      
     }
   else
     return FIRST_NOTE_INCOMPLETE;
}
/* Insert midi event as last event of queue */
static inline void insert_note_on(NOTE_SCANNER *ns, SMF_EVENT *s)
{
   NOTE_QUEUE *nq = ns->nq;
   NOTE_EVENT_P *a;
   NOTE_EVENT *b;
   MIDI_EVENT *mip = s->p;
   b = malloc(sizeof(NOTE_EVENT));
   a = malloc(sizeof(NOTE_EVENT_P));
   a->ev = b;
   a->previous = nq->last_event;
   a->next = NULL;
   if (nq->first_event == NULL)  /* queue was empty? */
     nq->first_event = nq->last_event = a;
   else
     {  
        nq->last_event->next = a;
        nq->last_event = a;
     }
   b->note = mip->data[0];
   nq->last_time = b->time = 
     s->time * QUARTER_DURATION / ns->smf_sc->ticks_quarter_note;
   b->duration = ULONG_MAX;
   b->velocity = mip->data[1];
}
NOTE_SCANNER * note_scanner_ist(SMF_SCANNER *s, GEN_INFO * g)
{
   NOTE_SCANNER * ns;
   NOTE_QUEUE * nq;
   ns = malloc(sizeof(NOTE_SCANNER));
   if (!ns)
     return NULL;
   ns->smf_sc = s;
   nq = malloc(sizeof(NOTE_QUEUE));
   if (!nq)
     {
        free(ns);
        return NULL;
     }
   ns->nq = nq;
   ns->end_of_track = 0;
   ns->active_channel = g->active_channel;
   ns->initial_skip_time = g->initial_skip;
   nq->first_event = nq->last_event = NULL;
   nq->last_time = 0;
   return ns;
}
void note_scanner_rls(NOTE_SCANNER *ns)
{
   free(ns->nq); /* FIXME verify that queue is empty ! */
   free(ns);
}
int get_note_event(NOTE_SCANNER *ns, NOTE_EVENT *nev)
{
   SMF_EVENT *ev;
   int rc;
   while (1)
     {
        MIDI_EVENT *mip;
        META_EVENT *mtp;
        rc = get_note_event_from_queue(ns->nq, nev);
        if (rc == RC_SUCCESS)
          return rc;
        if (rc == QUEUE_EMPTY && ns->end_of_track)
          return END_OF_TRACK;
        while(1) /* rc == FIRST_NOTE_INCOMPLETE || QUEUE_EMPTY */
          {          
             ev = smf_event(ns->smf_sc);
             if (ev->type == MIDI)
               {
                  mip = ev->p;
		  if (ns->active_channel == ALL_CHANNELS ||
		      mip->channel == ns->active_channel)
		    {		       
		       if (mip->cmnd == NOTE_ON || mip->cmnd == NOTE_OFF) 
			 break;
		       else;  /* Ignore other midi events */
		    }		  
               }
             else if (ev->type == META)
               {
                  mtp = ev->p;
                  if (mtp->cmnd == M_END_OF_TRACK)
                    {
                       rc = END_OF_TRACK;
                       break;
                    }
		  else if (mtp->cmnd == MIDI_PORT)
		    free(mtp->p);
                  else;  /* Ignore other meta events */
               }
             else;  /* Ignore Sysex events */
             smf_event_rls(ev);
          }
        if (rc == END_OF_TRACK)
          {
             ns->end_of_track = 1;
             complete_all_notes(ns, ev);
          }     
        else
          {
             if (mip->cmnd == NOTE_ON)
               insert_note_on(ns, ev);
             else if (mip->cmnd == NOTE_OFF)
               complete_note_event(ns, ev);
          }
        smf_event_rls(ev);
     }
}
/* Create an istance for a music event scanner */
ME_SCANNER *me_scanner_ist(SMF_SCANNER *s, GEN_INFO *g)
{
   ME_SCANNER *mesc;
   mesc = malloc(sizeof(ME_SCANNER));
   mesc->ns = note_scanner_ist(s, g);
   mesc->end_of_track = 0;
   mesc->first_time = 1;
   return mesc;
}
/* Release a music event scanner */
void me_scanner_rls(ME_SCANNER *mesc)
{
   note_scanner_rls(mesc->ns);
   free(mesc);
}
int get_music_event(ME_SCANNER *mesc, MUSIC_EVENT *me)
{
   int rc;
   if (mesc->first_time)
     {  
        mesc->end_of_track = 0;
        mesc->first_time = 0;
        rc = get_note_event(mesc->ns, &(mesc->nev_old));
        if (rc == END_OF_TRACK)
          return rc;
        if (mesc->nev_old.time < mesc->ns->initial_skip_time)
          {
	     char msg[] = 
	 "Abnormal termination: trying to skip initial notes instead of rest\n";
             logmsg(msg);
	     fprintf(stderr, msg);
             abort();
          }          
        if (mesc->nev_old.time > mesc->ns->initial_skip_time)
          {  /* return rest with no note */
             me->type = ME_REST;
             me->note = 0;
             me->velocity = 0;
             me->note_duration = 0;
             me->total_duration = mesc->nev_old.time 
               - mesc->ns->initial_skip_time;
             me->flags = 0;
             return RC_SUCCESS;
          }
     }
   /* If not first time come here */
   if (mesc->end_of_track)
     return END_OF_TRACK;
   rc = get_note_event(mesc->ns, &mesc->nev);
   if (rc == END_OF_TRACK)  /* Last note received was last note */
     {
        me->type = ME_NOTE;
        me->note = mesc->nev_old.note;
        me->note_duration = mesc->nev_old.duration;
        me->total_duration = mesc->ns->smf_sc->cur_time
          * QUARTER_DURATION / 
          mesc->ns->smf_sc->ticks_quarter_note - mesc->nev_old.time;
        me->flags = 0;
        mesc->end_of_track = 1;
        return RC_SUCCESS;  /* END_OF_TRACK next time */
     }
   me->type = ME_NOTE;
   me->note = mesc->nev_old.note;
   me->note_duration = mesc->nev_old.duration;
   me->total_duration = mesc->nev.time - mesc->nev_old.time;
   me->flags = 0;            
   if(mesc->nev.time < (mesc->nev_old.time + mesc->nev_old.duration))
     {  /* Notes overlap: shorten old note */
        me->note_duration = me->total_duration;
     }
   /* FIXME discard zero duration notes */
   mesc->nev_old = mesc->nev;
   return RC_SUCCESS;
}
   
               

