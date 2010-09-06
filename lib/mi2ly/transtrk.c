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



#define DEBUG 0
#include "mi2ly.h"
#include <assert.h>
#define NDEBUG
extern unsigned long **att_sec_times1;
static GEN_INFO *gen;
void inline correct(MUSIC_EVENT *ev, unsigned char);
void insert_initial_modifier(MUSIC_INTERVAL *in, unsigned char modifier);
void insert_final_modifier(MUSIC_INTERVAL *in, unsigned char modifier);
unsigned char inline numatt(MUSIC_INTERVAL *in);
void get_attack_times(unsigned long *att_times, MUSIC_INTERVAL *in);
void quantize_note_duration(MUSIC_EVENT *me);
unsigned long inline abs_delta(unsigned long t1, unsigned long t2)
{   
   if (t1 >= t2)
     return t1 - t2;
   else
     return t2 - t1;
}
/* TRIPLET = 0; DUIN = 1 */
unsigned long correct_num[] = 
{3L, 2L,};
unsigned long correct_den[] = 
{2L, 3L,};
   
void inline correct(MUSIC_EVENT *ev, unsigned char modifier)
{
   ev->note_duration = ev->note_duration * correct_num[modifier] /
     correct_den[modifier];
   ev->total_duration = ev->total_duration * correct_num[modifier] /
     correct_den[modifier];
}
/* Get times of note attacks, neglect 1st implicit attack at time 0 */
void get_attack_times(unsigned long *times, MUSIC_INTERVAL *in)
{
   MUSIC_EVENT_P *ev;
   unsigned long curtime;
   int i;
   curtime = 0;
   i = 0;
   ev = in->first_event;
   while (1)
     {
        if (ev->ev->note_duration > 0 && curtime !=0)
          times[i++] = curtime;
        curtime += ev->ev->total_duration;
        if (ev == in->last_event)
          break;
        ev = ev->next;
     }
}
void inline insert_initial_modifier(MUSIC_INTERVAL *in, unsigned char modifier)
{
   /* Insert music_event pointer */
   MUSIC_EVENT_P *evp = malloc(sizeof(MUSIC_EVENT_P));
   evp->previous = in->first_event->previous;
   if (in->first_event->previous)
     in->first_event->previous->next = evp;
   evp->next = in->first_event;
   in->first_event->previous = evp;
   in->first_event = evp;
   evp->ev = malloc(sizeof(MUSIC_EVENT));   
   switch(modifier)
     {
      case TRIPLET:
        evp->ev->type = TRIPLET_BEGIN;
        break;
      case DUIN:
        evp->ev->type = DUIN_BEGIN;
        break;
      default:
        break;
     }
}
void inline insert_final_modifier(MUSIC_INTERVAL *in, unsigned char modifier)
{
   /* Insert music_event pointer */
   MUSIC_EVENT_P *evp = malloc(sizeof(MUSIC_EVENT_P));
   evp->next = in->last_event->next;
   if (evp->next)
     evp->next->previous = evp;
   evp->previous = in->last_event;
   in->last_event->next = evp;
   in->last_event = evp;
   evp->ev = malloc(sizeof(MUSIC_EVENT));   
   switch(modifier)
     {
      case TRIPLET:
        evp->ev->type = TRIPLET_END;
        break;
      case DUIN:
        evp->ev->type = DUIN_END;
        break;
      default:
        break;
     }
}
unsigned long inline quant_err(
     unsigned long *t1, unsigned long *t2, 
     unsigned short n)  /* time 0 not included */
{   
   unsigned long q_err = 0;
   int i;
   /* Compute quantization error */
   for (i = 0; i < n; i++)
     q_err += abs_delta(t1[i], t2[i]);
   return q_err;
}
void inline quantize_first_note(MUSIC_EVENT_P *evp)
{  
   MUSIC_EVENT *ev;
   unsigned long ndur, totdur;
   double percent_note;
   ev = evp->ev;
   if(ev->note_duration == 0)
     {
#if 1 /* This is for debugging degenerate note events */
	if (ev->type != ME_REST)
	  {	     
	     fprintf(stderr, "Warning zero duration note is not a rest\n");
	     ev->type = ME_REST;
	     ev->note = 0;
	     ev-> velocity = 0;
	  }
#endif	
     return;
     }   
   ndur = ev->note_duration;
   totdur = ev->total_duration;
   percent_note = (double)ev->note_duration / (double)ev->total_duration;
   if (percent_note >= 0.5)
        ev->note_duration = ev->total_duration;
   if (totdur - ndur < SLUR_TOL && !(ev->flags & ME_NO_SLUR) 
       && !(ev->flags & ME_TIED))
     ev->flags |= ME_SLURRED;
   else if (percent_note >= 0.7)
     ; /* normal unslurred note */
   else if (percent_note >= 0.5)
     ev->flags |= ME_STACCATO;     
   else
     {
        ndur = ev->total_duration / 2;
        while(1)
          {
             percent_note = (double)ev->note_duration / (double)ndur;
             if (percent_note > 0.5 && percent_note <= 1.)
               break;
             ndur /= 2;
          }   /* Incomplete notes can be passed to output that has to identify
	       the rest and print it */
        ev->note_duration = ndur;
     }
}
/* Split a music interval with a special division pattern like triplet
 * or duin, splitting at time 0 is not performed */
void split_sec(MUSIC_INTERVAL *in, unsigned long *times, 
               unsigned char modifier)
{
   MUSIC_EVENT_P *ev;
   double conv;
   unsigned long td, old_spt;
   int i;
   i = 0;
   ev = in->first_event;
   old_spt = 0;
   /* Quantize attack times to split times */
   while(1)
     {
        td = ev->ev->total_duration;
        ev->ev->total_duration = times[i] - old_spt;
        old_spt = times[i++];
        conv = ev->ev->total_duration / td;
        if (ev->ev->note_duration)
          ev->ev->note_duration *= conv;
        if (ev->ev->note == ev->next->ev->note)
          ev->ev->flags |= ME_NO_SLUR;
        ev = ev->next;
        if (ev == in->last_event)
          break;
     }  /* FIXME if note equal, NOSLUR */
   /* Quantize last event's duration */
   td = ev->ev->total_duration;
   ev->ev->total_duration = in->total_duration - times[i - 1];
   conv = ev->ev->total_duration / td;
   if (ev->ev->note_duration)
     ev->ev->note_duration *= conv;
   ev = in->first_event;
   /* Multiply durations times correction factor */
   while(1)
     {
        correct(ev->ev, modifier);
        if (ev == in->last_event)
          break;
        ev = ev->next;
     }
#if 0 /* Must decide what to do with this problem */
      /* It should be used only if tied and is it really necessary */
      /* Neglect it for the time being */
   if (in->first_event->previous)  /* FIXME must be restore after */
     correct(in->first_event->previous->ev, modifier);
#endif   
   /* Quantize note durations and set flags */
   ev = in->first_event;
#if 0   
   /* First event takes previous event into account (only if tied) */
   if (in->first_event->previous)
     quantize_first_note(in->first_event);
   else
#endif     
     if (ev->ev->type == ME_NOTE)
       quantize_note_duration(ev->ev);
   while(ev != in->last_event)
     {
        ev = ev->next;
	if (ev->ev->type == ME_NOTE)
	  quantize_note_duration(ev->ev);
     }   
   insert_initial_modifier(in, modifier);
   insert_final_modifier(in, modifier);
}
void quantize_note_duration(MUSIC_EVENT *me)
{
   unsigned long ndur;
   double percent_note = (double)me->note_duration /
     (double)me->total_duration;
   if (me->note_duration == 0)
     {	
	me->type = ME_REST;  /* FIXME should never happen */
	return;
     }   
   if (percent_note >= 0.5)
        me->note_duration = me->total_duration;
#if 0   
   if (percent_note >= 0.9 && !(me->flags & ME_NO_SLUR))
     me->flags |= ME_SLURRED;
#endif   
   if (me->total_duration - me->note_duration < SLUR_TOL && 
       !(me->flags & ME_NO_SLUR))
     me->flags |= ME_SLURRED;
   else if (percent_note >= 0.7)
     ;
   else if (percent_note >= 0.5)
     me->flags |= ME_STACCATO;
   else
     {
        ndur = me->total_duration / 2;
        while(1)
          {
	     if (ndur == 0)
	       abort();
             percent_note = (double)me->note_duration / (double)ndur;
             if (percent_note > 0.5 && percent_note <= 1.)
               break;
             ndur /= 2;
          }
        me->note_duration = ndur;
     }
}
/* Return number of attack times; neglect attack at time 0 if present */
unsigned char inline numatt(MUSIC_INTERVAL *in)
{
   MUSIC_EVENT_P *ev = in->first_event;
   unsigned char numatt = 0;
   while(1)
     {
        if (ev != in->first_event && ev->ev->type == ME_NOTE)
          numatt++;         
        if(ev == in->last_event)
          break;
        ev = ev->next;
     }
   return numatt;
}
int quantize_interval(MUSIC_INTERVAL *in)
{
   int natt, i, ierr_min;
   unsigned long *att_times, qerr_min;
   unsigned long split_time0, split_time;       
   DIVISION_TABLE *dtp;
/* Split measure according to interval duration */
/* Compute number of attacks */
   natt = numatt(in);
   if (natt == 0)
     {
        quantize_first_note(in->first_event);
        return RC_SUCCESS;
     }
   att_times = malloc(natt * sizeof(unsigned long));
/* Get attack_times */   
   get_attack_times(att_times, in);
/* Get pointer to secondary division table for this time signature */
   dtp = in_sec_div_tbl(in);
   if (dtp)
     {  
/* Compute quantization error for each possible secondary division */   
        qerr_min = 100000000L;
        ierr_min = -1;
        for (i = 0; i < dtp->ndiv; i++)
          {
             unsigned long qerr;
             if (natt == dtp->natt[i])
               {             
                  qerr = quant_err(dtp->times[i], att_times, natt);
                  if (qerr < qerr_min)
                    {
                       ierr_min = i;
                       qerr_min = qerr;
                    }
               }
          }
     }
   free(att_times);
/* If best match with a secondary division is acceptable */
/* Quantize at these times and quit analysing measure */
   if (ierr_min != -1 && qerr_min < Q_TOL)
     {
     split_sec(in, dtp->times[ierr_min], dtp->modifier);
     }   
   else /* Split (recursively) at primary division */
     {
        /* Get the primary division times. Usually one can split
         * a measure either in 2 or in 3 */
        int n = in_pri_div(in);
        MUSIC_INTERVAL ** ins = malloc(n * sizeof(MUSIC_INTERVAL *));
        MUSIC_INTERVAL *to_be_split = malloc(sizeof(MUSIC_INTERVAL));
        MUSIC_INTERVAL *tmp_in = malloc(sizeof(MUSIC_INTERVAL));
        for (i = 0; i < n; i++)
          ins[i] = malloc(sizeof(MUSIC_INTERVAL));
        split_time0 = in->total_duration / n;
        split_time = split_time0;
        *to_be_split = *in;
        for (i = 0; i < n - 1; i++)
          {
             split_pri(ins[i], tmp_in, to_be_split, split_time);
             *to_be_split = *tmp_in;
          }
        *ins[i] = *tmp_in;
        for (i = 0; i < n; i++)
          {
#if DEBUG	     
	     printf("In quantize_interval()\n");
	     printf("Quantizing interval %d\n", i);
	     fflush(stdin);
	     dump_interval(ins[i]);
	     fflush(stdin);
#endif	     
             quantize_interval(ins[i]);
#if DEBUG	     
	     printf("Quantized interval %d\n", i);
	     fflush(stdin);
	     dump_interval(ins[i]);
	     fflush(stdin);
#endif	     
          }
	in->first_event = ins[0]->first_event;	
	in->last_event = ins[n - 1]->last_event;
        for (i = 0; i < n; i++)
          free(ins[i]);
        free(to_be_split);
        free(tmp_in);
        free(ins);
     }
   return RC_SUCCESS;
}
#if DEBUG
void dump_event(MUSIC_EVENT *ev)
{
   printf("type = %d\n", ev->type);
   printf("note = %d\n", ev->note);
   printf("tot_duration = %ld\n", ev->total_duration);
   printf("note_duration = %ld\n", ev->note_duration);
   printf("flags = %d\n", ev->flags);
   printf("-------------\n");
}	 
void dump_interval(MUSIC_INTERVAL *in)
{
   MUSIC_EVENT_P *ev = in->first_event;
   while(1)
     {
	dump_event(ev->ev);
	if (ev == in->last_event)
	  break;
	ev = ev->next;
	if (!ev)
	  abort();
	if (!ev->ev)
	  abort();
     }
   printf("\n");
}
#endif
/* Assume mea->first_measure_event->previous = NULL */
int quantize_measure(MEASURE *mea)
{
   int natt, i, ierr_min;
   unsigned long *att_times, qerr_min;
   unsigned long split_time;       
   DIVISION_TABLE *dtp;
   MUSIC_EVENT_P * ev;
   MUSIC_INTERVAL *in;
#if DEBUG   
   printf("Quantizing measure %d\n", mea->no);
   fflush(stdin);
#endif
   assert(mea->first_measure_event->previous == NULL);
   in = malloc(sizeof(MUSIC_INTERVAL));
/* Copy measure to music interval */
   in->first_event = mea->first_measure_event;
   in->last_event = mea->last_measure_event;
   in->total_duration = mea->duration;
/* Pre-pend previous music event if available */
   assert(in->first_event->previous == NULL);
   if (mea->previous_measure_event)
     {  
        ev = malloc(sizeof(MUSIC_EVENT_P));
        ev->previous = NULL;
        ev->next = in->first_event;
        ev->ev = mea->previous_measure_event;
        in->first_event->previous = ev;
     }
   else
      in->first_event->previous = NULL;
/* Split measure according to time signature */
/* Compute number of attacks */
#if DEBUG   
   dump_interval(in);
   fflush(stdin);
#endif   
   natt = numatt(in);
   if (natt == 0)
     {
        quantize_first_note(in->first_event);
	if (in->first_event->previous)
	  {
	     free(in->first_event->previous);
	     in->first_event->previous = NULL;
	  }	
	free(in);
        return RC_SUCCESS;
     }
   att_times = malloc(natt * sizeof(unsigned long));
/* Get attack_times */   
   get_attack_times(att_times, in);
/* Get pointer to secondary division table for this time signature */
   dtp = sec_div_tbl(mea);
   if(dtp) /* Is a secondary division table available for this time sig. ?*/
     {	
/* Compute quantization error for each possible secondary division */   
	qerr_min = QUARTER * 1000;
	ierr_min = -1;
	for (i = 0; i < dtp->ndiv; i++)
	  {
	     unsigned long qerr;
	     if (natt == dtp->natt[i])
	       {          
		  qerr = quant_err(dtp->times[i], att_times, natt);
		  if (qerr < qerr_min)
		    {
		       ierr_min = i;
		       qerr_min = qerr;
		    }
	       }
	  }
	free(att_times);
	/* If best match with a secondary division is acceptable */
	/* Quantize at these times and quit analysing measure */
     }
   if (dtp && ierr_min != -1 && qerr_min < Q_TOL)
	  split_sec(in, dtp->times[ierr_min], dtp->modifier);
   else /* Split (recursively) at primary division */
     {
	/* Get the primary division times. Usually one can split
	 * a measure either in 2 or in 3 */
	int n = pri_div(mea);
	MUSIC_INTERVAL ** ins = malloc(n * sizeof(MUSIC_INTERVAL *));
	MUSIC_INTERVAL *to_be_split = malloc(sizeof(MUSIC_INTERVAL));
	MUSIC_INTERVAL *tmp_in = malloc(sizeof(MUSIC_INTERVAL));
	for (i = 0; i < n; i++)
          ins[i] = malloc(sizeof(MUSIC_INTERVAL));
        split_time = in->total_duration / n;
        *to_be_split = *in;
        for (i = 0; i < n - 1; i++)
          {
             split_pri(ins[i], tmp_in, to_be_split, split_time);
#if DEBUG	     
	     printf ("ins[%d] :\n", i);
	     dump_interval(ins[i]);
	     printf ("tmp_in :\n");
	     dump_interval(tmp_in);
	     fflush(stdin);
#endif	     
             *to_be_split = *tmp_in;
          }
        *ins[i] = *tmp_in;
        for (i = 0; i < n; i++)
          {
#if DEBUG	     
	     printf("521: Quantizing interval %d\n", i);
	     dump_interval(ins[i]);
	     fflush(stdin);
#endif	     
             quantize_interval(ins[i]);
#if DEBUG	     
	     printf("526: Quantized interval %d\n", i);
	     dump_interval(ins[i]);
	     fflush(stdin);
#endif	     
          }
	in->last_event = ins[n - 1]->last_event;
	in->first_event = ins[0]->first_event;
        for (i = 0; i < n; i++)
          free(ins[i]);
        free(to_be_split);
        free(tmp_in);
        free(ins);
     }
#if DEBUG   
   printf("Quantized measure %d\n", mea->no);
   dump_interval(in);
   fflush(stdin);
#endif
   mea->first_measure_event = in->first_event;
   mea->last_measure_event = in->last_event;
   if (in->first_event->previous)
     {	
	free(in->first_event->previous);
	in->first_event->previous = NULL;
     }   
   free(in);
   return RC_SUCCESS;
}
void inline split_note(MUSIC_EVENT_P *evp, unsigned long split_time)
{   
   unsigned long delta;
   MUSIC_EVENT_P *mev;
   mev = malloc(sizeof(MUSIC_EVENT_P));
   mev->ev = malloc(sizeof(MUSIC_EVENT));   
   delta = evp->ev->note_duration - split_time;
   if (delta < Q_TOL)
     {  
        evp->ev->note_duration = split_time;
        mev->ev->type = ME_REST;
        mev->ev->note = 0;
        mev->ev->note_duration = 0;
	mev->ev->velocity = 0;
        mev->ev->total_duration = evp->ev->total_duration - split_time;
        mev->ev->flags = 0;
        evp->ev->total_duration = split_time;
        evp->ev->flags = 0;
        mev->next = evp->next;
        mev->previous = evp;
	if (evp->next)
	  evp->next->previous = mev;
        evp->next = mev;
     }
   else
     {
        mev->ev->note_duration = evp->ev->note_duration - split_time;
        evp->ev->note_duration = split_time;
        mev->ev->type = ME_NOTE;
        mev->ev->note = evp->ev->note;
	mev->ev->velocity = evp->ev->velocity;
        mev->ev->total_duration = evp->ev->total_duration - split_time;
        mev->ev->flags = 0;
        evp->ev->total_duration = split_time;
        evp->ev->flags = ME_TIED;
        mev->next = evp->next;
        mev->previous = evp;
	if (evp->next)
	  evp->next->previous = mev;
        evp->next = mev;
     }
}  
void inline split_rest(MUSIC_EVENT_P *evp, unsigned long split_time)
{   
   MUSIC_EVENT_P *mev;
   mev = malloc(sizeof(MUSIC_EVENT_P)); 
   mev->ev = malloc(sizeof(MUSIC_EVENT));  
   mev->ev->type = ME_REST;
   mev->ev->note = 0;
   mev->ev->note_duration = 0;
   mev->ev->velocity = 0;
   mev->ev->total_duration = evp->ev->total_duration - split_time;
   mev->ev->flags = 0;
   evp->ev->total_duration = split_time;
   evp->ev->flags = ME_NO_SLUR;
   mev->next = evp->next;
   mev->previous = evp;
   if (evp->next)
     evp->next->previous = mev;
   evp->next = mev;
}  
void split_pri(MUSIC_INTERVAL *i0, MUSIC_INTERVAL *i1, MUSIC_INTERVAL *i,
               unsigned long split_time)
{
   MUSIC_EVENT_P *evp;
   unsigned long curtime = 0;
   long delta;
   evp = i->first_event;
   i0->first_event = evp;
   if (i0->first_event)
   while (1)
     {
        curtime += evp->ev->total_duration;
        if (curtime >= split_time)
          break;
	if (evp == i->last_event)
	  {
	     fprintf (stdin, "Too short an event to be split\n");
	     abort();
	  }	
        evp=evp->next;
     }
   delta = curtime - split_time;
   if (delta <= Q_TOL)
     { /* evp ends a little bit after split time */
        evp->ev->total_duration -= delta;
        if (evp->ev->note_duration > evp->ev->total_duration)
          evp->ev->note_duration = evp->ev->total_duration;
        evp->next->ev->total_duration += delta;
        evp->next->ev->note_duration += delta;
        curtime = split_time;
     }
   /*   delta = split_time - (curtime - evp->ev->total_duration); */
   delta = split_time + evp->ev->total_duration - curtime;
   /* delta is how much the beginning of evp is behind split time */
   assert(delta >= 0);
   if (delta <= Q_TOL)
     {
        evp = evp->previous;
        evp->ev->total_duration += delta;
        evp->next->ev->total_duration -= delta;
        evp->next->ev->note_duration -= delta;
        curtime = split_time;
     }
   if (curtime == split_time)  /* No event splitting necessary */
     {
        i0->last_event = evp;
        i0->total_duration = curtime;
        i1->first_event = evp->next;
	i1->last_event = i->last_event;
        i1->total_duration = i->total_duration - curtime;
     }
   else /* curtime > split_time */
     {
        unsigned long event_begin = curtime - evp->ev->total_duration;
        if (event_begin + evp->ev->note_duration >
            split_time)
	  {
          split_note(evp, split_time - event_begin);
	  }	
        else
	  {	     
	     split_rest(evp, split_time - event_begin);
	  }	
        i0->last_event = evp;
        i0->total_duration = split_time;
        i1->first_event = evp->next;
        if (i->last_event == evp)
          i->last_event = evp->next;
        i1->last_event = i->last_event;
        i1->total_duration = i->total_duration - split_time;
     }
        if (i0->last_event->ev->note  == i1->first_event->ev->note
            && !(i0->last_event->ev->flags & ME_TIED))
          i0->last_event->ev->flags |= ME_NO_SLUR;
}                  
int translate_track(GEN_INFO *g)
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
		 LY_PRINTER *lyp;
                 fprintf (gen->fout, "%% Decoding Track %d\n", g->active_track);
		 lyp = lyp_ist(gen->fout);
                 measure_no = 0; /* Internal nubering base 0 */
		 update_directives(g, measure_no);
                 meas = meas_scanner_ist(scn, g);
		 init_lily_out_note(g);
                 while (1)
                   {
                      forget_notes();
                      rc = get_measure(meas, &mea);
                      if (rc)
                        break;
                      mea.no = measure_no++;
		      if (meas->first_queue_event)
                      rc = quantize_measure(&mea);
		      if (meas->first_queue_event)
                      output_measure(&mea, lyp);
                      clear_measure(&mea);
		      swap_note_buffers();
		      if(measure_duration_changed(g, measure_no))
			flush_multimeasure_rests(lyp, mea.no, mea.duration);
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
