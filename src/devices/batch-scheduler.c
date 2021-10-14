/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h" //generate random numbers

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1

// new varibles
struct semaphore bus_inuse;
struct semaphore bus_timeslot;
struct semaphore high_prio;
struct semaphore low_prio;
struct semaphore number_lock;
struct semaphore wakeup_lock;
struct semaphore bus_sender;
struct semaphore bus_receiver;

struct semaphore counter_lock;

int bus_direction;
int high_num;
int sender_direction;
int receiver_direction;
/*
 *	initialize task with direction and priority
 *	call o
 * */
typedef struct {
	int direction;
	int priority;
} task_t;


void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);


void oneTask(task_t task);/*Task requires to use the bus and executes methods below*/
	void getSlot(task_t task); /* task tries to use slot on the bus */
	void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
	void leaveSlot(task_t task); /* task release the slot */



/* initializes semaphores */ 
void init_bus(void){ 
 
    random_init((unsigned int)123456789); 
    sema_init(&bus_inuse, 1);
    sema_init(&bus_timeslot, BUS_CAPACITY);
    sema_init(&high_prio, 1);
    sema_init(&low_prio, 1);
    sema_init(&number_lock, 1);
    sema_init(&wakeup_lock, 1);
    sema_init(&bus_sender, 1);
    sema_init(&bus_receiver, 1);
    sema_init(&counter_lock, 1);
    
    bus_direction = -1;
    high_num = 0;
    sender_direction = 0;
    receiver_direction = 0;
    //msg("NOT IMPLEMENTED");
    /* FIXME implement */

}

/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread. 
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive)
{
    unsigned int i;
    /* create sender threads */
    for(i = 0; i < num_tasks_send; i++)
        thread_create("sender_task", 1, senderTask, NULL);

    /* create receiver threads */
    for(i = 0; i < num_task_receive; i++)
        thread_create("receiver_task", 1, receiverTask, NULL);

    /* create high priority sender threads */
    for(i = 0; i < num_priority_send; i++)
       thread_create("prio_sender_task", 1, senderPriorityTask, NULL);

    /* create high priority receiver threads */
    for(i = 0; i < num_priority_receive; i++)
       thread_create("prio_receiver_task", 1, receiverPriorityTask, NULL);
}

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED){
        task_t task = {SENDER, NORMAL};
        oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED){
        task_t task = {SENDER, HIGH};
        oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED){
        task_t task = {RECEIVER, NORMAL};
        oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED){
        task_t task = {RECEIVER, HIGH};
        oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task) {
  getSlot(task);
  transferData(task);
  leaveSlot(task);
}


/* task tries to get slot on the bus subsystem */
void getSlot(task_t task) 
{
  if (task.direction != bus_direction){
    printf("change");
    sema_down(&bus_inuse);
    if(task.direction == SENDER) {
      sema_down(&bus_sender);
      sema_down(&bus_receiver);
      bus_direction = SENDER;
      sender_direction++;
    } else {
      sema_down(&bus_receiver);
      sema_down(&bus_sender);
      bus_direction = RECEIVER;
      receiver_direction++;
    }
    sema_up(&bus_inuse);
  } else if(task.direction == SENDER){

    sema_down(&counter_lock);
    sender_direction++;
    sema_up(&counter_lock);
  } else if(task.direction == RECEIVER){
    sema_down(&counter_lock);
    receiver_direction++;
    sema_up(&counter_lock);
  }

  if(task.priority == HIGH) {
    sema_down(&high_prio);
    if(high_num == 0){
      sema_down(&low_prio);
    }
    high_num++;
    sema_down(&bus_timeslot);
    sema_up(&high_prio);
  } else {
    sema_down(&low_prio);
    sema_down(&bus_timeslot);
    sema_up(&low_prio);
  }

  /*
  if(task.direction != bus_direction){
    sema_down(&wakeup_lock);
    other_direction++;
    sema_up(&wakeup_lock);

    sema_down(&bus_inuse);

    sema_down(&number_lock);
    if(same_direction == 0){
      bus_direction = task.direction;
      sema_down(&wakeup_lock);
      for(int i = 0; i < other_direction-2; i++){
        sema_up(&bus_inuse);
        
      }
      other_direction = 0;
      sema_down(&wakeup_lock);
    }
    sema_up(&number_lock);
  }
  sema_down(&number_lock);
  same_direction++;
  sema_up(&number_lock);
  // number of same direction task

  if(task.priority == HIGH){
    // disable low prio task
    sema_down(&high_prio);
    if(high_num == 0){
      sema_down(&low_prio);
    }
    high_num++;
    sema_up(&high_prio);
    sema_down(&bus_timeslot);
  } else {
    sema_down(&low_prio);
    sema_down(&bus_timeslot);
    sema_up(&low_prio);
  }
  */
}

void changea(int* a){
  *a += 1;
  *a *= 2;
}
/* task processes data on the bus send/receive */
void transferData(task_t task) 
{
    /* FIXME implement */
  for(int i = 0;i<125;i++){
    int a;
    changea(&a);
  }
}

/* task releases the slot */
void leaveSlot(task_t task) 
{
  if(task.priority == HIGH){
    sema_down(&high_prio);
    high_num--;
    if(high_num == 0){
      sema_up(&low_prio);
    }
    sema_up(&high_prio);
  }

  sema_down(&counter_lock);
  if(task.direction == SENDER){
    sender_direction--;
  } else {
    receiver_direction--;
  }
  printf("%d %d %d",task.direction,task.priority,sender_direction);
  printf(" %d\n", receiver_direction);
  sema_up(&counter_lock);
  if(task.direction == SENDER && sender_direction == 0){
    sema_up(&bus_sender);
    sema_up(&bus_receiver);
  } else if(task.direction ==RECEIVER && receiver_direction ==0){
    sema_up(&bus_receiver);
    sema_up(&bus_sender);
  }

  sema_up(&bus_timeslot);
    /* FIXME implement 
  if(task.priority == HIGH){
    sema_down(&high_prio);
    high_num--;
    if(high_num == 0){
      sema_up(&low_prio);
    }
    sema_up(&high_prio);
  }
  sema_up(&bus_timeslot);
  sema_down(&number_lock);
  printf("%d ", task.priority);
  printf("%d %d\n",task.direction, same_direction);
  same_direction--;

  if(same_direction == 0 && bus_timeslot.value == BUS_CAPACITY)
    sema_up(&bus_inuse);
  sema_up(&number_lock);
    */
}
