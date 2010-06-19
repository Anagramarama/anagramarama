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
#include <stdio.h>
#include <assert.h>
#include "dlb.h"

/*
 * create and return a new letter node
 */

static struct dlb_node *
dlb_create_node(char c)
{
    struct dlb_node *node = malloc(sizeof(struct dlb_node));
    node->letter = c;
    node->valid = 0;
    node->sibling = NULL;
    node->child = NULL;
    return node;
}

static int
dlb_free_node(struct dlb_node *node)
{
    memset(node, 0, sizeof(struct dlb_node));
    free(node);
    return 0;
}

void
dlb_walk(struct dlb_node *node, dlb_node_operation op)
{
    while (node) {
        struct dlb_node *tmpnode = node;
        if (node->child) {
            dlb_walk(node->child, op);
        }
        node = node->sibling;
        op(tmpnode);
    }
}

void
dlb_free(struct dlb_node *head)
{
    dlb_walk(head, dlb_free_node);
}

/*
 * add a new word to the dictionary
 */

static void
dlb_push(struct dlb_node **dlbHead, const char *word)
{
    struct dlb_node* current = *dlbHead;
    struct dlb_node* previous = NULL;
    const char *p;
    int child = 0;
    int sibling = 0;
    int newHead = (*dlbHead==NULL);

    for (p = word; p && *p; ) {
		char letter = *p;

		if (current == NULL) {
			current = dlb_create_node(letter);
			if (newHead) {
				*dlbHead = current;
				newHead = 0;
			}
			if (child) {
				previous->child = current;
			}
			if (sibling){
				previous->sibling = current;
			}
		}

		child = 0;
		sibling = 0;
		previous = current;

		if (letter == previous->letter) {
            ++p;
			child = 1;
			current = previous->child;
		} else {
			sibling = 1;
			current = previous->sibling;
		}
	}
	previous->valid = 1;
}

/*
 * open the wordlist file and push all words onto the dictionary
 * tree. Returns false if the file could not be opened.
 */

int
dlb_create(struct dlb_node **dlbHead, const char *filename)
{
    int r = 0;
    char buffer[64];
	FILE *fp = fopen(filename, "r");
    if (fp) {
        while (fscanf(fp, "%63s", buffer) != EOF){
            dlb_push(dlbHead, buffer);
        }
        fclose(fp);
        r = 1;
    }
    return r;
}

/*
 * determine if a given word is in the dictionary essentially the same
 * as a push, but doesn't add any of the new letters
 */

int
dlb_lookup(struct dlb_node *head, const char *word)
{
    struct dlb_node *current = head;
    struct dlb_node *previous = NULL;
    const char *p;
    int retval = 0;
    
    for (p = word; p && *p; ) {
		char letter = *p;
		if (current == NULL) {
			retval = 0;
			break;
		}

		previous = current;

		if (letter == previous->letter) {
			++p;
			current = previous->child;
			retval = previous->valid;
		} else {
			current = previous->sibling;
			retval = 0;
		}
	}
	return retval;
}

#ifdef UNIT_TEST
#include "unittest.h"

static int test_dlb()
{
    struct dlb_node *head = NULL;
    test_equals_int("check code for bad file", 0,
                    dlb_create(&head, "src/garbage_bad_file"));
    test_equals_int("check code for valid file", 1,
                    dlb_create(&head, "i18n/en_GB/wordlist.txt"));
    test_equals_int("check lookup", 1, dlb_lookup(head, "anagram"));
    test_equals_int("check lookup", 1, dlb_lookup(head, "zygote"));
    dlb_free(head);
    return 0;
}

struct unit_test_t unit_tests[] = {
    { NULL, test_dlb, NULL }
};

int
main(void)
{
    run_tests(unit_tests);
    return 0;
}

#endif /* UNIT_TEST */

/*
 * Local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * End:
 */
