/*
Anagramarama - A word game.  Like anagrams?  You'll love anagramarama!
Copyright (C) 2003  Colm Gallagher

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Contact Details: colm@coralquest.com
		 12 Weston Terrace, West Kilbride, KA23 9JX.  Scotland.
*/

// ASCII OFFSET to convert a number to it's character equivalent
#define NUM_TO_CHAR 48

// pixel locations of boxes
#define SHUFFLEBOX 110
#define ANSWERBOX 245
#define BOX_START 30
#define BOX_LENGTH 644

#define LETTERSTARTPOS 80
#define LETTERWIDTH 80
#define LETTERHEIGHT 65
#define LETTERSPACING 5

// which box are we working with
#define ANSWER 1
#define SHUFFLE 2
#define CONTROLS 3

// enter button size and position
#define ENTERBOXSTARTX 690
#define ENTERBOXLENGTH 40
#define ENTERBOXSTARTY 254
#define ENTERBOXHEIGHT 35

// clear button size and position
#define CLEARBOXSTARTX 690
#define CLEARBOXLENGTH 40
#define CLEARBOXSTARTY 304
#define CLEARBOXHEIGHT 40

// shuffle button size and position
#define SHUFFLEBOXSTARTX 618
#define SHUFFLEBOXLENGTH 66
#define SHUFFLEBOXSTARTY 206
#define SHUFFLEBOXHEIGHT 16

// solve button size and position
#define SOLVEBOXSTARTX 612
#define SOLVEBOXLENGTH 66
#define SOLVEBOXSTARTY 0
#define SOLVEBOXHEIGHT 30

// new button size and position
#define NEWBOXSTARTX 686
#define NEWBOXLENGTH 46
#define NEWBOXSTARTY 0
#define NEWBOXHEIGHT 30

// quit button size and position
#define QUITBOXSTARTX 742
#define QUITBOXLENGTH 58
#define QUITBOXSTARTY 0
#define QUITBOXHEIGHT 30

// define the clock position and character width
#define CLOCK_X 690
#define CLOCK_Y 35
#define CLOCK_WIDTH 18
#define CLOCK_HEIGHT 32

// define the clock position and character width
#define SCORE_X 690
#define SCORE_Y 67
#define SCORE_WIDTH 18
#define SCORE_HEIGHT 32

#define SPACE_CHAR '£'
#define ASCII_SPACE 32

#define AVAILABLE_TIME 300
