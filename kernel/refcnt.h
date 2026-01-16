#ifndef _REFCNT_H_
#define _REFCNT_H_

#include "spinlock.h"

// Reference counter structure for physical pages
typedef struct {
    struct spinlock lock;  
    int count[(PGROUNDUP(PHYSTOP) - KERNBASE)/PGSIZE];
} refcnt_t;

extern refcnt_t refcnt; 

void refcnt_init();
uint64 page_index(uint64 pa);
int inc_ref(uint64 page_idx);
int dec_ref(uint64 page_idx);

void set_ref(uint64 page_idx, int value);

int get_ref(uint64 page_idx);

#endif // REFCNT_H
