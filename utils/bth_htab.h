// MIT License
// 
// Copyright (c) 2025 bobthehuge
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to 
// deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.

#ifndef BTH_HTAB_H
#define BTH_HTAB_H

#include <stdint.h>
#include <stdlib.h>

struct bth_htab_pair
{
    uint32_t hkey;
    char *key;
    void *value;
    struct bth_htab_pair *next;
};

struct bth_htab
{
    size_t capacity;
    size_t size;
    struct bth_htab_pair *data;
};

uint32_t one_at_a_time(char *key);
uint32_t djb2(char *key);

#ifndef BTH_HTAB_HASH
#define BTH_HTAB_HASH(key) djb2(key)
#endif

// Create a new empty hash table.
// The initial capacity is 4.
// The initial size is 0.
// If there is not enough memory, the program prints
// "Not enough memory!" and exits with the error code 1.
// (Use the errx() function of the standard library.)
// Be careful, you have to allocate two memory spaces.
// - The memory space that holds the 'struct htab' variable.
// - The memory space that holds the data.
//   All cells of the 'data' array must be initialized to zero
//   (they contain the sentinels of the linked lists.)
struct bth_htab *bth_htab_new();

// Delete all the pairs of a hash table.
// Free the memory allocated by the pairs.
// The 'data' array is not freed.
// The table's capacity does not change.
// The table's size is set to zero.
// After this function, the hash table can still be used.
void bth_htab_clear(struct bth_htab *ht);

// Delete a hash table.
// Free all the allocated memory.
// After this function, the hash table can no longer be used.
void bth_htab_free(struct bth_htab *ht);

// Return a pair of the hash table from its key.
// (The pair is not removed from the hash table.)
// If the pair is not in the table, return NULL.
struct bth_htab_pair *bth_htab_get(struct bth_htab *ht, char *key);

// Finds the pair's index in the bucket list and assign
// it to `idx_ptr` location if not NULL. 
// Returns the found pair.
// If the pair is not in the table, return NULL.
struct bth_htab_pair *bth_htab_find(
    struct bth_htab *ht, char *key, size_t *idx);

// Insert a pair into the hash table.
// If the pair is already in the table, return 0.
// Otherwise:
// - Insert the pair in the hash table.
// - Increment the size if the pair has been placed into an empty cell.
// - If the ratio (size / capacity) is larger than 75%,
//   double the capacity of the hash table.
// - Return 1
int bth_htab_insert(struct bth_htab *ht, char *key, void *value);

// Remove a pair from the table.
// The pair is also freed.
// After this function, the pair can no longer be used.
// The size is updated.
void bth_htab_remove(struct bth_htab *ht, char *key);

#endif
