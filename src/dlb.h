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

struct dlb_node{

	char letter;
	int valid;
	struct dlb_node* sibling;
	struct dlb_node* child;
};

struct dlb_node* dlb_insertLetter(char thisLetter);
void dlb_push(struct dlb_node** dlbHead, char* thisWord);
void createDLBTree(struct dlb_node** dlbHead,char* language);
int dlb_lookup(struct dlb_node* dlbHead, char* thisWord);
