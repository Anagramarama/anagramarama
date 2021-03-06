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

typedef struct Box {
	int x;
	int y;
	int width;
	int height;
} Box;

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

#define SPACE_CHAR '#'
#define ASCII_SPACE 32
#define SPACE_FILLED_STRING "#######"
#define SPACE_FILLED_CHARS {SPACE_CHAR, SPACE_CHAR, SPACE_CHAR, SPACE_CHAR,\
                             SPACE_CHAR, SPACE_CHAR, SPACE_CHAR, 0}
#define AVAILABLE_TIME 300
#define DEFAULT_LOCALE_PATH "i18n/en_GB/"
