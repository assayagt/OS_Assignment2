#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Create a new channel 
uint64 
sys_channel_create(void) { 
  return channel_create(); 
} 

// Put data into a channel 
uint64 
sys_channel_put(void) { 
  int cd, data; 
  argint(0, &cd);
  argint(1, &data); 
  return channel_put(cd, data); 
} 
  
// Take data from a channel 
uint64 
sys_channel_take(void) {
  int cd; 
  uint64 data; 
  argint(0, &cd);
  argaddr(1, &data); 
  int d; 
  if (channel_take(cd, &d) < 0) 
    return -1; 
  if (copyout(myproc()->pagetable, data, (char*)&d, sizeof(d)) < 0) 
    return -1; 
  return 0; 
} 
    
// Destroy a channel 
uint64 
sys_channel_destroy(void) {
  int cd; 
  argint(0, &cd);
  return channel_destroy(cd);
}  
