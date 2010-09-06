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
#include <assert.h>
#define NDEBUG
static int inline append_music_event(MEAS_SCANNER *meas, MUSIC_EVENT *ev);
MEAS_SCANNER *meas_scanner_ist(SMF_SCANNER *s, GEN_INFO *g)
{
   MEAS_SCANNER *meas;
   meas = malloc(sizeof(MEAS_SCANNER));
   meas->mesc = me_scanner_ist(s, g);
   meas->smf_sc = s;
   meas->first_queue_event = NULL;
   meas->last_measure_event = NULL;
   meas->last_event = NULL;
   meas->last_event_tied = NULL;
   meas->dur_belonging_events = 0;
   meas->credit = 0;
   meas->time_num = g->time_num;
   meas->time_den = g->time_den;
   meas->end_of_track = 0;
   return meas;
}
void update_meas(MEAS_SCANNER *meas, GEN_INFO *g)
{
   meas->time_num = g->time_num;
   meas->time_den = g->time_den;
}
void inline clean_queue(MEAS_SCANNER *meas)
{
   MUSIC_EVENT_P *ev, *p;
   ev = meas->first_queue_event;
   while(ev)
     {
        if(ev->ev)
          free(ev->ev);
        p = ev;
        ev = ev->next;
        free(p);
     }
}
void meas_scanner_rls(MEAS_SCANNER *meas)
{
   clean_queue(meas);
   me_scanner_rls(meas->mesc);
   if(meas->last_event_tied)
     free(meas->last_event_tied);
   free(meas);
}
unsigned long queue_duration(MEAS_SCANNER *meas)
{
   unsigned long dur = 0;
   MUSIC_EVENT_P *ev = meas->first_queue_event;
   while (ev)
     {  /* If last_measure_event == NULL continue until queue end */
        dur += ev->ev->total_duration;
        ev = ev->next;
     }
   return dur;
}
int fill_queue(MEAS_SCANNER *meas)
{  /* Until strictly longer than measure duration */
   unsigned long dur, durml;
   unsigned long measure_duration = FULL * meas->time_num / meas->time_den;
   MUSIC_EVENT ev;
   int rc;
   if(meas->end_of_track)
     return END_OF_TRACK;
   dur = queue_duration(meas);
   while(dur <= measure_duration)
     {
        rc = get_music_event(meas->mesc, &ev);
        if (rc == END_OF_TRACK)
          break;
        rc = append_music_event(meas, &ev);
        dur += ev.total_duration;
     }
   /* complete last measure if necessary */
   if (rc == END_OF_TRACK)
     {
        unsigned long delta = measure_duration - dur;
        if(delta > 0)
          {
             meas->last_event->ev->total_duration += delta;
             dur += delta;
          }
	else if (delta == 0)
	  ; /* Do nothing */
        else
          {
             fprintf(stderr, "Unable to handle condition in fill_queue()\n");
	     fprintf(stderr, "measure_duration = %ld, dur = %ld\n",
		     measure_duration, dur);
             abort();
          }
        meas->end_of_track = 1;
     }   
   /* does last event span across measure end or finish at measure end? */
   /* duration of queue minus last event */
   durml = dur - meas->last_event->ev->total_duration;
   if (durml < measure_duration) 
     /* last event spans across measure end */
     {  
        meas->last_measure_event = meas->last_event;
        meas->dur_belonging_events = dur;
     }   
   else  /* last event begins exactly with next measure */
     {  
        meas->last_measure_event = meas->last_event->previous;
        meas->dur_belonging_events = measure_duration;
     }
   return RC_SUCCESS;
}
int append_music_event(MEAS_SCANNER *meas, MUSIC_EVENT *ev)
{
   if(!meas->first_queue_event) /* FIXME make creator */
     {
        meas->first_queue_event = malloc(sizeof(MUSIC_EVENT_P));
        meas->first_queue_event->ev = malloc(sizeof(MUSIC_EVENT));
        meas->first_queue_event->previous = NULL;
        meas->first_queue_event->next = NULL;
        *(meas->first_queue_event->ev) = *ev;
        meas->last_event = meas->first_queue_event;
     }
   else
     {
        MUSIC_EVENT_P *p;
        meas->last_event->next = malloc(sizeof(MUSIC_EVENT_P));
        p = meas->last_event;
        meas->last_event = meas->last_event->next;
        meas->last_event->ev = malloc(sizeof(MUSIC_EVENT));
        *(meas->last_event->ev) = *ev;
        meas->last_event->previous = p;
        meas->last_event->next = NULL;
     }
   return RC_SUCCESS;
}
void quantize_measure_termination(MEAS_SCANNER *meas)
{
   unsigned long measure_duration = FULL * meas->time_num / meas->time_den;
   long delta1, delta2;
   /* how far does queue extend into next measure */
   delta1 = meas->dur_belonging_events - measure_duration;
   /* how far is beginning of last event behind measure end */
   delta2 = meas->last_measure_event->ev->total_duration - delta1;
   assert(delta1 >= 0);
   if (delta1 > 0)
     {
	if (delta1 < MEASURE_TOL)
	  {
	     if (delta2 < MEASURE_TOL)
	       {
		  /* This should never happen event shorter than 2 * TOL */
		  /* The proper way todeal with this is to remove the event */
		  fprintf(stderr, "Too short a note across measure end\n");
		  abort();
	       }
	     else
	       {		  
		  /* shorten last measure event by delta1 */
		  meas->last_measure_event->ev->total_duration -= delta1;
		  if (meas->last_measure_event->ev->note_duration >
		      meas->last_measure_event->ev->total_duration)
		    meas->last_measure_event->ev->note_duration =
		    meas->last_measure_event->ev->total_duration;
		  meas->dur_belonging_events = measure_duration;
		  /* remember how much we have to lengthen next event */
		  meas->credit += delta1;
	       }	     
	  }
	else
	  {	     
	     if (delta2 < MEASURE_TOL)
	       { /* lengthen this measure shorten next */
		  meas->last_measure_event->previous->ev->total_duration += 
		    delta2;
		  meas->last_measure_event->ev->total_duration -= delta2;
		  if(meas->last_measure_event->ev->note_duration > delta2)
		    {
		       meas->last_measure_event->ev->note_duration -= delta2;
		    }
		  else
		    {
		       meas->last_measure_event->ev->note_duration = 0;
		       meas->last_measure_event->ev->type = ME_REST;
		    }		  
		  meas->last_measure_event = meas->last_measure_event->previous;
		  meas->dur_belonging_events = measure_duration;
	       }
	     else;  /* Do nothing */
	  }	
     }
   else; /* Last measure event ends exactly with measure no quantization
	  * required */
}
/* Return:
 1 if last_measure event ends exacty at measur end
 2 if last rest spans across measur end or exactly begins at mesure end
 3 if last note spans across measure end
 */
int inline termination_case(MEAS_SCANNER *meas)
{
   unsigned long measure_duration = FULL * meas->time_num / meas->time_den;
   if (meas->dur_belonging_events == measure_duration)
     return 1;
   else /* must be > */
     {
        if (meas->dur_belonging_events -
            meas->last_measure_event->ev->total_duration +
            meas->last_measure_event->ev->note_duration
            <= measure_duration)
          return 2; /* last rest extends across measure end */
        else
          return 3; /* last note extends across measure end */
     }
}
void inline split_rest_across_measure(MEAS_SCANNER *meas)
{   
   unsigned long measure_duration = FULL * meas->time_num / meas->time_den;
   unsigned long delta;
   MUSIC_EVENT_P *mev, *p;
   unsigned char last_measure_event_was_last_event =
     meas->last_measure_event == meas->last_event;
   mev = malloc(sizeof(MUSIC_EVENT_P)); /* future 1st event of next measure */
   mev->ev = malloc(sizeof(MUSIC_EVENT));   
   delta = meas->dur_belonging_events - measure_duration;
   mev->ev->type = ME_REST;
   mev->ev->note_duration = 0;
   mev->ev->note = 0;
   mev->ev->velocity = 0;
   mev->ev->total_duration = delta;
   mev->ev->flags = 0;
   meas->last_measure_event->ev->total_duration -= delta;
   /* Inset new event int queue */
   p = meas->last_measure_event->next;
   meas->last_measure_event->next = mev;
   mev->previous = meas->last_measure_event;
   mev->next = p;
   if (last_measure_event_was_last_event)
     meas->last_event = mev;
}  
void inline split_note_across_measure(MEAS_SCANNER *meas)
{   
   unsigned long measure_duration = FULL * meas->time_num / meas->time_den;
   unsigned long delta, new_total_duration;
   MUSIC_EVENT_P *mev, *p;
   unsigned char last_measure_event_was_last_event =
     meas->last_measure_event == meas->last_event;
   mev = malloc(sizeof(MUSIC_EVENT_P)); /* future 1st event of next measure */
   mev->ev = malloc(sizeof(MUSIC_EVENT));   
   delta = meas->dur_belonging_events - measure_duration;
   new_total_duration = meas->last_measure_event->ev->total_duration - delta;
   mev->ev->type =  ME_NOTE;
   mev->ev->total_duration = delta;
   mev->ev->note_duration = meas->last_measure_event->ev->note_duration -
     new_total_duration;
   mev->ev->note = meas->last_measure_event->ev->note;
   mev->ev->velocity = meas->last_measure_event->ev->velocity;
   mev->ev->flags = 0;
   meas->last_measure_event->ev->total_duration = new_total_duration;
   meas->last_measure_event->ev->note_duration = new_total_duration;
   meas->last_measure_event->ev->flags = ME_TIED;
   p = meas->last_measure_event->next;
   meas->last_measure_event->next = mev;
   mev->previous = meas->last_measure_event;
   mev->next = p;
   if (last_measure_event_was_last_event)
     meas->last_event = mev;
}  
int get_measure(MEAS_SCANNER *meas, MEASURE *m)
{
   int rc;
   MUSIC_EVENT_P *le;
   if (meas->last_event_tied)
     {
        m->previous_measure_event = meas->last_event_tied;
        meas->last_event_tied = NULL;
     }
   else
     m->previous_measure_event = NULL;
   rc = fill_queue(meas);
   if (meas->credit)
     {
	MUSIC_EVENT *ev = meas->first_queue_event->ev;
	ev->total_duration += meas->credit;
	if (ev->note_duration)
	  ev->note_duration += meas->credit;
	meas->credit = 0;
     }   
   if (rc == END_OF_TRACK)
     return rc;
   quantize_measure_termination(meas);
   switch (termination_case(meas))
     {
      case 1:  /* last measure event ends exactly at measure end */
	       /* there is nothing to do */
        break;
      case 2:  /* rest spans across measure end or begins with next measure */
        split_rest_across_measure(meas);
        break;
      case 3:  /* note spans across measure */
        split_note_across_measure(meas);
        break;
      default:
        fprintf(stderr, "get_measure(): Returning RC_FAIL\n");
        return RC_FAIL;
     }
   m->first_measure_event = meas->first_queue_event;
   assert(m->first_measure_event->previous == NULL);
   m->last_measure_event = meas->last_measure_event;
   le = m->last_measure_event;
   if(
       (!le->next) /* there is no next measure */
                              ||
       (le->next->ev->type == ME_REST)
                              ||
       (
         (le->ev->type == ME_NOTE)
                              &&
         (le->ev->note == le->next->ev->note) /* next note equal */
                              &&
         !(le->ev->flags & ME_TIED)
       )
     ) le->ev->flags |= ME_NO_SLUR;
   m->time_num = meas->time_num;
   m->time_den = meas->time_den;
   m->duration = 4 * QUARTER * m->time_num / m->time_den;
   /* Clear measure; assume first_measure_event = first_queue_event */
   if (meas->last_measure_event->ev->flags & ME_TIED)
     {  
        meas->last_event_tied = malloc(sizeof(MUSIC_EVENT));
        *(meas->last_event_tied) = *(meas->last_measure_event->ev);
     }
   meas->first_queue_event = meas->last_measure_event->next;
   if (meas->first_queue_event)
     meas->first_queue_event->previous = NULL;
   meas->last_measure_event = NULL; /* Undefined */
   return RC_SUCCESS;
}
/* Free sub-queue of events in this measure null everyting */
void clear_measure(MEASURE *m)
{
   MUSIC_EVENT_P *ev, *p;
   ev = m->first_measure_event;
   while(1)
     {
        if(ev->ev)
          free(ev->ev);
        p = ev;
        ev = ev->next;
        free(p);
        if(p == m->last_measure_event)
          break;
     }
   if(m->previous_measure_event)
     {       
        free(m->previous_measure_event);
        m->previous_measure_event = NULL;
     }
   if (m->last_measure_event->next)
     m->last_measure_event->next->previous = NULL;
   m->first_measure_event = m->last_measure_event = NULL;   
}
