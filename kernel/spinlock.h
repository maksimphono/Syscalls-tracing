// Mutual exclusion lock.
#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

void initlock(struct spinlock *lk, char *name);

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void acquire(struct spinlock *lk);
// Release the lock.
void release(struct spinlock *lk);

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int holding(struct spinlock *lk);

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void push_off(void);

void pop_off(void);

#endif
