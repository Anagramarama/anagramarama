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

/*
 * Count the length of the linked list.
 */

int
Length(struct node *head)
{
    struct node *current = head;
    int count = 0;
    for ( ; current; current = current->next) {
        ++count;
    }
    return count;
}

/*
 * swap the data content of two linked list nodes without changing
 * the position of the node within the list
 */

void
swap(struct node **from, struct node **to)
{
    char *word = (*from)->anagram;
    int len = (*from)->length;

    (*from)->anagram = (*to)->anagram;
    (*from)->length = (*to)->length;
    (*to)->anagram = word;
    (*to)->length = len;
}

/*
 * sort the list first alphabetically and then by increasing word length
 */

void
sort(struct node **headRef)
{
    struct node* left, *right;
    int completed = 0;

    while (!completed) {
        left = *headRef;
        right = left->next;
        completed = 1;
        for (; left && right; left = left->next, right = right->next) {
            if (strcmp(left->anagram, right->anagram) > 0) {
                swap(&left, &right);
                completed = 0;
            }
        }
    }

    completed = 0;
    while (!completed) {
        left = *headRef;
        right = left->next;
        completed = 1;
        for (; left && right; left = left->next, right = right->next) {
            if (left->length > right->length) {
                swap(&left, &right);
                completed = 0;
            }
        }
    }
}    

/*
 * Free the list and the allocated data in each node
 */

void
destroyAnswers(struct node** headRef)
{
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
void
push(struct node **headRef, const char *anagram)
{
    struct node * current = *headRef;
    int len;
    int duplicate = 0;

    /* walk the list first, so we can ignore duplicates...
     * this is probably slower than clearing duplicates at the end 
     * but simpler to write in the first instance
     */
    while (current != NULL) {
        if (!strcmp(anagram, current->anagram)){
            duplicate = 1;
            break;
        }
        current = current->next;
    }

    if (!duplicate) {
        struct node *newNode = malloc(sizeof(struct node));
        len = strlen(anagram);
        newNode->anagram = strdup(anagram);
        newNode->length = len;
        newNode->found = 0;
        newNode->guessed = 0;
        newNode->next = *headRef;

        *headRef = newNode;
    }
}

#ifdef UNIT_TEST
#include "unittest.h"

static int test_list_creation()
{
    struct node *head = NULL, *node = NULL;
    const char *words[] = {"one", "two", "three", "four", "five" };
    const char *ordrd[] = {"one", "two", "five", "four", "three" };
    size_t n, word_count = sizeof(words)/sizeof(words[0]);

    for (n = 0; n < word_count;++n) {
        push(&head, (char *)words[n]);
    }
    test_equals_int("check list length", 5, Length(head));
    
    for (node = head, n = word_count; node; --n, node = node->next) {
        if (strcmp(words[n-1], node->anagram) != 0)
            test_fail("incorrect node value");
        if (strlen(words[n-1]) != node->length)
            test_fail("incorrect node value length");
    }
    
    node = head->next;
    swap(&head, &node);
    test_equals_str("check swapped nodes", words[3], head->anagram);
    test_equals_str("check swapped nodes", words[4], head->next->anagram);

    sort(&head);

    for (node = head, n = 0; node; ++n, node = node->next) {
        if (strcmp(ordrd[n], node->anagram) != 0)
            test_fail("incorrect node value");
    }

    destroyAnswers(&head);
    test_equals_ptr("check list destroyed", head, NULL);
    return 0;
}

struct unit_test_t unit_tests[] = {
    { NULL, test_list_creation, NULL}
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
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
