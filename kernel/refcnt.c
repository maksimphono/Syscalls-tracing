#include "refcnt.h"

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