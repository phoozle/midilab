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



#define MAIN
#include "mi2ly.h"
void inline close_files (GEN_INFO *gen)
{
   if (gen->fmid)
     fclose(gen->fmid);
   if (gen->flog)
     fclose(gen->flog);
   if (gen->fcmd)
     fclose(gen->fcmd);
   if (gen->fout)
     fclose(gen->fout);
}
int main(int argc, char**argv)
{
   int rc;
   GEN_INFO *gen;
   gen = gen_info_ist(); /* Instance of general information structure
                          * set everything to default values */
   rc = iofiles(argc, argv, gen);
   gen->time_num = 4;
   gen->time_den = 4;
           /* FIXME better define it in iofiles 
	    default value modified by update_directives */
   if (rc != RC_SUCCESS)
     return RC_FAIL;
   rc = parsecmd(gen);
   if (rc == RC_FAIL)
     {
	fprintf(stderr, "Parsing directive file unsuccesful\n");
	abort();
     }
   if (gen->dump_mode == -1)
     {  
        fprintf(gen->flog, "\nActive_track = %d\n", gen->active_track);
        fprintf(gen->flog, "\nActive_channel = %d\n", gen->active_channel);
        rc = translate_track(gen);
        if (rc == MATCH)
          fprintf(gen->flog,"\nParsing was successful\n");
        else
          fprintf(gen->flog,"\nParsing was unsuccessful\n");
     }
   else if (gen->dump_mode == 0)
     {
        rc = dump_smf(gen);
        if (rc == MATCH)
          fprintf(gen->flog,"\nParsing was successful\n");
        else
          fprintf(gen->flog,"\nParsing was unsuccessful\n");
     }
   else if (gen->dump_mode == 1)
     { /* FIXME parsecmd() and update_directives() also for dumpX */
        rc = dump_track_1(gen);
        if (rc == MATCH)
          fprintf(gen->flog,"\nParsing was successful\n");
        else
          fprintf(gen->flog,"\nParsing was unsuccessful\n");
     }
   else if (gen->dump_mode == 2)
     {
        rc = dump_track_2(gen); 
        if (rc == MATCH)
          fprintf(gen->flog,"\nParsing was successful\n");
        else
          fprintf(gen->flog,"\nParsing was unsuccessful\n");
     }
   else if (gen->dump_mode == 3)
     {
        rc = dump_track_3(gen); 
        if (rc == MATCH)
          fprintf(gen->flog,"\nParsing was successful\n");
        else
          fprintf(gen->flog,"\nParsing was unsuccessful\n");
     }
   else if (gen->dump_mode == 4 || gen->dump_mode == 5)
     {
        rc = dump_track_4_5(gen); 
        if (rc == MATCH)
          fprintf(gen->flog,"\nParsing was successful\n");
        else
          fprintf(gen->flog,"\nParsing was unsuccessful\n");
     }
   else;
   close_files(gen);
   free(gen);
   return 0;
}
