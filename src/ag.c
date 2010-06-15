/*
 * Anagramarama - A word game.  Like anagrams?  You'll love anagramarama!
 * Copyright (C) 2003  Colm Gallagher
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Contact Details: colm@coralquest.com
 *  	 12 Weston Terrace, West Kilbride, KA23 9JX.  Scotland.
 */


/*
 * Contributors
 *
 * Colm Gallagher  : Concept and initial programming
 * Alan Grier      : Graphics
 * THomas Plunkett : Audio
 * Shard           : BEOS Port and bugfixes
 * Adolfo          : Bugfix for Linux Red-Hat version 7.3
 * Pat Thoyts      : C code cleanup, memory leak and cpu usage fixes
 *
 *
 * ----------------------------------------------------------------
 * version		who		changes
 * -------------------------------------------------------------------
 * 0.1		Colm		initial Linux & Windows revisions
 * 
 * 0.2		Shard		Bugfix: buffer overrun in clearWord
 *                      function corrupted memory.  Strange
 *			            thing is it crashed BEOS, but not
 *			            Linux or Windows - guess they handle
 *	                    memory differently or BEOS is much
 *			            better at detecting exceptions
 *
 * 0.3		Shard		added BEOS port (new makefile)
 *
 * 0.4		Adolfo		Bugfix: oops!  in the checkGuess
 * 			            function, I tried to initialise
 * 			            test[] using a variable  as if I was
 * 			            using vb6 !  Have changed this to a 
 * 			            static buffer and all is now well.
 * 
 * 0.5		Colm		Added keyboard input
 *
 * -------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "dlb.h"
#include "linked.h"
#include "sprite.h"
#include "ag.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* module level variables for game control */
char shuffle[]  = "£££££££";
char answer[]   = "£££££££";

char* rootWord;
int updateAnswers = 0;
int startNewGame = 0;
int solvePuzzle = 0;
int shuffleRemaining = 0;
int clearGuess = 0;

int gameStart = 0;
int gameTime = 0;
int stopTheClock = 0;

int totalScore = 0;
int score = 0;
int answersSought = 0;
int answersGot = 0;
int gotBigWord = 0;
int bigWordLen = 0;
int updateTheScore = 0;
int gamePaused = 0;
int foundDuplicate = 0;
int quitGame = 0;
int winGame = 0;

int letterSpeed = LETTER_FAST;

/* Graphics cache */
SDL_Surface* letterBank = NULL;
SDL_Surface* smallLetterBank = NULL;
SDL_Surface* numberBank = NULL;
struct sprite* clockSprite = NULL;
struct sprite* scoreSprite = NULL;

/* audio vars */
Uint32 audio_len;
Uint8 *audio_pos;
struct sound {
	char* name;
	Mix_Chunk *audio_chunk;
	struct sound* next;
};
struct sound* soundCache = NULL;




/***********************************************************
synopsis: walk the module level soundCache until the required
	  name is found.  when found, return the audio data
	  if name is not found, return NULL instead.

inputs:   name - the unique id string of the required sound

outputs:  returns a chunk of audio or NULL if not found
***********************************************************/
static Mix_Chunk* 
getSound(const char *name)
{
    struct sound* currentSound = soundCache;

	while (currentSound != NULL) {

		if (!strcmp(currentSound->name, name)) {
			return currentSound->audio_chunk;
		}
		currentSound = currentSound->next;
	}

	return NULL;
}

/***********************************************************
synopsis: push a sound onto the soundCache

inputs:   soundCache - pointer to the head of the soundCache
	  name - unique id string for the sound, this is used
	         to later play the sound
	  filename - the filename of the WAV file

outputs:  n/a
***********************************************************/
static void
pushSound(struct sound **soundCache, const char *name, const char *filename)
{
    struct sound* thisSound = NULL;

	thisSound = malloc(sizeof(struct sound));
	thisSound->name = malloc(sizeof(name)*(strlen(name) + 1));
	strcpy(thisSound->name, name);
	thisSound->next = *soundCache;

	/* Attempt to load a sample */
	thisSound->audio_chunk = Mix_LoadWAV(filename);

	*soundCache = thisSound;
}

/***********************************************************
synopsis: push all the game sounds onto the soundCache
	  linked list.  Not that soundCache is passed into
	  pushSound by reference, so that the head pointer
	  can be updated

inputs:   pointer to the soundCache

outputs:  n/a
***********************************************************/
static void
bufferSounds(struct sound **soundCache)
{
	pushSound(soundCache, "click-answer", "audio/click-answer.wav");
	pushSound(soundCache, "click-shuffle", "audio/click-shuffle.wav");
	pushSound(soundCache, "foundbig", "audio/foundbig.wav");
	pushSound(soundCache, "found", "audio/found.wav");
	pushSound(soundCache, "clear", "audio/clearword.wav");
	pushSound(soundCache, "duplicate", "audio/duplicate.wav");
	pushSound(soundCache, "badword", "audio/badword.wav");
	pushSound(soundCache, "shuffle", "audio/shuffle.wav");
	pushSound(soundCache, "clock-tick", "audio/clock-tick.wav");
}

/***********************************************************
synopsis: free all of the data in the audio buffer
	  the audio buffer is a module variable

inputs:   n/a

outputs:  n/a
***********************************************************/
static void
clearSoundBuffer()
{
    struct sound* currentSound = soundCache, *previousSound = NULL;

	while (currentSound!=NULL) {
		Mix_FreeChunk(currentSound->audio_chunk);
		free(currentSound->name);
		previousSound = currentSound;
		currentSound = currentSound->next;
		free(previousSound);
	}
}

/***********************************************************
synopsis: determine the next blank space in a string 
	  blanks are indicated by pound £ not space
	  
inputs:   pointer the string to check

outputs:  returns position of next blank (1 is first character)
	  or 0 if no blanks found
***********************************************************/
static int
nextBlank(const char *string)
{
    const char *p = strchr(string, SPACE_CHAR);
    if (p)
        return 1 + (p - string);
    return 0;
}


/***********************************************************
synopsis: shift a string of characters 1 character to the left
          truncating the leftmost character

inputs:   pointer to string to shift

outputs:  pointer to the shifted string
***********************************************************/
static char *
shiftLeftKill(const char *string)
{
    return strdup(string + 1);
}

/***********************************************************
synopsis: shift a string of characters 1 character to the left
	  move the first character to the end of the string
	  so it wraps around

inputs:   pointer to string to shift

outputs:  pointer to the shifted string
***********************************************************/
static char *
shiftLeft(char *string)
{
    char c = *string;
    char *p = string, *q = string+1;
    for (; p && *p && q && *q; ) {
        *p++ = *q++;
    }
    *p = c;
    return string;
}

/***********************************************************
synopsis: Generate all possible combinations of the root word
	  the initial letter is fixed, so to work out all
	  anagrams of a word, prefix with space.

inputs:   head - pointer to the answers list
	  dlbHead - pointer to the dictionary
	  guess - pointer to the current guess
	  remain - pointer to the remaining letters

outputs:  all parameters are in/out
***********************************************************/
static void
ag(struct node** head, struct dlb_node* dlbHead, 
   const char* guess, const char* remain)
{
    char*  newGuess;
    char*  newRemain;
    int    totalLen=0, guessLen=0, remainLen=0;
    
	/* allocate space for our working variables */
	guessLen = strlen(guess);
	remainLen = strlen(remain);
	totalLen = guessLen + remainLen;
    
	newGuess = malloc(sizeof(char) * (totalLen+1));
	newRemain = malloc(sizeof(char) * (totalLen+1));
    
	/* move last remaining letter to end of guess */
	strcpy(newGuess, guess);
	strcpy(newRemain, remain);
	newGuess[guessLen] = newRemain[remainLen-1];
	newGuess[guessLen+1] = '\0';
	newRemain[remainLen-1] = '\0';
    
	if (strlen(newGuess) > 3){
        char *str = shiftLeftKill(newGuess);
        if (dlb_lookup(dlbHead, str)) {
            push(head, str);
        }
        free(str);
	}

	if (strlen(newRemain)) {
        size_t i;
		ag(head, dlbHead, newGuess, newRemain);

		for (i = totalLen-1; i > 0; i--) {
			if (strlen(newRemain) > i){
                newRemain = shiftLeft(newRemain);
                ag(head, dlbHead, newGuess, newRemain);
			}
		}
	}
	/* free the space */
	free(newGuess);
	free(newRemain);
}

/***********************************************************
synopsis: update all of the answers to "found"

inputs:   head - pointer to the answers linked list

outputs:  n/a
***********************************************************/
static void
solveIt(struct node *head)
{
    struct node* current = head;
    
	while (current != NULL){
		current->found = 1;
		current = current->next;
	}
}

/***********************************************************
synopsis: load the named image to position x,y onto the
	  required surface

inputs:  file - the filename to load (.BMP)
	 screen - the SDL_Surface to display the image
	 x,y - the top left postion

outputs:  n/a
***********************************************************/
static void
ShowBMP(const char *file, SDL_Surface *screen, int x, int y)
{
	SDL_Surface *image;
	SDL_Rect dest;

	/* Load the BMP file into a surface */
	image = SDL_LoadBMP(file);
	if ( image == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
		return;
	}

	/* Blit onto the screen surface.
	 *        The surfaces should not be locked at this point.
	 */
	dest.x = x;
	dest.y = y;
	dest.w = 50;
	dest.h = 50;

	SDL_SetColorKey(image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format,255,0,255));

	SDL_BlitSurface(image, NULL, screen, &dest);

	/* Update the changed portion of the screen */
	SDL_FreeSurface(image);
}




/***********************************************************
synopsis: for each letter to each answer in the nodelist,
	  display a neat little box.  If the answer has
	  been found display the letter for each box.
	  if the answer was guessed (as opposed to set found
	  by solveIt), display a white background, otherwise
	  display a blue background.

inputs:   head - pointer to the answers linked list
	  screen - pointer to the SDL_Surface to update

outputs:  n/a
***********************************************************/
static void
displayAnswerBoxes(struct node* head, SDL_Surface* screen)
{
    struct node* current = head;
    SDL_Rect outerrect, innerrect, letterBankRect;
    int i;
    int numWords = 0;
    int acrossOffset = 70;
    int numLetters = 0;
    int listLetters = 0;

	/* width and height are always the same */
	outerrect.w = 16;
	outerrect.h = 16;
	outerrect.x = acrossOffset;
	outerrect.y = 380;

	letterBankRect.w = 10;
	letterBankRect.h = 16;
	letterBankRect.y = 0;
	letterBankRect.x = 0; /* letter is chosen by 10*letter where a is 0 */
    
    while (current != NULL){
        
        /* new word */
		numWords++;
		numLetters =0;
        
		/* update the x for each letter */
		for (i=0;i<current->length;i++) {

			numLetters++;

			SDL_FillRect(screen, &outerrect,0);
			innerrect.w = outerrect.w - 1;
			innerrect.h = outerrect.h - 1;
			innerrect.x = outerrect.x + 1;
			innerrect.y = outerrect.y + 1;

			if (current->guessed){
				SDL_FillRect(screen, &innerrect,SDL_MapRGB(screen->format,255,255,255));
			}
			else{
				SDL_FillRect(screen, &innerrect,SDL_MapRGB(screen->format,217,220,255));
			}

			if (current->found){
				innerrect.x+=2;
				letterBankRect.x = 10 * ((int)current->anagram[i] - 97);
				SDL_SetColorKey(smallLetterBank, SDL_SRCCOLORKEY, SDL_MapRGB(smallLetterBank->format,255,0,255));
				SDL_BlitSurface(smallLetterBank, &letterBankRect, screen, &innerrect);
			}

			outerrect.x += 18;

		}

		if (numLetters > listLetters){
			listLetters = numLetters;
		}

		if (numWords == 11){
			numWords = 0;
			acrossOffset += (listLetters * 18) + 9;
			outerrect.y = 380;
			outerrect.x = acrossOffset;
		}
		else{
			outerrect.x = acrossOffset;
			outerrect.y += 19;
		}

		current=current->next;
	}
}




/***********************************************************
synopsis: walk the linked list of answers checking
	  for our guess.  If the guess exists, mark it as
	  found and guessed.
	  if it's the longest word play the foundbig 
	  sound otherwise play the got word sound.
	  If the word has already been found, play the
	  duplicate sound.
	  If it cannot be found, play the badword sound

inputs:   answer - the string that we're checking
	  head - pointer to the linked list of answers

outputs:  n/a
***********************************************************/
static void
checkGuess(char* answer, struct node* head)
{
    /* check the guess against the answers */
    struct node* current = head;
    int i, len;
    int foundWord = 0;
    char test[7];
    
	len = nextBlank(answer) -1;
	if (len<0) len=7;
	for (i=0; i<len; i++){
		test[i] = answer[i];
	}
	test[len] = '\0';

	while (current != NULL){

		if (!strcmp(current->anagram, test)){
			foundWord = 1;
			if (!current->found){
				score+=current->length;
				totalScore+=current->length;
				answersGot++;
				if (len==bigWordLen){
					gotBigWord = 1;
					Mix_PlayChannel(-1, getSound("foundbig"), 0);
				}
				else{
					/* just a normal word */
					Mix_PlayChannel(-1, getSound("found"),0);
				}
				if (answersSought==answersGot){
					/* getting all answers gives us the game score again!!*/
					totalScore+=score;
					winGame = 1;
				}
				current->found = 1;
				current->guessed=1;
				updateTheScore = 1;

			}
			else{
				foundDuplicate = 1;
				Mix_PlayChannel(-1, getSound("duplicate"),0);
			}
			updateAnswers = 1;
			break;
		}

		current=current->next;
	}

	if (!foundWord){
		Mix_PlayChannel(-1, getSound("badword"),0);
	}
}

/***********************************************************
synopsis: determine the next blank space in a string 
          blanks are indicated by pound £ not space.
	  When a blank is found, move the chosen letter
	  from one box to the other.
	  i.e. If we're using the ANSWER box,
	  move the chosen letter from the SHUFFLE box 
	  to the ANSWER box and move a SPACE back to the
	  SHUFFLE box. and if we're using the SHUFFLE box
	  move the chosen letter from ANSWER to SHUFFLE
	  and move a SPACE into ANSWER.

inputs:   box - the ANSWER or SHUFFLE box
	  *index - pointer to the letter we're interested in

outputs:  retval : the coords of the next blank position
          *index : pointer to the new position were interested in
***********************************************************/
int 
nextBlankPosition(int box, int* index)
{
    int i=0;

	switch(box){
		case ANSWER:
			for(i=0;i<7;i++){
				if (answer[i]==SPACE_CHAR){
					break;
				}
			}
			answer[i] = shuffle[*index];
			shuffle[*index] = SPACE_CHAR;
			break;
		case SHUFFLE:
			for(i=0;i<7;i++){
				if (shuffle[i]==SPACE_CHAR){
					break;
				}
			}
			shuffle[i] = answer[*index];
			answer[*index] = SPACE_CHAR;
			break;
		default:
			break;

	}

	*index = i;

	return i * (GAME_LETTER_WIDTH+GAME_LETTER_SPACE)+BOX_START_X;
}




/***********************************************************
synopsis: handle the keyboard events
	  BACKSPACE & ESCAPE - clear letters
	  RETURN - check guess
	  SPACE - shuffle
	  a-z - select the first instance of that letter
	  	in the shuffle box and move to the answer box

inputs: event - the key that has been pressed
	node - the top of the answers list
	letters - the letter sprites

outputs:  n/a
***********************************************************/
static void
handleKeyboardEvent(SDL_Event *event, struct node* head,
                    struct sprite** letters)
{
    struct sprite* current = *letters;
    char keyedLetter;

	keyedLetter = event->key.keysym.sym;

	if (!gamePaused){

		switch(keyedLetter){

			case SDLK_BACKSPACE: case SDLK_ESCAPE:
				/* clear has been pressed */
				clearGuess = 1;
				break;

			case SDLK_RETURN:
				/* enter has been pressed */
				checkGuess(answer, head);
				break;
			case ' ':
				/* shuffle has been pressed */
				shuffleRemaining = 1;
				Mix_PlayChannel(-1, getSound("shuffle"),0);
				break;
			default:
				/* loop round until we find the first instance of the 
                 * selected letter in SHUFFLE
                 */
				while (current!=NULL&&current->box!=CONTROLS){
					if (current->box == SHUFFLE){
						if (current->letter == keyedLetter){
							current->toX = nextBlankPosition(ANSWER, &current->index);
							current->toY = ANSWER_BOX_Y;
							current->box = ANSWER;
							Mix_PlayChannel(-1, getSound("click-shuffle"), 0);
							break;
						}
					}
					current=current->next;
				}

		}

	}
}




/***********************************************************
synopsis: checks where the mouse click occurred - if it's in
	  a defined hotspot then perform the appropriate action

	  Hotspot	        Action
	  -----------------------------------------------------
	  A letter		set the new x,y of the letter
	                        and play the appropriate sound

	  ClearGuess		set the clearGuess flag

	  checkGuess		pass the current answer to the
	  			checkGuess routine

	  solvePuzzle		set the solvePuzzle flag

	  shuffle		set the shuffle flag and
	  			play the appropriate sound

	  newGame		set the newGame flag

	  quitGame		set the quitGame flag

inputs:  button - mouse button that has ben clicked
         x, y - the x,y coords of the mouse
	 screen - the SDL_Surface to display the image
	 head - pointer to the top of the answers list
	 letters - pointer to the letters sprites

outputs:  n/a
***********************************************************/
static void
clickDetect(int button, int x, int y, SDL_Surface *screen, 
            struct node* head, struct sprite** letters)
{

    struct sprite* current = *letters;

	if (!gamePaused) {

		while (current!=NULL&&current->box!=CONTROLS){
			if (x>= current->x && x<= current->x+current->w && y>= current->y && y<=current->y + current->h){
				if (current->box == SHUFFLE){
					current->toX = nextBlankPosition(ANSWER, &current->index);
					current->toY = ANSWER_BOX_Y;
					current->box = ANSWER;
					Mix_PlayChannel(-1, getSound("click-shuffle"), 0);
				}
				else{
					current->toX = nextBlankPosition(SHUFFLE, &current->index);
					current->toY = SHUFFLE_BOX_Y;
					current->box = SHUFFLE;
					Mix_PlayChannel(-1, getSound("click-answer"), 0);
				}

				break;
			}
			current=current->next;
		}

		if (x > CLEARBOXSTARTX && x < CLEARBOXLENGTH+CLEARBOXSTARTX && y > CLEARBOXSTARTY && y < CLEARBOXSTARTY+CLEARBOXHEIGHT){
			/* clear has been pressed */
			clearGuess = 1;
		}

		/* check the other hotspots */
		if (x > ENTERBOXSTARTX && x < ENTERBOXLENGTH+ENTERBOXSTARTX && y > ENTERBOXSTARTY && y < ENTERBOXSTARTY+ENTERBOXHEIGHT){
			/* enter has been pressed */
			checkGuess(answer, head);
		}

		if (x > SOLVEBOXSTARTX && x < SOLVEBOXLENGTH+SOLVEBOXSTARTX && y > SOLVEBOXSTARTY && y < SOLVEBOXSTARTY+SOLVEBOXHEIGHT){
			/* solve has been pressed */
			solvePuzzle = 1;
		}
		
		if (x > SHUFFLEBOXSTARTX && x < SHUFFLEBOXLENGTH+SHUFFLEBOXSTARTX && y > SHUFFLEBOXSTARTY && y < SHUFFLEBOXSTARTY+SHUFFLEBOXHEIGHT){
			/* shuffle has been pressed */
			shuffleRemaining = 1;
			Mix_PlayChannel(-1, getSound("shuffle"),0);
		}
	}

	if (x > NEWBOXSTARTX && x < NEWBOXLENGTH+NEWBOXSTARTX && y > NEWBOXSTARTY && y < NEWBOXSTARTY+NEWBOXHEIGHT){
		/* new has been pressed */
		startNewGame = 1;
	}

	if (x > QUITBOXSTARTX && x < QUITBOXLENGTH+QUITBOXSTARTX && y > QUITBOXSTARTY && y < QUITBOXSTARTY+QUITBOXHEIGHT){
		/* new has been pressed */
		quitGame = 1;
	}
}




/***********************************************************
synopsis: move all letters from answer to shuffle

inputs:  letters - the letter sprites

outputs:  n/a
***********************************************************/
static int
clearWord(struct sprite** letters)
{
    struct sprite* current = *letters;
    struct sprite* orderedLetters[7];
    int i;
    int count = 0;
    
	for (i = 0; i < sizeof(orderedLetters)/sizeof(orderedLetters[0]); ++i) {
		orderedLetters[i] = NULL;
	}

	/* move the letters back up */
	while (current != NULL) {
		if (current->box == ANSWER) {
			count ++;
			orderedLetters[current->index] = current;
			current->toY = SHUFFLE_BOX_Y;
			current->box = SHUFFLE;
		}
		current=current->next;
	}

	for (i=0; i < 7; i++) {
		if (orderedLetters[i] != NULL)
			orderedLetters[i]->toX = nextBlankPosition(SHUFFLE, &orderedLetters[i]->index);
	}

	return count;
}




/***********************************************************
synopsis: display the score graphic

inputs: screen - the SDL_Surface to display the image

outputs: n/a
***********************************************************/
static void
updateScore(SDL_Surface* screen)
{
    /* we'll display the total Score, this is the game score */
    
    char buffer [256];
    size_t i;
    SDL_Rect fromrect, torect, blankRect;
    
	blankRect.x = SCORE_WIDTH * 11;
	blankRect.y = 0;
	blankRect.w = SCORE_WIDTH;
	blankRect.h = SCORE_HEIGHT;

	fromrect.x = 0;
	fromrect.y = 0;
	fromrect.w = SCORE_WIDTH;
	fromrect.h = SCORE_HEIGHT;

	torect.y = 0;
	torect.w = SCORE_WIDTH;
	torect.h = SCORE_HEIGHT;

	/* move the totalScore into a string */
	snprintf (buffer, sizeof (buffer), "%i", totalScore);

	for (i = 0; i < strlen(buffer); i++){
		fromrect.x = SCORE_WIDTH * ((int)buffer[i]-48);
		torect.x = SCORE_WIDTH * i;
		SDL_BlitSurface(numberBank, &fromrect, scoreSprite->sprite, &torect);
	}
}




/***********************************************************
synopsis: displays the graphical representation of time

inputs: screen - the SDL_Surface to display the image

outputs: n/a
***********************************************************/
static void
updateTime(SDL_Surface* screen)
{
    /* the time is x seconds  minus the number of seconds of game time */
    int thisTime;
    int seconds;
    int minutes;
    int minute_units;
    int minute_tens;
    int second_units;
    int second_tens;
    
    SDL_Rect fromrect, torect, blankRect;

	blankRect.x = CLOCK_WIDTH * 11;
	blankRect.y = 0;
	blankRect.w = CLOCK_WIDTH;
	blankRect.h = CLOCK_HEIGHT;

	fromrect.x = 0;
	fromrect.y = 0;
	fromrect.w = CLOCK_WIDTH;
	fromrect.h = CLOCK_HEIGHT;

	torect.y = 0;
	torect.w = CLOCK_WIDTH;
	torect.h = CLOCK_HEIGHT;

	thisTime = AVAILABLE_TIME-gameTime;
	minutes = thisTime/60;
	seconds = thisTime-(minutes*60);
	minute_tens = minutes/10;
	minute_units = minutes-(minute_tens*10);
	second_tens = seconds/10;
	second_units = seconds-(second_tens*10);

	fromrect.x = CLOCK_WIDTH * minute_tens;
	torect.x = CLOCK_WIDTH * 0;
	SDL_BlitSurface(numberBank, &fromrect, clockSprite->sprite, &torect);
	fromrect.x = CLOCK_WIDTH * minute_units;
	torect.x = CLOCK_WIDTH * 1;
	SDL_BlitSurface(numberBank, &fromrect, clockSprite->sprite, &torect);
	fromrect.x = CLOCK_WIDTH * second_tens;
	torect.x = CLOCK_WIDTH * 3;
	SDL_BlitSurface(numberBank, &fromrect, clockSprite->sprite, &torect);
	fromrect.x = CLOCK_WIDTH * second_units;
	torect.x = CLOCK_WIDTH * 4;
	SDL_BlitSurface(numberBank, &fromrect, clockSprite->sprite, &torect);

	/* tick out the last 10 seconds */
	if (thisTime<=10 && thisTime>0) {
		Mix_PlayChannel(-1, getSound("clock-tick"), 0);
	}
}




/***********************************************************
synopsis: spin the word file to a random location and then
	  loop until a 7 or 8 letter word is found
	  this is quite a weak way to get a random word
	  considering we've got a nice dbl Dictionary to
	  hand - but it works for now.

inputs: n/a

outputs: a random word
***********************************************************/
static char *
getRandomWord()
{
    FILE* wordlist;
    int filelocation;
    int i;
    char* wordFromList = malloc(sizeof(char) * 50);
    int len=0;
    int done = 0;
    
	filelocation = rand()%10000;
	wordlist=fopen("wordlist.txt","r");

	for (i=0;i<=filelocation;i++){

		if(fscanf(wordlist, "%s", wordFromList) != EOF){
			/* spin on */
		}
		else{
			/* go back to the start of the file */
			fclose(wordlist);
			fopen("wordlist.txt", "r");
		}
	}

	/* ok random location reached */
	while (!done){

		len = strlen(wordFromList);
		if ((len==7)) {/* ||(len==6)||(len==5)){ */
			done = 1;
		}
		else{

			if(fscanf(wordlist, "%s", wordFromList) != EOF){
				/* spin on */
			}
			else{
				/* go back to the start of the file */
				fclose(wordlist);
				fopen("wordlist.txt", "r");
				fscanf(wordlist, "%s", wordFromList);
			}
		}
	}

	
	fclose(wordlist);

	/* add in our space character */
	wordFromList[len] = ' ';
	wordFromList[len+1] = '\0';

	return wordFromList;
}




/***********************************************************
synopsis: swap 2 characters in a string

inputs: from, to - the characters to swap

outputs: the swapped string
***********************************************************/
static char *
swapChars(int from, int to, char *string)
{
    char swap;

	swap = string[from];
	string[from] = string[to];
	string[to] = swap;
	return string;
}

/***********************************************************
synopsis: working backwards in the string,
	  find the first non space character

inputs: a string to check

outputs: the position of the character
***********************************************************/
static int
revFirstNonSpace(const char *thisWord)
{
    int i;

	for (i = strlen(thisWord) ; i>0; i--){
		if (thisWord[i-1] != SPACE_CHAR){
			return i;
		}
	}

	return 0;
}




/***********************************************************
synopsis: replace characters randomly

inputs: string to randomise (in/out)

outputs: n/a
***********************************************************/
static void
shuffleWord(char** thisWord)
{
    int numSwaps;
    int from, to;
    int i;
    int len;
    
	len = 7;

	numSwaps = (rand()%len)+20;

	for (i=0;i<numSwaps;i++){
		from = rand()%len;
		to = rand()%len;
		strcpy(*thisWord, swapChars(from, to, *thisWord));
	}

}




/***********************************************************
synopsis: returns the index of a specific letter in a string

inputs: string - the string to check
        letter - the letter to return

outputs: the index of the letter
***********************************************************/
static int
whereinstr(const char *s, char c)
{
    const char *p;
    if ((p = strchr(s, c)) != NULL) {
        return (p - s);
    }
    return 0;
}

/***********************************************************
synopsis: same as shuffle word, but also tell the letter 
	  sprites where to move to

inputs: thisWord - the string to shuffle (in/out)
        letters - the letter sprites

outputs: n/a
***********************************************************/
static void
shuffleAvailableLetters(char** thisWord, struct sprite** letters)
{
    struct sprite *thisLetter = *letters;
    int from, to;
    char swap, posSwap;
    char* shuffleChars;
    char* shufflePos;
    int i=0;
    int numSwaps;

	shuffleChars = malloc(sizeof(char) * 8);
	shufflePos = malloc(sizeof(char) * 8);

	for(i=0;i<7;i++){
		shufflePos[i]=i+1;
	}
	shufflePos[7] = '\0';

	strcpy(shuffleChars, *thisWord);


	numSwaps = rand()%10+20;

	for (i=0;i<numSwaps;i++){
		from = rand()%7;
		to = rand()%7;

		swap = shuffleChars[from];
		shuffleChars[from]=shuffleChars[to];
		shuffleChars[to]=swap;

		posSwap = shufflePos[from];
		shufflePos[from]=shufflePos[to];
		shufflePos[to] = posSwap;
	}

	while(thisLetter != NULL){
		if (thisLetter->box == SHUFFLE){
			thisLetter->toX = (whereinstr(shufflePos, (char)(thisLetter->index+1)) * (GAME_LETTER_WIDTH + GAME_LETTER_SPACE)) + BOX_START_X;
			thisLetter->index = whereinstr(shufflePos, (char)(thisLetter->index+1));
		}

		thisLetter = thisLetter->next;
	}

	strcpy(*thisWord, shuffleChars);

	free(shuffleChars);
	free(shufflePos);
}




/***********************************************************
synopsis: build letter string into linked list of letter graphics

inputs: letters - letter sprites head node (in/out)
	screen - the SDL_Surface to display the image

outputs: n/a
***********************************************************/
void
buildLetters(struct sprite** letters, SDL_Surface* screen)
{
    struct sprite *thisLetter = NULL, *previousLetter = NULL;
    int i;
    int len;
    SDL_Rect rect, blankRect;
    Uint32 flags = SDL_SRCCOLORKEY;
    Uint8 bpp;
    Uint32 rmask, gmask, bmask, amask;
    int index = 0;

	blankRect.x = 27 * GAME_LETTER_WIDTH;
	blankRect.y = 0;
	blankRect.w = GAME_LETTER_WIDTH;
	blankRect.h = GAME_LETTER_HEIGHT;

	rect.y = 0;
	rect.w = GAME_LETTER_WIDTH;
	rect.h = GAME_LETTER_HEIGHT;

	len = strlen(shuffle);

	if(screen->flags & SDL_SWSURFACE)
			flags |= SDL_SWSURFACE;
	if(screen->flags & SDL_HWSURFACE)
			flags |= SDL_HWSURFACE;

	bpp = screen->format->BitsPerPixel;
	rmask = screen->format->Rmask;
	gmask = screen->format->Gmask;
	bmask = screen->format->Bmask;
	amask = screen->format->Amask;

	for (i=0;i<len;i++){

		thisLetter=malloc(sizeof(struct sprite));

		/* determine which letter we're wanting and load it from 
         * the letterbank*/
		if(shuffle[i] != ASCII_SPACE && shuffle[i] != SPACE_CHAR){
			rect.x = ((int)shuffle[i]-97) * GAME_LETTER_WIDTH;
			thisLetter->sprite = SDL_CreateRGBSurface(flags, GAME_LETTER_WIDTH, GAME_LETTER_HEIGHT, bpp, rmask, gmask, bmask, amask);
			thisLetter->replace = SDL_CreateRGBSurface(flags, GAME_LETTER_WIDTH, GAME_LETTER_HEIGHT, bpp, rmask, gmask, bmask, amask);

			SDL_BlitSurface(letterBank, &rect, thisLetter->sprite, NULL);
			SDL_BlitSurface(letterBank, &blankRect, thisLetter->replace, NULL);
			thisLetter->x = rand() % 799;/*i * (GAME_LETTER_WIDTH + GAME_LETTER_SPACE) + BOX_START_X;*/
			thisLetter->y = rand() % 599; /* SHUFFLE_BOX_Y; */
			thisLetter->letter = shuffle[i];
			thisLetter->h = thisLetter->sprite->h;
			thisLetter->w = thisLetter->sprite->w;
			thisLetter->toX = i * (GAME_LETTER_WIDTH + GAME_LETTER_SPACE) + BOX_START_X;
			thisLetter->toY = SHUFFLE_BOX_Y;
			thisLetter->next = previousLetter;
			thisLetter->box = SHUFFLE;
			thisLetter->index = index++;

			previousLetter = thisLetter;

			*letters = thisLetter;

			thisLetter = NULL;
		}
		else{
			shuffle[i] = '£';
            /*	rect.x = 26 * GAME_LETTER_WIDTH;*/
		}

	}
}




/***********************************************************
synopsis: add the clock to the sprites
	  keep a module reference to it for quick and easy update
	  this sets the clock to a fixed 5:00 start

inputs: letters - letter sprites head node (in/out)
	screen - the SDL_Surface to display the image

outputs: n/a
***********************************************************/
static void
addClock(struct sprite** letters, SDL_Surface* screen)
{
    struct sprite *thisLetter = NULL;
    struct sprite *previousLetter = NULL;
    struct sprite *current = *letters;
    int i;
    SDL_Rect fromrect, torect, blankRect;
    Uint32 flags = SDL_SRCCOLORKEY;
    Uint8 bpp;
    Uint32 rmask, gmask, bmask, amask;
    int index = 0;
    
	blankRect.x = CLOCK_WIDTH * 11;
	blankRect.y = 0;
	blankRect.w = CLOCK_WIDTH;
	blankRect.h = CLOCK_HEIGHT;

	fromrect.x = 0;
	fromrect.y = 0;
	fromrect.w = CLOCK_WIDTH;
	fromrect.h = CLOCK_HEIGHT;

	torect.y = 0;
	torect.w = CLOCK_WIDTH;
	torect.h = CLOCK_HEIGHT;
	
	if(screen->flags & SDL_SWSURFACE)
			flags |= SDL_SWSURFACE;
	if(screen->flags & SDL_HWSURFACE)
			flags |= SDL_HWSURFACE;

	bpp = screen->format->BitsPerPixel;
	rmask = screen->format->Rmask;
	gmask = screen->format->Gmask;
	bmask = screen->format->Bmask;
	amask = screen->format->Amask;

	/* add the clock onto the end - so we don't slow letter processing any*/
	while (current != NULL){
		previousLetter = current;
		current =current->next;
	}

	thisLetter=malloc(sizeof(struct sprite));

	thisLetter->sprite = SDL_CreateRGBSurface(flags, CLOCK_WIDTH*5, CLOCK_HEIGHT, bpp, rmask, gmask, bmask, amask);
	thisLetter->replace = SDL_CreateRGBSurface(flags, CLOCK_WIDTH*5, CLOCK_HEIGHT, bpp, rmask, gmask, bmask, amask);

	/* initialise with 05:00*/
	for (i=0;i<5;i++){

        /*	printf("i:%i\n", CLOCK_WIDTH * i); */
		
		torect.x = CLOCK_WIDTH * i;
		switch(i){

			case 1:
				fromrect.x = 5 * CLOCK_WIDTH;
				break;
			case 2: 
				fromrect.x = CLOCK_WIDTH * 10; /* the colon */
				break;
			case 0:
			case 3:
			case 4:
				fromrect.x = 0;
				break;
			default:
				break;
		}

		SDL_BlitSurface(numberBank, &fromrect, thisLetter->sprite, &torect);

		SDL_BlitSurface(numberBank, &blankRect, thisLetter->replace, &torect);
	}

	thisLetter->x = CLOCK_X;
	thisLetter->y = CLOCK_Y;
	thisLetter->h = thisLetter->sprite->h;
	thisLetter->w = thisLetter->sprite->w;
	thisLetter->toX = thisLetter->x;
	thisLetter->toY = thisLetter->y;
	thisLetter->next = NULL;
	thisLetter->box = CONTROLS;
	thisLetter->index = index++;

	previousLetter->next = thisLetter;
	clockSprite = thisLetter;
}




/***********************************************************
synopsis: add the Score to the sprites
          keep a module reference to it for quick and easy update

inputs: letters - letter sprites head node (in/out)
	screen - the SDL_Surface to display the image

outputs: n/a
***********************************************************/
static void
addScore(struct sprite** letters, SDL_Surface* screen)
{
    struct sprite *thisLetter = NULL;
    struct sprite *previousLetter = NULL;
    struct sprite *current = *letters;
    SDL_Rect fromrect, torect, blankRect;
    Uint32 flags = SDL_SRCCOLORKEY;
    Uint8 bpp;
    Uint32 rmask, gmask, bmask, amask;
    int index = 0;
    int i;

	blankRect.x = SCORE_WIDTH * 11;
	blankRect.y = 0;
	blankRect.w = SCORE_WIDTH;
	blankRect.h = SCORE_HEIGHT;

	fromrect.x = 0;
	fromrect.y = 0;
	fromrect.w = SCORE_WIDTH;
	fromrect.h = SCORE_HEIGHT;

	torect.y = 0;
	torect.w = SCORE_WIDTH;
	torect.h = SCORE_HEIGHT;
	
	if(screen->flags & SDL_SWSURFACE)
			flags |= SDL_SWSURFACE;
	if(screen->flags & SDL_HWSURFACE)
			flags |= SDL_HWSURFACE;

	bpp = screen->format->BitsPerPixel;
	rmask = screen->format->Rmask;
	gmask = screen->format->Gmask;
	bmask = screen->format->Bmask;
	amask = screen->format->Amask;

	/* add the score onto the end - so we don't slow letter processing any */
	while (current != NULL){
		previousLetter = current;
		current =current->next;
	}

	/* previousLetter = clockSprite;*/
	
	thisLetter = malloc(sizeof(struct sprite));

	thisLetter->sprite = SDL_CreateRGBSurface(flags, SCORE_WIDTH*5, SCORE_HEIGHT, bpp, rmask, gmask, bmask, amask);
	thisLetter->replace = SDL_CreateRGBSurface(flags, SCORE_WIDTH*5, SCORE_HEIGHT, bpp, rmask, gmask, bmask, amask);

	for (i=0;i<5;i++){
		if (i==0)
			fromrect.x = 0;
		else
			fromrect.x = SCORE_WIDTH*11;
		torect.x = i * SCORE_WIDTH;
		SDL_BlitSurface(numberBank, &fromrect, thisLetter->sprite, &torect);
		SDL_BlitSurface(numberBank, &blankRect, thisLetter->replace, &torect);
	}

	thisLetter->x = SCORE_X;
	thisLetter->y = SCORE_Y;
	thisLetter->h = thisLetter->sprite->h;
	thisLetter->w = thisLetter->sprite->w;
	thisLetter->toX = thisLetter->x;
	thisLetter->toY = thisLetter->y;
	thisLetter->next = NULL;
	thisLetter->box = CONTROLS;
	thisLetter->index = index++;

	previousLetter->next = thisLetter;
	scoreSprite = thisLetter;
}




/***********************************************************
synopsis: do all of the initialisation for a new game:
          build the screen
	  get a random word and generate anagrams
	  (must get less than 66 anagrams to display on screen)
	  initialise all the game control flags

inputs: head - first node in the answers list (in/out)
        dblHead - first node in the dictionary list
	screen - the SDL_Surface to display the image
	letters - first node in the letter sprites (in/out)

outputs: n/a
***********************************************************/
static void
newGame(struct node** head, struct dlb_node* dlbHead, 
        SDL_Surface* screen, struct sprite** letters)
{
    char* guess;
    char* remain;
    int happy = 0;   /* we don't want any more than ones with 66 answers */
                     /* - that's all we can show... */
    int i;

	/* show background */
	ShowBMP("images/background.bmp",screen, 0,0);

	destroyLetters(letters);

	guess = malloc(sizeof(char)*50);
	remain = malloc(sizeof(char)*50);

	while (!happy) {
		strcpy(guess,"");
		strcpy(rootWord, getRandomWord());
		bigWordLen = strlen(rootWord)-1;
		strcpy(remain,rootWord);

		rootWord[bigWordLen] = '\0';

		/* destroy answers list */
		destroyAnswers(head);

		/* generate anagrams from random word */
		ag(head, dlbHead, guess, remain);

		sort(head);
		answersSought = Length(*head);
		happy = ((answersSought <= 77) && (answersSought >= 6));

#ifdef DEBUG
		if (!happy) {
			printf("Too Many Answers!  word: %s, answers: %i\n",
                   rootWord, answersSought);
		}
#endif
	}

	for (i = bigWordLen; i < 7; i++){
		remain[i]='£';
	}

	remain[7] = '\0';

	remain[bigWordLen]='\0';

	shuffleWord(&remain);
	strcpy(shuffle, remain);

	free(guess);
	free(remain);

	strcpy(answer, "£££££££");

	/* build up the letter sprites */
	buildLetters(letters, screen);
	addClock(letters, screen);
	addScore(letters, screen);

	/* display all answer boxes */
	displayAnswerBoxes(*head, screen);

	gotBigWord = 0;
	score = 0;
	updateTheScore = 1;
	gamePaused = 0;
	winGame = 0;
	answersGot = 0;

	gameStart = time(0);
	gameTime = 0;
	stopTheClock = 0;
}

static Uint32
TimerCallback(Uint32 interval, void *param)
{
    SDL_UserEvent evt;
    evt.type = SDL_USEREVENT;
    evt.code = 0;
    evt.data1 = 0;
    evt.data2 = 0;
    SDL_PushEvent((SDL_Event *)&evt);
    return 0;
}

/***********************************************************
synopsis: a big while loop that runs the full length of the
	  game, checks the game events and responds
	  accordingly

	  event		action
	  -------------------------------------------------
	  winGame	stop the clock and solve puzzle
	  timeRemaining update the clock tick
	  timeUp	stop the clock and solve puzzle
	  solvePuzzle	trigger solve puzzle and stop clock
	  updateAnswers trigger update answers
	  startNew      trigger start new
	  updateScore	trigger update score
	  shuffle	trigger shuffle
	  clear		trigger clear answer
	  quit		end loop
	  poll events   check for keyboard/mouse and quit

	  finally, move the sprites -this is always called
	  so the sprites are always considered to be moving
	  no "move sprites" event exists - sprites x&y just
	  needs to be updated and they will always be moved

inputs: head - first node in the answers list (in/out)
        dblHead - first node in the dictionary list
	screen - the SDL_Surface to display the image
	letters - first node in the letter sprites (in/out)

outputs: n/a
***********************************************************/
static void
gameLoop(struct node **head, struct dlb_node *dlbHead, 
         SDL_Surface *screen, struct sprite **letters)
{
    int done=0;
    SDL_Event event;
    int timeNow;
    SDL_TimerID timer;
    int timer_delay = 20;
    
    timer = SDL_AddTimer(timer_delay, TimerCallback, NULL);
	/* main game loop */
	while (!done) {

		if (winGame) {

			stopTheClock = 1;
			solvePuzzle = 1;
		}

		if ((gameTime < AVAILABLE_TIME) && !stopTheClock) {
			timeNow = time(0) - gameStart;
			if (timeNow!=gameTime){
				gameTime = timeNow;
				updateTime(screen);
			}
		}
		else{
			if (!stopTheClock){
				stopTheClock = 1;
				solvePuzzle = 1;
			}
		}

		/* check messages */
		if (solvePuzzle) {
			/* walk the list, setting everything to found */
			solveIt(*head);
			clearWord(letters);
			strcpy(shuffle, "£££££££");
			strcpy(answer, rootWord);
			/*displayLetters(screen);*/
			displayAnswerBoxes(*head, screen);
			gamePaused = 1;
			if (!stopTheClock){
				stopTheClock = 1;
			}

			solvePuzzle = 0;
		}

		if (updateAnswers){
			/* move letters back down again */
			clearWord(letters);
			/* displayLetters(screen);*/
			displayAnswerBoxes(*head, screen);

			updateAnswers = 0;
		}

		if(startNewGame){
			/* move letters back down again */
			if (!gotBigWord){
				totalScore = 0;
			}
			newGame(head, dlbHead, screen, letters);

			startNewGame = 0;
		}

		if(updateTheScore){
			updateScore(screen);

			updateTheScore = 0;
		}

		if (shuffleRemaining){
			/* shuffle up the shuffle box */
			char *shuffler;
			shuffler = malloc(sizeof(char) * 8);
			strcpy(shuffler, shuffle);
			shuffleAvailableLetters(&shuffler, letters);
			strcpy(shuffle, shuffler);
			free(shuffler);
			/* clearLetters(screen); */
			/* displayLetters(screen); */

			shuffleRemaining = 0;
		}

		if (clearGuess){
			/* clear the guess; */
			if (clearWord(letters) > 0)
				Mix_PlayChannel(-1, getSound("clear"),0);

			clearGuess = 0;
		}

		if (quitGame){
			done=1;
		}

		while (SDL_WaitEvent(&event))
		{
			if (event.type == SDL_USEREVENT) {
                timer_delay = anySpritesMoving(letters) ? 10 : 100;
                moveSprites(&screen, letters, letterSpeed);
                timer = SDL_AddTimer(timer_delay, TimerCallback, NULL);
                break;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                clickDetect(event.button.button, event.button.x,
                            event.button.y, screen, *head, letters);
            } else if (event.type == SDL_KEYUP) {
                handleKeyboardEvent(&event, *head, letters);
            } else if (event.type == SDL_QUIT) {
                done = 1;
                break;
			}
            moveSprites(&screen, letters, letterSpeed);
        }
    }
}




/***********************************************************
synopsis: initialise graphics and sound, build the dictionary
          cache the images and start the game loop
	  when the game is done tidy up

inputs: argc - argument count
        argv - arguments

outputs: retval  0 = success   1 = failure
***********************************************************/
int
main(int argc, char *argv[])
{
    struct node* head = NULL;
    struct dlb_node* dlbHead = NULL;
    SDL_Surface *screen;
    struct sprite* letters = NULL;

	/* buffer sounds */
	int audio_rate = MIX_DEFAULT_FREQUENCY;
	Uint16 audio_format = AUDIO_S16;
	int audio_channels = 1;
	int audio_buffers = 256;

	/* seed the random generator */
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0){
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);

	screen=SDL_SetVideoMode(800, 600, 16, SDL_HWSURFACE|SDL_DOUBLEBUF);
	if (screen == NULL)
	{
		printf("Unable to set 800x600 video: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("Anagramarama", "ANAGRAMARAMA");

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)){
		printf("unable to open audio!\n");
		exit(1);
	}

	bufferSounds(&soundCache);

	/* create dictionary */
	createDLBTree(&dlbHead);

	/* cache in-game graphics */
	letterBank = SDL_LoadBMP("images/letterBank.bmp");
	smallLetterBank = SDL_LoadBMP("images/smallLetterBank.bmp");
	numberBank = SDL_LoadBMP("images/numberBank.bmp");

	rootWord = malloc(sizeof(char)*9);
	newGame(&head, dlbHead, screen, &letters);

	gameLoop(&head, dlbHead, screen, &letters);

	/* tidy up and exit */
	free(rootWord);
	Mix_CloseAudio();
	clearSoundBuffer(&soundCache);
	/* trashDLBTree(); */
	destroyLetters(&letters);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(letterBank);
	SDL_FreeSurface(smallLetterBank);
	SDL_FreeSurface(numberBank);
	/*SDL_Quit(); */
	return 0;
}
