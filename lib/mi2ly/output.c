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
LY_PRINTER *lyp_ist(FILE *fmly)
{
   LY_PRINTER * lyp;
   lyp = malloc(sizeof(LY_PRINTER));
   lyp->fmly = fmly;
   lyp->slur_open = 0;
   lyp->multimeasure_rests = 1;
   lyp->n_multi = 0;
   return lyp;
}
/* Output rests to fill dur shorter rests come first */
void output_rest_to_strong(unsigned long dur, FILE * fmly)
{
   unsigned char r_digit;
   dur /= (QUARTER / 16);  /* Rests to output are now bits of dur right to left */
   r_digit = 64;
   while(1)
     {
	if (dur & 0x1)
	  fprintf(fmly,"r%d ", r_digit);
	else;
	dur /= 2;
	r_digit /= 2;
	if(!dur)
	  break;
     }
}
void output_note_to_strong(unsigned long dur, unsigned char note, FILE * fmly)
{
   unsigned char n_digit;
   dur /= (QUARTER / 16);  /* Rests to output are now bits of dur right to left */
   n_digit = 64;
   while(1)
     {
	if (dur & 0x1)
	  {
	     lily_out_note (note);
	     fprintf(fmly,"%d", n_digit);
	     if (dur - 0x1)
	       fprintf(fmly, "~");
	  }
	else;
	dur /= 2;
	n_digit /= 2;
	if(!dur)
	  break;
     }
}
void output_rest_after_strong(unsigned long dur, FILE *fmly)
{
   unsigned char first = 1, r_digit, old_r_digit;
   unsigned long d1;
   dur /= (QUARTER / 16);  
            /* Rests to output are now bits of dur right to left */
            /* Find highest order bit */
   d1 = 1;
   while (dur >= d1)
     d1 <<= 1;
   d1 >>= 1;
   r_digit = 64 / d1;
   old_r_digit = 0;
   while (1)
     {
	if (d1 & dur)
	  {	
	     if (r_digit == old_r_digit * 2)
	       fprintf(fmly,".");
	     else
	       {		  
		  if (!first)
		    fprintf(fmly," ");
		  fprintf(fmly,"r%d", r_digit);
	       }
	     if (first)
	       first = 0;
	     dur -= d1;
	     old_r_digit = r_digit;
	  }
	else;
	r_digit *= 2;
	d1 >>= 1;
	if(!dur)
	  {	     
	     fprintf(fmly," ");		  
	     break;
	  }	
     }
}
void output_note_after_strong(unsigned long dur, unsigned char note, 
			      FILE *fmly)
{
   unsigned char first = 1, n_digit, old_n_digit;
   unsigned long d1;
   dur /= (QUARTER / 16);  
              /* Notes to output are now bits of dur right to left */
              /* Find highest order bit */
   if (dur == 0)
     return;
   d1 = 1;
   while (dur >= d1)
     d1 <<= 1;
   d1 >>= 1;
   n_digit = 64 / d1;
   old_n_digit = 0;
   while (1)
     {
	if (d1 & dur)
	  {	     
	     if (n_digit == old_n_digit * 2)
	       fprintf(fmly,".");
	     else
	       {		  
		  if (!first)
		    fprintf(fmly,"~");
		  lily_out_note (note);
		  fprintf(fmly,"%d", n_digit);
	       }
	     if (first)
	       first = 0;
	     dur -= d1;
	     old_n_digit = n_digit;
	  }
	else;
	n_digit *= 2;
	d1 >>= 1;
	if(!dur)
	  break;
     }
}
void flush_multimeasure_rests(LY_PRINTER *lyp, unsigned short no, 
			      unsigned long dur)
{
   if (lyp->n_multi)
     {
	fprintf (lyp->fmly, "%% %d\n", no + 1 - lyp->n_multi);
	fprintf (lyp->fmly, "R%s*%d |\n", 
		 dotted_duration(dur), lyp->n_multi);
	lyp->n_multi = 0;
     }
}
void output_measure(MEASURE *mea, LY_PRINTER *lyp)
{
   MUSIC_EVENT_P *ev;
   unsigned long cur_time = 0, old_cur_time;
   unsigned long strong_dur = strong_duration(mea);
   unsigned char conv_num = 1, conv_den = 1;
   unsigned char inside_triplet_or_duin = 0;
   ev = mea->first_measure_event;
   if (lyp->multimeasure_rests)  
     {      
	if (ev->ev->type == ME_REST && ev == mea->last_measure_event)
	  {	  
	     lyp->n_multi++;
	     return;
	  }      
	else if (lyp->n_multi)
	  {
	     fprintf (lyp->fmly, "%% %d\n", mea->no + 1 - lyp->n_multi);
	     fprintf (lyp->fmly, "R%s*%d |\n", 
		      dotted_duration(mea->duration), lyp->n_multi);
	     lyp->n_multi = 0;
	  }
	else
	  ;
     }  
   fprintf (lyp->fmly, "%% %d\n", mea->no + 1);
   while (1) /* Loop over music events */
     {	
	if (ev->ev->type == TRIPLET_BEGIN)
	  {	     
	     fprintf (lyp->fmly, "\\times 2/3 {");
	     conv_num = 2;
	     conv_den = 3;
	     old_cur_time = cur_time;
	     inside_triplet_or_duin = 1;
	  }	
	else if (ev->ev->type == TRIPLET_END || ev->ev->type == DUIN_END)
	  {	     
	     fprintf (lyp->fmly, "} ");
	     cur_time = old_cur_time + (cur_time - old_cur_time) * 
	       conv_num / conv_den;
	     conv_num = 1;
	     conv_den = 1;
	     inside_triplet_or_duin = 0;
	  }	
	else if (ev->ev->type == DUIN_BEGIN)
	  {	     
	     fprintf (lyp->fmly, "\\times 3/2 {");
	     conv_num = 3;
	     conv_den = 2;
	     old_cur_time = cur_time;
	     inside_triplet_or_duin = 1;
	  }	
	else if (ev->ev->type == ME_REST)
	  {
	     unsigned long rest_dur = ev->ev->total_duration, 
	       time_after_strong,
	       time_to_strong;
	     /* determine total duration of rest */
	     while(1)
	       {
		  if (ev == mea->last_measure_event || 
		      ev->next->ev->type != ME_REST)
		    break;
		  ev = ev->next;
		  rest_dur += ev->ev->total_duration;
	       }
	     if (inside_triplet_or_duin)
	       time_after_strong = 0;
	     else
	       time_after_strong = cur_time % strong_dur;
	     /* compute time to reach strong beat */
	     if (time_after_strong)
	       {	     
		  /* output rest up to strong beat */
		  time_to_strong =  strong_dur - time_after_strong;
		  if (rest_dur >= time_to_strong)
		    {	     
		       output_rest_to_strong(time_to_strong, lyp->fmly);
		       rest_dur -= time_to_strong;
		       cur_time += time_to_strong;
		       if (rest_dur)
			 {		  
			    /* output rest after strong beat */
			    output_rest_after_strong(rest_dur, lyp->fmly);
			    cur_time += rest_dur;
			 }	     
		    }
		  else
		    {		       
		       output_rest_after_strong(rest_dur, lyp->fmly); /* Same as async */
		       cur_time += rest_dur;
		    }
	       }	
	     else
	       {		  
		  time_to_strong = 0; /* FIXME Useless? */
		  if (rest_dur)
		    {		  
		       /* output rest after strong beat */
		       output_rest_after_strong(rest_dur, lyp->fmly);
		       cur_time += rest_dur;
		    }	     
	       }	     
	  }
	else if (ev->ev->type == ME_NOTE)
	  {
	     unsigned long note_dur = ev->ev->note_duration, 
	       time_after_strong,
	       time_to_strong, tot_dur = ev->ev->total_duration, rest_dur;
	     unsigned char note = ev->ev->note;
	     /* determine total duration of note */
	     while(1)
	       {
		  if (ev == mea->last_measure_event || 
		      ev->next == NULL ||
		      ev->next->ev->type != ME_NOTE ||
		      note != ev->next->ev->note ||
		      !(ev->ev->flags & ME_TIED))
		    break;
		  ev = ev->next;
		  note_dur += ev->ev->note_duration;
		  tot_dur += ev->ev->total_duration;
	       }
	     rest_dur = tot_dur - note_dur;
	     /* compute time to reach strong beat */
	     if (inside_triplet_or_duin)
	       time_after_strong = 0;
	     else
	       time_after_strong = cur_time % strong_dur;
	     if (time_after_strong)
	       {
		  /* output note up to strong beat */
		  time_to_strong = strong_dur - time_after_strong;
		  if (note_dur >= time_to_strong)
		    {	     
		       output_note_to_strong(time_to_strong, note, lyp->fmly);
		       note_dur -= time_to_strong;
		       cur_time += time_to_strong;
		       if (note_dur)
			 {			    
			    /* output note after strong beat */
			    fprintf(lyp->fmly, "~");
			    output_note_after_strong(note_dur, note, lyp->fmly);
			    cur_time += note_dur;
			 }		       
		    }
		  else
		    {		       
		       output_note_after_strong(note_dur, note, lyp->fmly); 
		       /* Same as async */
		       cur_time += note_dur;
		    }		  
	       }	
	     else 
	       {		  
		  time_to_strong = 0; /* FIXME, useless? */
		  if (note_dur)
		    {			  
		       /* output note after strong beat */
		       output_note_after_strong(note_dur, note, lyp->fmly);
		       cur_time += note_dur;
		    }		  
	       }	     
	     if (ev->ev->flags & ME_SLURRED && !lyp->slur_open &&
		 ev->next->ev->type != ME_REST)
	       {	       
		  lyp->slur_open = 1;
		  fprintf(lyp->fmly, "( ");
	       }	
	     else if (!(ev->ev->flags & ME_SLURRED) && lyp->slur_open
		      && !(ev->ev->flags & ME_TIED))
	       {	       
		  lyp->slur_open = 0;
		  fprintf(lyp->fmly, ") ");
	       }	
	     else if (ev->ev->flags & ME_TIED)
	       fprintf(lyp->fmly, "~ ");
	     else
	       fprintf(lyp->fmly, " ");	       
	     if (rest_dur)	
	       {		  
		  /* output rest after strong beat */
		  output_rest_after_strong(rest_dur, lyp->fmly);
		  cur_time += rest_dur;
	       }	     
	  }
	if (ev == mea->last_measure_event)
	  break;
	else
	  {
	     ev = ev->next;
	  }	
     }	   
   fprintf (lyp->fmly, "|\n");
}
