#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "refcnt.h"

refcnt_t refcnt;

// Function to initialize the reference counter
void refcnt_init() {
    for (int i = 0; i < (PGROUNDUP(PHYSTOP) - KERNBASE)/PGSIZE; i++) {
        refcnt.count[i] = 0;  // Initialize the reference count to 0 for all pages
    }
    initlock(&refcnt.lock, "refcnt");  // Initialize the spinlock
}

uint64 page_index(uint64 pa){
    return ((PGROUNDUP(pa) - KERNBASE)/PGSIZE);
}
void inc_ref(uint64 page_idx){
    acquire(&refcnt.lock);
    refcnt.count[page_idx] += 1;
    release(&refcnt.lock);
}

void dec_ref(uint64 page_idx){
    acquire(&refcnt.lock);
    refcnt.count[page_idx] -= 1;
    release(&refcnt.lock);
}

void set_ref(uint64 page_idx, int value) {
    acquire(&refcnt.lock);
    refcnt.count[page_idx] = value;
    release(&refcnt.lock);
}