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




#ifdef __STRICT_ANSI__
int getopt(int argc, char **argv, char *optstring);
char *strdup(char *);
#endif

void clear_measure(MEASURE *m);
char *dotted_duration(unsigned long measure_duration);
void dump_measure(MEASURE *mea, FILE *fdump);
int dump_smf(GEN_INFO *);
int dump_track(SMF_SCANNER *s, FILE *fdump, unsigned char active_channel);
int dump_track_1(GEN_INFO *g);
int dump_track_2(GEN_INFO *g);
int dump_track_3(GEN_INFO *g);
int dump_track_4_5(GEN_INFO *g);
int fill_queue(MEAS_SCANNER *meas);
void flush_multimeasure_rests(LY_PRINTER *lyp, unsigned short no, 
			      unsigned long dur);
void forget_notes(void);
GEN_INFO *gen_info_ist(void);
int get_event_from_midi(void);
int get_measure(MEAS_SCANNER *meas, MEASURE *m);
int get_music_event(ME_SCANNER *mesc, MUSIC_EVENT *me);
int get_note_event(NOTE_SCANNER *ns, NOTE_EVENT *nev);
int in_pri_div(MUSIC_INTERVAL *in);
DIVISION_TABLE *in_sec_div_tbl(MUSIC_INTERVAL *mea);
DIVISION_TABLE *in_pri_div_tbl(MUSIC_INTERVAL *mea);
void init_lily_out_note(GEN_INFO *g);
int init_next_track(SMF_SCANNER *);
int iofiles(int argc, char **argv, GEN_INFO *gen);
void lily_out_note(unsigned char note_number);
void logmsg(char * str);
LY_PRINTER *lyp_ist(FILE *fmly);
ME_SCANNER *me_scanner_ist(SMF_SCANNER *s, GEN_INFO *g);
void me_scanner_rls(ME_SCANNER *mesc);
MEAS_SCANNER *meas_scanner_ist(SMF_SCANNER *s, GEN_INFO *g);
void meas_scanner_rls(MEAS_SCANNER *meas);
int measure_duration_changed(GEN_INFO *g, unsigned short no);
NOTE_SCANNER * note_scanner_ist(SMF_SCANNER *s, GEN_INFO *g);
void note_scanner_rls(NOTE_SCANNER *ns);
void output_measure(MEASURE *mea, LY_PRINTER *lyp);
int parsecmd(GEN_INFO *g);
int pri_div(MEASURE *in);
DIVISION_TABLE *pri_div_tbl(MEASURE *mea);
void quantize_measure_termination(MEAS_SCANNER *meas);
int quantize_measure(MEASURE *);
int smf_scn_init(SMF_SCANNER *s);
SMF_SCANNER *smf_scn_inst(FILE *f, STRETCH_DATA *str);
void smf_scn_rls(SMF_SCANNER *s);
SMF_EVENT *smf_event(SMF_SCANNER *s);
void smf_event_rls(SMF_EVENT * eve);
DIVISION_TABLE *sec_div_tbl(MEASURE *mea);
int skip_track(SMF_SCANNER *scn);
void inline split_note_across_measure(MEAS_SCANNER *);
void split_pri(MUSIC_INTERVAL *i0, MUSIC_INTERVAL *i1, 
               MUSIC_INTERVAL *i, unsigned long split_time);
void inline  split_rest_across_measure(MEAS_SCANNER *);
unsigned long strong_duration(MEASURE *mea);
void swap_note_buffers();
int termination_case(MEAS_SCANNER *);
int translate_track(GEN_INFO *gen);
void update_directives(GEN_INFO *g, unsigned short no);
void update_meas(MEAS_SCANNER *m, GEN_INFO *g);
