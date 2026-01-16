#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "refcnt.h"

#define MAX_REFCNT ((PGROUNDUP(PHYSTOP) - KERNBASE)/PGSIZE)

refcnt_t refcnt;

// Function to initialize the reference counter
void refcnt_init() {
    for (uint32 i = 0; i < MAX_REFCNT; i++) {
        refcnt.count[i] = 0;  // Initialize the reference count to 0 for all pages
    }
    initlock(&refcnt.lock, "refcnt");  // Initialize the spinlock
}

uint64 page_index(uint64 pa){
    return ((pa - KERNBASE)/PGSIZE);
}
int inc_ref(uint64 page_idx){
    int value = -1;
    acquire(&refcnt.lock);
    if (0 <= page_idx && page_idx < MAX_REFCNT) {
        refcnt.count[page_idx] += 1;
        value = refcnt.count[page_idx];
    } else
        value = -1; // error
    release(&refcnt.lock);

    return value;
}

int dec_ref(uint64 page_idx){
    int value = -1;
    acquire(&refcnt.lock);
    if (0 <= page_idx && page_idx < MAX_REFCNT) {
        refcnt.count[page_idx] -= 1;
        value = refcnt.count[page_idx];
    } else
        value = -1; // error
    release(&refcnt.lock);

    return value;
}

void set_ref(uint64 page_idx, int value) {
    acquire(&refcnt.lock);
    if (0 <= page_idx && page_idx < MAX_REFCNT)
        refcnt.count[page_idx] = value;
    release(&refcnt.lock);
}

int get_ref(uint64 page_idx) {
    int value = -1;
    acquire(&refcnt.lock);
    if (0 <= page_idx && page_idx < MAX_REFCNT)
        value = refcnt.count[page_idx];
    release(&refcnt.lock);

    return value;
}