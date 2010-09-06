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
DIRECTIVE directives[MAX_DIRECTIVES]; /* FIXME should be variable allocation */
static const char delimiters[] = " \t\n";
static int isnumber(char *s)
{
  while(*s)
    if (!isdigit(*s++))
      return 0;
  return 1;
}
static int do_measure_duration(GEN_INFO *g)
{
  int rc;
  char *token;
  token = strtok(NULL, delimiters);
  if (token != NULL)
    {
      directives[g->nmd].cmd = MEASURE_DURATION;
      rc = RC_SUCCESS;
      if (strcmp (token, "4/4") == 0)
        {
          directives[g->nmd].data.time_sig.num = 4;
          directives[g->nmd].data.time_sig.den = 4;
        }
      else if (strcmp (token ,"3/4") == 0)
        {
          directives[g->nmd].data.time_sig.num = 3;
          directives[g->nmd].data.time_sig.den = 4;
        }
      else if (strcmp (token ,"2/4") == 0)
        {
          directives[g->nmd].data.time_sig.num = 2;
          directives[g->nmd].data.time_sig.den = 4;
        }
      else if (strcmp (token ,"2/2") == 0)
        {
          directives[g->nmd].data.time_sig.num = 2;
          directives[g->nmd].data.time_sig.den = 2;
        }
      else if (strcmp (token ,"3/2") == 0)
        {
          directives[g->nmd].data.time_sig.num = 3;
          directives[g->nmd].data.time_sig.den = 2;
        }
      else if (strcmp (token ,"2/8") == 0)
        {
          directives[g->nmd].data.time_sig.num = 2;
          directives[g->nmd].data.time_sig.den = 8;
        }
      else if (strcmp (token ,"3/8") == 0)
        {
          directives[g->nmd].data.time_sig.num = 3;
          directives[g->nmd].data.time_sig.den = 8;
        }
      else if (strcmp (token ,"6/8") == 0)
        {
          directives[g->nmd].data.time_sig.num = 6;
          directives[g->nmd].data.time_sig.den = 8;
        }
      else
	 {
	    fprintf(stderr, 
	      "Unknown time signature in MEASURE_DURATION directive\n");
	    abort();
	 }       
    }
  else
    rc = RC_FAIL;
  return rc;
}
static int do_numbered_directive(GEN_INFO *g, char * token)
{  
  int rc, n;
  sscanf(token, "%d", &n);
  rc = RC_SUCCESS;
  if (n > 0)
    {         
      directives[g->nmd].measure = n - 1;
                                    /* Internal measur enumbering is base 0 */
                                    /* External is base 1 */
      token = strtok(NULL, delimiters);
      if (token != NULL)
        {
          if (strcmp (token, "MEASURE_DURATION") == 0)
            rc = do_measure_duration(g);
          else if (strcmp (token, "KEY_SIGNATURE") == 0)
            {                        
              directives[g->nmd].cmd = KEY_SIGN;
              token = strtok(NULL, delimiters);
              sscanf(token, "%d", &n);
              if (n <= 7 && n >= -6)
                directives[g->nmd].data.sharps = n;
              else
                {            
                  rc = RC_FAIL;
                  fprintf (stderr, "%d is not a valid key\n", n);
                }             
            }
          else if (strcmp (token, "STRETCH") == 0)
            {
	      double f;
              directives[g->nmd].cmd = STRETCH;
              token = strtok(NULL, delimiters);
              sscanf(token, "%lf", &f);
              if (f > 0.)
		 {		    
		    directives[g->nmd].data.stretch_factor = f;
		    g->str->nstr++;
		 }	       
              else
                {            
                  rc = RC_FAIL;
                  fprintf (stderr, "%lf is not a valid stretch factor\n", f);
                }             
            }
          else
            rc = RC_FAIL;
        }
      else
        rc = RC_FAIL;
    }
  else
    rc = RC_FAIL;
  return rc;
}
static inline int do_initial_skip(GEN_INFO * gen)
{
  char *token;
  int rc;
  double skip;
  rc = RC_SUCCESS;
  token = strtok(NULL, delimiters);
  if (token != NULL)
    {
      sscanf(token, "%lf", &skip);
      if (skip > 0)
        gen->initial_skip = skip * QUARTER;
      else
        rc = RC_FAIL;
    }
  else
    rc = RC_FAIL;
  return rc;
}
int parsecmd(GEN_INFO *g)
{
  char *token, *cp, *buf;
  size_t nbuf;
  int rc, lineno, i, istr;
  unsigned long old_measure_duration, cur_time;
  unsigned short old_measure;
  if (g->fcmd)
     {   	
	buf = malloc(81 * sizeof(char));
	nbuf = 81;
	lineno = 0;
	g->nmd = 0;
	g->str->nstr = 0;
	g->initial_skip = 0;
	while (1)
	  {
	     char *rcc;
	     rcc = fgets(buf, nbuf, g->fcmd);
	     if (rcc == NULL)
	       {
		  rc = RC_EOF;
		  break;
	       }      
	     lineno++;
	     if (buf[0] == '#') /* Is it a comment? */
	       continue;
	     cp = strdup(buf);
	     token = strtok(cp, delimiters);
	     if (strcmp (token, "INITIAL_SKIP") == 0)
	       {          
		  if ((rc = do_initial_skip(g)) == RC_FAIL)
		    break;
	       }       
	     else 
	       {
		  rc = isnumber(token);
		  if (rc)
		    {
		       if((rc = do_numbered_directive(g, token)) == RC_FAIL)
			 break;
		       g->nmd++;
		       if (g->nmd >= MAX_DIRECTIVES)
			 {
			    fprintf(stderr, "Maximum number of directives exceeded\n");
			    abort();
			 }      
		    }      
	       }  
	  }   
	if (rc == RC_FAIL)
	  fprintf (stderr, "Syntax error in directive file at line %d \n %s\n", 
		   lineno, buf);
	/* Build two arrays of stretched and unstretched times */
	old_measure_duration = FULL;
	old_measure = 0;
	cur_time = 0;
	istr = 0;
	g->str->nstr++; /* allocate also implicit time 0 factor 1 point */
	/* FIXME assert Sort according to measure ? */
	g->str->factors = malloc((g->str->nstr) * sizeof(double));
	g->str->stretched = malloc((g->str->nstr) * sizeof(unsigned long));
	g->str->unstretched = malloc((g->str->nstr) * sizeof(unsigned long));
	g->str->factors[istr] = 1.;
	g->str->stretched[istr] = g->str->unstretched[istr] = 0;
	istr++;
	for (i = 0; i < g->nmd; i++)
	  {
	     if (directives[i].cmd == MEASURE_DURATION)
	       {	     
		  cur_time += old_measure_duration * 
		    (directives[i].measure - old_measure);
		  old_measure = directives[i].measure;
		  old_measure_duration = FULL *
		    directives[i].data.time_sig.num /
		    directives[i].data.time_sig.den;
	       }
	     else if (directives[i].cmd == STRETCH)
	       {
		  cur_time += old_measure_duration * 
		    (directives[i].measure - old_measure);
		  old_measure = directives[i].measure;
		  g->str->stretched[istr] = cur_time;
		  g->str->unstretched[istr] = 
		    (double)g->str->unstretched[istr - 1] + 
		    (double)(cur_time - g->str->stretched[istr - 1]) / 
		    g->str->factors[istr - 1] + 0.5;
		  g->str->factors[istr] = directives[i].data.stretch_factor;
		  istr++;
	       }	
	  }
#if DEBUG   
	printf("istr = %d, g->str->nstr = %d\n", istr, g->str->nstr);
	for (i = 0 ; i < istr; i++)
	  printf("%ld %ld %lf\n", 
		 g->str->unstretched[i], 
		 g->str->stretched[i], 
		 g->str->factors[i]);
#endif   
	free (buf);
	free (cp);
	return rc;
     }
   else
     {
	g->str->nstr = 1; /* allocate implicit time 0 factor 1 point */
	g->str->factors = malloc(sizeof(double));
	g->str->stretched = malloc(sizeof(unsigned long));
	g->str->unstretched = malloc(sizeof(unsigned long));
	g->str->factors[0] = 1.;
	g->str->stretched[0] = g->str->unstretched[0] = 0;
#if DEBUG   
	printf("g->str->nstr = %d\n", g->str->nstr);
	for (i = 0 ; i < g->str->nstr; i++)
	  printf("%ld %ld %lf\n", 
		 g->str->unstretched[i], 
		 g->str->stretched[i], 
		 g->str->factors[i]);
#endif   
     }
return RC_SUCCESS;   
}

          
      
      
