#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "refcnt.h"

volatile static int started = 0;

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

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    procinit();      // process table
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    virtio_disk_init(); // emulated hard disk
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
