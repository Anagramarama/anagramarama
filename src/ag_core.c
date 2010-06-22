/*
 * This file contains all the anagram handling code so that the
 * validity of the anagram management can be tested independently of
 * the SDL graphical management code.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dlb.h"
#include "linked.h"
#include "ag.h"

extern char txt[50];
extern char language[64];

/***********************************************************
synopsis: determine the next blank space in a string 
	  blanks are indicated by pound not space
	  
inputs:   pointer the string to check

outputs:  returns position of next blank (1 is first character)
	  or 0 if no blanks found
***********************************************************/
int
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
void
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

/* checkGuess - needs to return a sound to play */

/***********************************************************
synopsis: spin the word file to a random location and then
	  loop until a 7 or 8 letter word is found
	  this is quite a weak way to get a random word
	  considering we've got a nice dbl Dictionary to
	  hand - but it works for now.

inputs: n/a

outputs: a random word
***********************************************************/
void
getRandomWord(char *output, size_t length)
{
    FILE *fp;
    struct stat statbuf;
    char buffer[64];
    
    assert(length == 9);

	strcpy(txt,language);
    strcat(txt,"wordlist.txt");
	fp = fopen(txt,"r");

    stat(txt, &statbuf);
    fseek(fp, (rand() % statbuf.st_size), SEEK_SET);
    if (fscanf(fp, "%63s", buffer) == EOF)
        fseek(fp, 0, SEEK_SET);

	/* ok random location reached */
	while (strlen(buffer) != 7) {
        if (fscanf(fp, "%63s", buffer) == EOF) {
            /* go back to the start of the file */
            int i;
            fseek(fp, 0L, SEEK_SET);
            i = fscanf(fp, "%63s", buffer);
            assert(i != EOF);
		}
	}
	
	fclose(fp);

	/* add in our space character */
    strcpy(output, buffer);
    strcat(output, " ");
	return;
}

#ifdef UNIT_TEST
#include "unittest.h"

char txt[50];
char language[64];

static int test_nextBlank(void *cd)
{
    char a[7] = {'a','b',SPACE_CHAR,'c',SPACE_CHAR,'d', 0};
    test_equals_int("nextBlank", 3, nextBlank(a));
    test_equals_int("nextBlank substr", 2, nextBlank(a+3));
    test_equals_int("nextBlank no-blanks", 0, nextBlank("abcdef"));
    test_equals_int("nextBlank zero-length", 0, nextBlank(""));
    return 0;
}

static int test_shiftLeftKill(void *cd)
{
    char a[7] = { 'a','b','c','d','e','f', 0 };
    char *s = shiftLeftKill(a);
    test_equals_str("shiftLeftKill", "bcdef", s);
    free(s);
    s = shiftLeftKill("abcdef");
    test_equals_str("shiftLeftKill const str", "bcdef", s);
    free(s);
    return 0;
}

static int test_shiftLeft(void *cd)
{
    char a[7] = { 'a','b','c','d','e','f', 0 };
    char b[2] = { 'a', 0 };
    test_equals_str("shiftLeft string", "bcdefa", shiftLeft(a));
    test_equals_str("shiftLeft short string", "a", shiftLeft(b));
    return 0;
}

static int test_getRandomWord()
{
    char buffer[9];
    strcpy(language, "i18n/en_GB/");
    getRandomWord(buffer, sizeof(buffer));
    test_equals_int("randword last char is space", ' ', buffer[7]);
    return 0;
}

static int test_anagram(void *cd)
{
    char lexicon[255];
    char buffer[9];
    int n, count = 0;
    struct dlb_node *tree = NULL;
    struct node *list = NULL;

    strcpy(language, "i18n/en_GB/");
    for (n = 0; n < 10; ++n) {
        getRandomWord(buffer, sizeof(buffer));
        if (strlen(buffer) == 8) ++count;
    }
    test_equals_int("check random words", 10, count);

    strcpy(lexicon, language);
    strcat(lexicon, "wordlist.txt");
    dlb_create(&tree, lexicon);
    ag(&list, tree, "", "zipper ");
    test_equals_int("check anagrams of anagram", 14, Length(list));
    destroyAnswers(&list);
    dlb_free(tree);

    return 0;
}

struct unit_test_t unit_tests[] = {
    {NULL, test_shiftLeftKill, NULL},
    {NULL, test_shiftLeft, NULL},
    {NULL, test_nextBlank, NULL},
    {NULL, test_getRandomWord, NULL},
    {NULL, test_anagram, NULL}
};

int
main(void)
{
    run_tests(unit_tests);
    return 0;
}
#endif /* UNIT_TEST */
