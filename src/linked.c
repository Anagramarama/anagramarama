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

#include <stdlib.h>
#include <string.h>
#include "linked.h"

/***********************************************************/
int Length(struct node* head){

struct node* current = head;
int count = 0;

        while (current != NULL){
                //printf("%s\n", current->anagram);
                count++;
                current = current->next;
        }

        return count;
}

/***********************************************************/
void swap(struct node** from, struct node** to){
// swaps the contents of 2 linked list nodes
// doesn't disturb the pointers

char* swap;

	swap = malloc(sizeof((*from)->anagram));
	strcpy(swap, (*from)->anagram);
	(*from)->anagram = (*to)->anagram;
	(*from)->length = (*to)->length;
	(*to)->anagram = swap;
	(*to)->length = strlen(swap);

}

/***********************************************************/
void sort(struct node** headRef){
// sort the linked list alpha/num of chars
struct node* current = *headRef;
struct node* next = malloc(sizeof(struct node));
int done = 0;
int swaps = 0;

	// walk the list
	while (!done){
		while (current !=NULL){
			next = current->next;
			if (next != NULL){
				//printf("%s, %s - %i\n", next->anagram, current->anagram, strcmp(next->anagram, current->anagram));
				if (strcmp(next->anagram, current->anagram)<0){
					swap(&next, &current);
					swaps++;
				}
			}
			current = current->next;
		}
		if (!swaps){
			done = 1;
		}
		else{
			swaps = 0;
			current = *headRef;
		}
	}

	done = 0;
	current = *headRef;
	swaps = 0;

	// walk the list
	while (!done){
		while (current !=NULL){
			next = current->next;
			if (next != NULL){
				//printf("%s, %s \n", next->anagram, current->anagram);
				if (strlen(next->anagram) < strlen(current->anagram)){
					swap(&next, &current);
					swaps++;
				}
			}
			current = current->next;
		}
		if (!swaps){
			done = 1;
		}
		else{
			swaps = 0;
			current = *headRef;
		}
	}

	free(next);
}

/***********************************************************/
void destroyAnswers(struct node** headRef){
// destroy the whole answers list

struct node* current = *headRef;
struct node* previous = *headRef;

	while (current != NULL){
		free(current->anagram);
		previous = current;
		current = current->next;
		free(previous);
		previous = NULL;
	}

	*headRef = NULL;
}



/***********************************************************/
void push(struct node** headRef, char* anagram){

struct node* newNode = malloc(sizeof(struct node));
int len;
struct node* current = *headRef;
int duplicate = 0;

	// walk the list first, so we can ignore duplicates...
	// this is probably slower than clearing duplicates at the end 
	// but simpler to write in the first instance
	while (current != NULL){
		if (!strcmp(anagram, current->anagram)){
			duplicate = 1;
			break;
		}
		current = current->next;
	}

	if (!duplicate){
	        len = strlen(anagram+1);
	        newNode->anagram = malloc(sizeof(char)*(len + 1));
	        strcpy(newNode->anagram, anagram);
	        newNode->length = len + 1;
	        newNode->found = 0;
		newNode->guessed = 0;
	        newNode->next = *headRef;  // dereference back the the real head pointer

	        *headRef = newNode;        // ditto when replacing it with the new one
	}
}
