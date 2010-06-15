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
#include <SDL/SDL.h>
#include "sprite.h"

/********************************************************************/
void setBackground(SDL_Surface** screen, struct sprite** movie){

SDL_Rect rect;

	rect.x = (*movie)->x;
	rect.y = (*movie)->y;
	rect.w = (*movie)->w;
	rect.h = (*movie)->h;

	// copy the background into the replace buffer
	SDL_BlitSurface(*screen, &rect, (*movie)->replace, NULL);
}

/********************************************************************/
void showSprite(SDL_Surface** screen, struct sprite** movie){

SDL_Rect rect;

	rect.x = (*movie)->x;
	rect.y = (*movie)->y;
	rect.w = (*movie)->w;
	rect.h = (*movie)->h;

	// display the image in new location
	SDL_SetColorKey((*movie)->sprite, SDL_SRCCOLORKEY, SDL_MapRGB((*movie)->sprite->format,255,0,255));
	SDL_BlitSurface((*movie)->sprite, NULL, *screen, &rect);
}

/********************************************************************/
void resetBackground(SDL_Surface** screen, struct sprite** movie){

SDL_Rect rect;

	rect.x = (*movie)->x;
	rect.y = (*movie)->y;
	rect.w = (*movie)->w;
	rect.h = (*movie)->h;

	// put the screen back the way it was
	SDL_SetColorKey((*movie)->replace, SDL_SRCCOLORKEY, SDL_MapRGB((*movie)->replace->format,255,0,255));
	SDL_BlitSurface((*movie)->replace, NULL, *screen, &rect);
}

/********************************************************************/
static int
isSpriteMoving(struct sprite *p)
{
    /* returns true if this sprite needs to move */
	return (p->y != p->toY) ||  (p->x != p->toX);
}
int
anySpritesMoving(struct sprite **letters)
{
    struct sprite *current;
    for (current = *letters; current != NULL; current = current->next) {
        if (isSpriteMoving(current))
            return 1;
    }
    return 0;
}

/********************************************************************/
void moveSprite(SDL_Surface** screen, struct sprite** movie, int letterSpeed){

int i;
int x, y;
int Xsteps;

	// move a sprite from it's curent location to the new location
	if( (  (*movie)->y != (*movie)->toY )  ||  (   (*movie)->x != (*movie)->toX )   ){

		x = (*movie)->toX - (*movie)->x;
		y = (*movie)->toY - (*movie)->y;
		if (y){
			if (x<0) x *= -1;
			if (y<0) y *= -1;
			Xsteps = (x / y) * letterSpeed;
		}
		else{
			Xsteps = letterSpeed;
		}

		for (i = 0; i<Xsteps; i++){
			if((*movie)->x < (*movie)->toX){
				(*movie)->x++;
			}
			if((*movie)->x > (*movie)->toX){
				(*movie)->x--;
			}
		}

		for (i=0;i<letterSpeed; i++){
			if((*movie)->y < (*movie)->toY){
				(*movie)->y++;
			}
			if((*movie)->y > (*movie)->toY){
				(*movie)->y--;
			}
		}
	}

	setBackground(&(*screen), &(*movie));
}

/********************************************************************/
void moveSprites(SDL_Surface** screen, struct sprite** letters, int letterSpeed){

struct sprite* current;

	current= *letters;
	while(current!=NULL){
		moveSprite(&(*screen), &current, letterSpeed);
		current = current->next;
	}
	current = *letters;
	while(current!=NULL){
		showSprite(&(*screen), &current);
		current=current->next;
	}
	SDL_Flip(*screen);
	current = *letters;
	while(current!=NULL){
		resetBackground(&(*screen), &current);
		current= current->next;
	}
}

/********************************************************************/
void destroyLetters(struct sprite** letters){
struct sprite* current = *letters;
struct sprite* previous = NULL;

	while(current!=NULL){
		SDL_FreeSurface(current->sprite);
		SDL_FreeSurface(current->replace);
		previous = current;
		current = current->next;
		free(previous);
	}

	(*letters)=NULL;

}
