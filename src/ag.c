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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
//#include <pthread.h>

#include "dlb.h"
#include "linked.h"
#include "sprite.h"
#include "ag.h"

//module level variables for game control
char shuffle[]  = "£££££££";
char answer[]   = "£££££££";

char* rootWord;
int updateAnswers = 0;
int startNewGame = 0;
int solvePuzzle = 0;
int shuffleRemaining = 0;
int clearGuess = 0;
//int winGame = 0;

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

// Graphics cache
SDL_Surface* letterBank = NULL;
SDL_Surface* smallLetterBank = NULL;
SDL_Surface* numberBank = NULL;
struct sprite* clockSprite = NULL;
struct sprite* scoreSprite = NULL;

// audio vars
Uint32 audio_len;
Uint8 *audio_pos;

struct sound{
	char* name;
	Mix_Chunk *audio_chunk;
	struct sound* next;
};

struct sound* soundCache = NULL;
/*******************************************************************************/
Mix_Chunk* getSound(char* name){

struct sound* currentSound = soundCache;

	while (currentSound!=NULL){

		if(!strcmp(currentSound->name, name)){
			return currentSound->audio_chunk;
			break;
		}
		currentSound = currentSound->next;
	}

	return NULL;
}

/*******************************************************************************/
void pushSound(struct sound** soundCache, char* name, char* filename){

struct sound* thisSound = NULL;

	thisSound = malloc(sizeof(struct sound));
	thisSound->name = malloc(sizeof(name)*strlen(name));
	strcpy(thisSound->name, name);
	thisSound->next = *soundCache;

	// Attempt to load a sample
	thisSound->audio_chunk = Mix_LoadWAV(filename);

	*soundCache = thisSound;
}

/*******************************************************************************/
void bufferSounds(struct sound** soundCache){

	pushSound(&(*soundCache),"click-answer", "audio/click-answer.wav");
	pushSound(&(*soundCache),"click-shuffle", "audio/click-shuffle.wav");
	pushSound(&(*soundCache),"foundbig", "audio/foundbig.wav");
	pushSound(&(*soundCache),"found", "audio/found.wav");
	pushSound(&(*soundCache),"clear", "audio/clearword.wav");
	pushSound(&(*soundCache),"duplicate", "audio/duplicate.wav");
	pushSound(&(*soundCache),"badword", "audio/badword.wav");
	pushSound(&(*soundCache),"shuffle", "audio/shuffle.wav");
	pushSound(&(*soundCache),"clock-tick", "audio/clock-tick.wav");

}

/*******************************************************************************/
void clearSoundBuffer(){

struct sound* currentSound = soundCache, *previousSound = NULL;

	while (currentSound!=NULL){

		Mix_FreeChunk(currentSound->audio_chunk);
		free(currentSound->name);
		previousSound = currentSound;
		currentSound = currentSound->next;
		free(previousSound);
	}
}

/***********************************************************/
int nextBlank(char* string){
// determine the next blank space in a string - blanks are indicated by pound £ not space

int i;
int found=0;

	for(i=0;i<7;i++){
		if (string[i]==SPACE_CHAR){
			found = 1;
			break;
		}
	}
	if (found){
		return i+1;
	}
	else{
		return 0;
	}
}

/***********************************************************/
char* shiftLeftKill(char* string){
// shift a string of characters 1 character to the left

int i;
char start;
char* newString;
int len;

	len = strlen(string);

	newString = malloc(sizeof(char) * (len));

	start = string[0];

	for (i=1;i<len;i++){
		newString[i-1] = string[i];
        }

	newString[len-1] = '\0';

	return(newString);
	free(newString);
}

/***********************************************************/
char* shiftLeft(char* string){
// shift a string of characters 1 character to the left
// move the first character to the end of the string

int i;
char start;
char* newString;
int len;

	len = strlen(string);

	newString = malloc(sizeof(char) * (len+1));

	start = string[0];

	for (i=1;i<len;i++){
		newString[i-1] = string[i];
        }

	newString[len-1] = start;
	newString[len] = '\0';

	return(newString);
	free(newString);
}

/***********************************************************/
void ag(struct node** head, struct dlb_node* dlbHead, char** guess, char** remain){
// generate all possible combinations of the root word
// the initial letter is fixed (hence the space character
// at the end of the possible list)

char*  newGuess;
char*  newRemain;
int    totalLen=0, guessLen=0, remainLen=0, i;

	// allocate space for our working variables
	guessLen = strlen(*guess);
	remainLen = strlen(*remain);
	totalLen = guessLen + remainLen;

	newGuess = malloc(sizeof(char) * (totalLen+1));
	newRemain = malloc(sizeof(char) * (totalLen+1));

	// move last remaining letter to end of guess
	strcpy(newGuess, *guess);
	strcpy(newRemain, *remain);
	newGuess[guessLen] = newRemain[remainLen-1];
	newGuess[guessLen+1] = '\0';
	newRemain[remainLen-1] = '\0';

//	printf("%s\n", newGuess);

	if(strlen(newGuess) > 3){
		if (dlb_lookup(dlbHead,shiftLeftKill(newGuess))){
			push(&(*head), shiftLeftKill(newGuess));
		}
	}

	if (strlen(newRemain)){
		ag(&(*head), dlbHead, &newGuess, &newRemain);

		for (i=totalLen-1;i>0;i--){
			if(strlen(newRemain) > i){
				strcpy(newRemain, shiftLeft(newRemain));
					ag(&(*head), dlbHead, &newGuess, &newRemain);
			}
		}
	}
	// free the space
	free(newGuess);
	free(newRemain);
}

/***********************************************************/
void solveIt(struct node* head){

struct node* current = head;

	while (current != NULL){
		current->found = 1;
		current = current->next;
	}
}

/***********************************************************/
void ShowBMP(char *file, SDL_Surface *screen, int x, int y){
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
	//SDL_UpdateRects(screen, 1, &dest);
	SDL_FreeSurface(image);
}

/***********************************************************/
/*void displayAnswerBoxes(struct node* head, SDL_Surface* screen){
// for each letter to each answer in the nodelist,
// display a neat little box to fill in later

struct node* current = head;
SDL_Rect outerrect, innerrect;
int i;

char imagePath[] = "images/small-£.bmp";
int twoOnALine = 0;

	// width and height are always the same
	outerrect.w = 17;
	outerrect.h = 17;
	outerrect.x = 70;
	outerrect.y = 380;

        while (current != NULL){

		// update the x for each letter
		for (i=0;i<current->length;i++){
			SDL_FillRect(screen, &outerrect,0);
			innerrect.w = outerrect.w - 2;
			innerrect.h = outerrect.h - 2;
			innerrect.x = outerrect.x + 1;
			innerrect.y = outerrect.y + 1;

			if (current->guessed){
				SDL_FillRect(screen, &innerrect,SDL_MapRGB(screen->format,255,255,255));
			}
			else{
				SDL_FillRect(screen, &innerrect,SDL_MapRGB(screen->format,217,220,255));
			}

			if (current->found){
				imagePath[13] = current->anagram[i];
				ShowBMP(imagePath, screen,  innerrect.x+2, innerrect.y );
			}

			outerrect.x += 18;
		}

		if ( twoOnALine<4 ){
			// shift x along to show the next word
			outerrect.x += (8-current->length) * 18;
			twoOnALine++;
		}
		else{
			// reset the x, ready for the next line
			outerrect.x = 70;
			twoOnALine = 0;
			// update the y for each word
			outerrect.y += 19;
		}
		current=current->next;
	}
	SDL_Flip(screen);
}
*/
/***********************************************************/
void displayAnswerBoxes(struct node* head, SDL_Surface* screen){
// for each letter to each answer in the nodelist,
// display a neat little box to fill in later

struct node* current = head;
SDL_Rect outerrect, innerrect, letterBankRect;
int i;
int numWords = 0;
int acrossOffset = 70;
int numLetters = 0;
int listLetters = 0;
//char imagePath[] = "images/small-£.bmp";

	// width and height are always the same
	outerrect.w = 16;
	outerrect.h = 16;
	outerrect.x = acrossOffset;
	outerrect.y = 380;

	letterBankRect.w = 10;
	letterBankRect.h = 16;
	letterBankRect.y = 0;
	letterBankRect.x = 0; // letter is chosen by 10*letter where a is 0

        while (current != NULL){

		// new word
		numWords++;
		numLetters =0;

		// update the x for each letter
		for (i=0;i<current->length;i++){

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
//				innerrect.y-=1;
				letterBankRect.x = 10 * ((int)current->anagram[i] - 97);
				//imagePath[13] = current->anagram[i];
				//ShowBMP(imagePath, screen,  innerrect.x+2, innerrect.y );
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
//	SDL_Flip(screen);
}

/***********************************************************/
void checkGuess(char* answer, struct node* head){
// check the guess against the answers
struct node* current = head;
int i, len;
int foundWord = 0;

	len = nextBlank(answer) -1;
	if (len<0) len=7;
	char test[len];
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
					// just a normal word
					Mix_PlayChannel(-1, getSound("found"),0);
				}
				if (answersSought==answersGot){
					// getting all answers gives us the game score again!!
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

/***********************************************************/
int nextBlankPosition(int box, int* index){
// determine the next blank space in a string - blanks are indicated by pound £ not space

int i;

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
/***********************************************************/
void clickDetect(int button, int x, int y, SDL_Surface *screen, struct node* head, struct sprite** letters){

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
			// clear has been pressed
			clearGuess = 1;
		}

		// check the other hotspots
		if (x > ENTERBOXSTARTX && x < ENTERBOXLENGTH+ENTERBOXSTARTX && y > ENTERBOXSTARTY && y < ENTERBOXSTARTY+ENTERBOXHEIGHT){
			// enter has been pressed
			checkGuess(answer, head);
		}

		if (x > SOLVEBOXSTARTX && x < SOLVEBOXLENGTH+SOLVEBOXSTARTX && y > SOLVEBOXSTARTY && y < SOLVEBOXSTARTY+SOLVEBOXHEIGHT){
			// solve has been pressed
			solvePuzzle = 1;
		}
		
		if (x > SHUFFLEBOXSTARTX && x < SHUFFLEBOXLENGTH+SHUFFLEBOXSTARTX && y > SHUFFLEBOXSTARTY && y < SHUFFLEBOXSTARTY+SHUFFLEBOXHEIGHT){
			// shuffle has been pressed
			shuffleRemaining = 1;
			Mix_PlayChannel(-1, getSound("shuffle"),0);
		}
	}

	if (x > NEWBOXSTARTX && x < NEWBOXLENGTH+NEWBOXSTARTX && y > NEWBOXSTARTY && y < NEWBOXSTARTY+NEWBOXHEIGHT){
		// new has been pressed
		startNewGame = 1;
	}

	if (x > QUITBOXSTARTX && x < QUITBOXLENGTH+QUITBOXSTARTX && y > QUITBOXSTARTY && y < QUITBOXSTARTY+QUITBOXHEIGHT){
		// new has been pressed
		quitGame = 1;
	}
}

/***********************************************************/
int clearWord(struct sprite** letters){

struct sprite* current = *letters;
struct sprite* orderedLetters[7];
int i;
int count = 0;

	for (i=0;i<=7;i++){
		orderedLetters[i] = NULL;
	}

	// move the letters back up
	while(current != NULL){
		if(current->box == ANSWER){
			count ++;
			orderedLetters[current->index] = current;
			current->toY = SHUFFLE_BOX_Y;
			current->box = SHUFFLE;
		}
		current=current->next;
	}

	for (i=0;i<=7;i++){
		if(orderedLetters[i] != NULL)
			orderedLetters[i]->toX = nextBlankPosition(SHUFFLE, &orderedLetters[i]->index);
	}

	return count;
}
/***********************************************************/
void updateScore(SDL_Surface* screen){
// we'll display the total Score, this is the game score

char buffer [256];
int i;
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

	// move the totalScore into a string
	snprintf (buffer, sizeof (buffer), "%i", totalScore);

	for (i = 0; i < strlen(buffer); i++){
		fromrect.x = SCORE_WIDTH * ((int)buffer[i]-48);
		torect.x = SCORE_WIDTH * i;
		SDL_BlitSurface(numberBank, &fromrect, scoreSprite->sprite, &torect);
	}
}
/***********************************************************/
void updateTime(SDL_Surface* screen){

// the time is x seconds  minus the number of seconds of game time
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

	// tick out the last 10 seconds
	if (thisTime<=10 && thisTime>0){
		Mix_PlayChannel(-1, getSound("clock-tick"), 0);
	}
}
/***********************************************************/
char* getRandomWord(){
// spin the word file to a random location and then
// loop until a 7 or 8 letter word is found

FILE* wordlist;
int filelocation;
int i;
char* wordFromList = malloc(sizeof(char) * 50);
int len;
int done = 0;

	filelocation = rand()%10000;
	wordlist=fopen("wordlist.txt","r");

	for (i=0;i<=filelocation;i++){

		if(fscanf(wordlist, "%s", wordFromList) != EOF){
			// spin on
		}
		else{
			// go back to the start of the file
			fclose(wordlist);
			fopen("wordlist.txt", "r");
		}
	}

	// ok random location reached
	while (!done){

		len = strlen(wordFromList);
		if ((len==7)){//||(len==6)||(len==5)){
			done = 1;
		}
		else{

			if(fscanf(wordlist, "%s", wordFromList) != EOF){
				// spin on
			}
			else{
				// go back to the start of the file
				fclose(wordlist);
				fopen("wordlist.txt", "r");
				fscanf(wordlist, "%s", wordFromList);
			}
		}
	}

	
	fclose(wordlist);

	// add in our space character
	wordFromList[len] = ' ';
	wordFromList[len+1] = '\0';

	return wordFromList;
	free(wordFromList);
}

/***********************************************************/
char* swapChars(int from, int to, char* string){
// swap 2 characters in a string

char swap;

	swap = string[from];
	string[from] = string[to];
	string[to] = swap;

	return string;

}

/***********************************************************/
int revFirstNonSpace(char* thisWord){
// working backwards in the string,
// find the first non space character
int i;

	for (i = strlen(thisWord) ; i>0; i--){
		if (thisWord[i-1] != SPACE_CHAR){
			return i;
		}
	}

	return 0;
}

/***********************************************************/
void shuffleWord(char** thisWord){
// replace characters randomly

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

	// move all the spaces to the end
//	strcpy(buffer, *thisWord);
//	for (i=0;i<strlen(buffer);i++){
//		if(buffer[i] == SPACE_CHAR){
//			from = i;
//			to = revFirstNonSpace(buffer);
//			if (to == 0) break;	// no spaces
//			to--;
//			if (from < to){
//				letter = buffer[i+1];
//				buffer[i+1] = buffer[i];
//				buffer[i] = letter;
//			}
//		}
//	}
//	strcpy(*thisWord, buffer);
}

/***********************************************************/
int whereinstr(char* string, char letter){
// returns the index of a specific letter in a string
int i, len;

	len=strlen(string);

	for(i=0;i<len;i++){
		if (string[i]==letter){
			return i;
		}
	}

	return 0;
}
	
/***********************************************************/
void shuffleAvailableLetters(char** thisWord, struct sprite** letters){
// same as shuffle word, but also tell the letter sprites where
// to move to

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
			thisLetter->toX = (whereinstr(shufflePos, thisLetter->index+1) * (GAME_LETTER_WIDTH + GAME_LETTER_SPACE)) + BOX_START_X;
			thisLetter->index = whereinstr(shufflePos, thisLetter->index+1);
		}

		thisLetter = thisLetter->next;
	}

	strcpy(*thisWord, shuffleChars);

	free(shuffleChars);
	free(shufflePos);
}
/***********************************************************/
/*void* updateClock(void *arg){

int timeNow;
	while((gameTime < AVAILABLE_TIME) && !stopTheClock){
		timeNow = time(0) - gameStart;
		if (timeNow!=gameTime){
			gameTime = timeNow;
			updateTimeDisplay = 1;		
			//sleep(1);
		}
	}
	stopTheClock = 1;
	solvePuzzle = 1;
	return NULL;
}
*/
/********************************************************************/
void buildLetters(struct sprite** letters, SDL_Surface* screen){
// build letter string into linked list of letter graphics

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

		// determine which letter we're wanting and load it from the letterbank
		if(shuffle[i] != ASCII_SPACE && shuffle[i] != SPACE_CHAR){
			rect.x = ((int)shuffle[i]-97) * GAME_LETTER_WIDTH;
			thisLetter->sprite = SDL_CreateRGBSurface(flags, GAME_LETTER_WIDTH, GAME_LETTER_HEIGHT, bpp, rmask, gmask, bmask, amask);
			thisLetter->replace = SDL_CreateRGBSurface(flags, GAME_LETTER_WIDTH, GAME_LETTER_HEIGHT, bpp, rmask, gmask, bmask, amask);

			SDL_BlitSurface(letterBank, &rect, thisLetter->sprite, NULL);
			SDL_BlitSurface(letterBank, &blankRect, thisLetter->replace, NULL);
			thisLetter->x = rand()%799;//i * (GAME_LETTER_WIDTH + GAME_LETTER_SPACE) + BOX_START_X;
			thisLetter->y = rand()%599;//SHUFFLE_BOX_Y;
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
		//	rect.x = 26 * GAME_LETTER_WIDTH;
		}

	}
}

/***********************************************************/
void addClock(struct sprite** letters, SDL_Surface* screen){
// add the clock to the sprites
// keep a module reference to it for quick and easy update

struct sprite *thisLetter = NULL, *previousLetter = NULL, *current = *letters;
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

	// add the clock onto the end - so we don't slow letter processing any
	while (current != NULL){
		previousLetter = current;
		current =current->next;
	}

	thisLetter=malloc(sizeof(struct sprite));

	thisLetter->sprite = SDL_CreateRGBSurface(flags, CLOCK_WIDTH*5, CLOCK_HEIGHT, bpp, rmask, gmask, bmask, amask);
	thisLetter->replace = SDL_CreateRGBSurface(flags, CLOCK_WIDTH*5, CLOCK_HEIGHT, bpp, rmask, gmask, bmask, amask);

	// initialise with 05:00
	for (i=0;i<5;i++){

//		printf("i:%i\n", CLOCK_WIDTH * i);
		
		torect.x = CLOCK_WIDTH * i;
		switch(i){

			case 1:
				fromrect.x = 5 * CLOCK_WIDTH;
				break;
			case 2: 
				fromrect.x = CLOCK_WIDTH * 10; // the colon
				break;
			case 0:
			case 3:
			case 4:
				fromrect.x = 0;
				break;
			default:
				break;
		}
		//if(i!=2)
		//	fromrect.x = 0;
		//else
		//	fromrect.x = CLOCK_WIDTH * 10; //the colon

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

/***********************************************************/
void addScore(struct sprite** letters, SDL_Surface* screen){
// add the Score to the sprites
// keep a module reference to it for quick and easy update

struct sprite *thisLetter = NULL, *previousLetter = NULL, *current = *letters;
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

	// add the score onto the end - so we don't slow letter processing any
	while (current != NULL){
		previousLetter = current;
		current =current->next;
	}

	//previousLetter = clockSprite;
	
	thisLetter=malloc(sizeof(struct sprite));

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
/***********************************************************/
void newGame(struct node** head, struct dlb_node* dlbHead, SDL_Surface* screen, struct sprite** letters){

char* guess;
char* remain;
//pthread_t clock;
int happy = 0;   // we don't want any more than ones with 66 answers - that's all we can show...
int i;

	// show background
	ShowBMP("images/background.bmp",screen, 0,0);

	destroyLetters(&(*letters));

	guess = malloc(sizeof(char)*50);
	remain = malloc(sizeof(char)*50);

	while (!happy){
		strcpy(guess,"");
		strcpy(rootWord, getRandomWord());
		bigWordLen = strlen(rootWord)-1;
		strcpy(remain,rootWord);

		rootWord[bigWordLen] = '\0';

		// destroy answers list
		destroyAnswers(&(*head));

		// generate anagrams from random word
		ag(&(*head), dlbHead, &guess, &remain);

		sort(&(*head));
		answersSought = Length(*head);
		happy = ((answersSought <= 77) && (answersSought >= 6));

		if(!happy){
			printf("Too Many Answers!  word: %s, answers: %i\n", rootWord, answersSought);
		}
	}

	for (i=bigWordLen;i<7;i++){
		remain[i]='£';
	}

	remain[7] = '\0';

//	printf("%s,%i\n", remain, bigWordLen);
	remain[bigWordLen]='\0';

	shuffleWord(&remain);
	strcpy(shuffle, remain);

//	printf("shuffle:%s\n", shuffle);
	free(guess);
	free(remain);

	strcpy(answer, "£££££££");

	//build up the letter sprites
	buildLetters(&(*letters), screen);
	addClock(&(*letters), screen);
	addScore(&(*letters), screen);

	// display all answer boxes
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

	// create the clock loop on a new thread
	//pthread_create(&clock, NULL, updateClock, NULL);
}
/***********************************************************/
void gameLoop(struct node** head, struct dlb_node* dlbHead, SDL_Surface* screen, struct sprite** letters){

int done=0;
SDL_Event event;
int timeNow;

	// main game loop
	while (!done){

		if(winGame){

			stopTheClock = 1;
			solvePuzzle = 1;
		}

		if((gameTime < AVAILABLE_TIME) && !stopTheClock){
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

		// check messages
		if(solvePuzzle){
			// walk the list, setting everything to found
			solveIt(*head);
			clearWord(&(*letters));
			strcpy(shuffle, "£££££££");
			strcpy(answer, rootWord);
			//displayLetters(screen);
			displayAnswerBoxes(*head, screen);
			gamePaused = 1;
			if (!stopTheClock){
				stopTheClock = 1;
			}

			solvePuzzle = 0;
		}

		if (updateAnswers){
			//move letters back down again
			clearWord(&(*letters));
			//displayLetters(screen);
			displayAnswerBoxes(*head, screen);

			updateAnswers = 0;
		}

		if(startNewGame){
			//move letters back down again
			if (!gotBigWord){
				totalScore = 0;
			}
			newGame(&(*head), dlbHead, screen, &(*letters));

			startNewGame = 0;
		}

		if(updateTheScore){
			updateScore(screen);

			updateTheScore = 0;
		}

		if (shuffleRemaining){
			// shuffle up the shuffle box
			char* shuffler;
			shuffler = malloc(sizeof(char)*8);
			strcpy(shuffler, shuffle);
			shuffleAvailableLetters(&shuffler, &(*letters));
			strcpy(shuffle, shuffler);
			free(shuffler);
			//clearLetters(screen);
			//displayLetters(screen);

			shuffleRemaining = 0;


		}

		if (clearGuess){
			// clear the guess;
			if (clearWord(&(*letters)) > 0)
				Mix_PlayChannel(-1, getSound("clear"),0);

			clearGuess = 0;
		}

		if (quitGame){
			done=1;
		}

		while (SDL_PollEvent(&event))
		{
			switch (event.type) {
				case SDL_MOUSEBUTTONDOWN:
//			printf("shuffle: %s\n", shuffle);
//			printf("guess:   %s\n", answer);
					clickDetect(event.button.button, event.button.x, event.button.y, screen, *head, &(*letters));
					break;
				case SDL_QUIT:
					done=1;
			}
		}
		moveSprites(&screen, &(*letters), letterSpeed);
	}
}

/***********************************************************/
int main(int argc, char *argv[]){

struct node* head = NULL;
struct dlb_node* dlbHead = NULL;
SDL_Surface *screen;
struct sprite* letters = NULL;
//pthread_t audio;

	// seed the random generator
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0){
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);

	screen=SDL_SetVideoMode(800,600,16,SDL_HWSURFACE|SDL_DOUBLEBUF);//|SDL_FULLSCREEN);
	if (screen == NULL)
	{
		printf("Unable to set 800x600 video: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("Anagramarama", "ANAGRAMARAMA");

	// buffer sounds
	int audio_rate = MIX_DEFAULT_FREQUENCY;
	Uint16 audio_format = AUDIO_S16;
	int audio_channels = 1;
	int audio_buffers = 256;

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)){
		printf("unable to open audio!\n");
		exit(1);
	}

	bufferSounds(&soundCache);

	// create the audio handler on a new thread
	//pthread_create(&audio, NULL, handleSounds, NULL);

	// create dictionary
	createDLBTree(&dlbHead);

	// cache in-game graphics
	letterBank = SDL_LoadBMP("images/letterBank.bmp");
	smallLetterBank = SDL_LoadBMP("images/smallLetterBank.bmp");
	numberBank = SDL_LoadBMP("images/numberBank.bmp");

	rootWord = malloc(sizeof(char)*9);
	newGame(&head, dlbHead, screen, &letters);

	gameLoop(&head, dlbHead, screen, &letters);

	// tidy up and exit
	free(rootWord);
	Mix_CloseAudio();
	clearSoundBuffer(&soundCache);
	//trashDLBTree();
	destroyLetters(&letters);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(letterBank);
	SDL_FreeSurface(smallLetterBank);
	SDL_FreeSurface(numberBank);
	//SDL_Quit(); - no need to call this directly - we do it with the atexit() command
	return 0;
}
