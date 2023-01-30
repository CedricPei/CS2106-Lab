/*************************************
* Lab 1 Exercise 2
* Name:
* Student No:
* Lab Group:
*************************************/

#include "node.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit (although you do not need to)


// Traverses list and returns the sum of the data values
// of every node in the list.
long sum_list(list *lst) {


}

// Traverses list and returns the number of data values in the list.
int list_len(list *lst) {


}



// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {



}

// Deletes node at index (counting from head starting from 0).
// Note: index is guarenteed to be valid.

void delete_node_at(list *lst, int index) {



}

// Search list by the given element.
// If element not present, return -1 else return the index. If lst is empty return -2.

int search_list(list *lst, int element) {




}

// Reverses the list with the last node becoming the first node.
void reverse_list(list *lst) {



}

// Resets list to an empty state (no nodes) and frees
// any allocated memory in the process
void reset_list(list *lst) {


}



// Traverses list and applies func on data values of
// all elements in the list.
void map(list *lst, int (*func)(int)) {


}
	

