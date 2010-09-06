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
#define inline
#endif
#ifndef defs_h
#define defs_h
#ifndef uchar
#  define uchar unsigned char
#endif
#define ABS(x) ((x) < 0) ? - (x) : (x)
#define END_OF_TRACK 3
#define MATCH 2
#define NO_MATCH -2
#define RC_SUCCESS 0
#define RC_FAIL -1
#define QUEUE_EMPTY -2
#define FIRST_NOTE_INCOMPLETE -3
#define TRACK_BEGIN  4
#endif
