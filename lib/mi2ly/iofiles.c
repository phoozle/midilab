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
extern int optind;
extern char *optarg;
char *outfile;
int iofiles(int argc, char **argv, GEN_INFO *g)
{   
   int lmf;
   char optstring[] = "o:d:t:c:", c, *cmdfile, *logfile;
   char *midifile;
   unsigned char got_output_file = 0;
   g->dump_mode = -1; /* this implies convert active track to lilypond */
   g->active_track = 1;  /* FIXME Already initialized in geninfo.c ? */
   g->active_channel = ALL_CHANNELS;
   while(1)
     {
        c = getopt(argc, argv, optstring);
        if (c == -1)
          break;
        switch (c)
          {
           case 0: 
             break;
           case 'o':  /* output file */
             outfile = malloc((strlen(optarg) + 1) * sizeof(char));
             strcpy(outfile, optarg);
             got_output_file = 1;
             break;
           case 'd':  /* dump mode */
             g->dump_mode = atoi(optarg); /* FIXME check range */
             break;
           case 't':  /* track no */
             g->active_track = atoi(optarg);
	     break;
           case 'c':  /* channel no */
             g->active_channel = atoi(optarg);
             break;
          }
     }
   if (optind >= argc)
     return RC_FAIL;
   lmf = strlen(argv[optind]);
   midifile = malloc((lmf + 1) * sizeof(char));
   strcpy(midifile, argv[optind]);
   if (strcmp(&(midifile[lmf - 3]), "mid") != 0)
     { 
        fprintf (stderr,"\n%s doesn't look like a midifile\n", midifile);
        return RC_FAIL;
     }
   if((g->fmid = fopen(midifile,"r+b")) == NULL) 
     { 
        fprintf (stderr,"\nCannot open input file %s\n",argv[1]);
        return RC_FAIL;
     }
   /* Strip .mid */
   midifile[lmf - 3] = 0;
   cmdfile  = malloc((lmf + 1) * sizeof(char));
   strcpy (cmdfile, midifile);
   strcat (cmdfile,"cmd");
   logfile  = malloc((lmf + 1) * sizeof(char));
   g->fcmd = fopen(cmdfile,"r");
   if (g->fcmd == NULL)
     fprintf (stderr,"\nWarning - Cannot open directive file %s\n", cmdfile);
   strcpy (logfile, midifile);
   strcat (logfile,"log");
   g->flog = fopen(logfile,"w");
   if (g->flog == NULL)
     {
        fprintf (stderr,"\nCannot open output file %s\n", logfile);
        goto output_file_error;
     }
   if (!got_output_file)
     {  
        outfile  = malloc((lmf + 1) * sizeof(char));
        if (g->dump_mode == -1)
          {          
             strcpy (outfile, midifile);
             strcat (outfile,"mly");
          }
        else
          {          
             strcpy (outfile, midifile);
             strcat (outfile,"dmp");
          }
     }
   g->fout = NULL;
   if (g->dump_mode == -1)
     {       
        g->fout = fopen(outfile,"w");
        if (g->fout == NULL)
          {
             fprintf (stderr,"\nCannot open output file %s\n", outfile);
             goto output_file_error;
          }
     }
   else
     {       
        g->fout = fopen(outfile,"w");
        if (g->fout == NULL)
          {
             fprintf (stderr,"\nCannot open output file %s\n", outfile);
             goto output_file_error;
          }
     }
   free(cmdfile);
   free(outfile);
   free(logfile);
   free(midifile);
   return RC_SUCCESS;
   output_file_error:
   free(outfile);
   free(logfile);
   /*   midifile_error: */
   free(midifile);
   return RC_FAIL;
}
