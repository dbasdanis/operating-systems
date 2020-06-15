/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define NEWTASKSLICE (NS_TO_JIFFIES(100000000))

#define GOODNESS_WITH_WAITINGINRQ      //define if you want to use the waitining in running queue time in the goodness calculation 
//#define GOODNESS_WITHOUT_WAITINGINRQ    

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *			 There are HZ jiffies in a second, (HZ is 
 *			 declared in macros.h), and is usually
 *			 1 or 10 milliseconds.
 */
extern long long jiffies;
extern struct task_struct *idle;

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
 /* initscheduler
  * Sets up and allocates memory for the scheduler, as well
  * as sets initial values. This function should also
  * set the initial effective priority for the "seed" task 
  * and enqueu it in the scheduler.
  * INPUT:
  * newrq - A pointer to an allocated rq to assign to your
  *			local rq.
  * seedTask - A pointer to a task to seed the scheduler and start
  * the simulation.
  */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask)
{
	seedTask->next = seedTask->prev = seedTask;
	newrq->head = seedTask;
	newrq->nr_running++;
}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule()
{
	//printf("Kill schedule called.\n");	
	return;
}


void print_rq () {
	struct task_struct *curr;
	
	printf("Rq: \n");
	curr = rq->head;
	if (curr)
		printf("%p(%Lf)", curr,curr->exp_burst);
	while(curr->next != rq->head) {
		curr = curr->next;
		printf(", %p(%Lf)", curr,curr->exp_burst);
	};
	printf("\n");
}

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task in the queue
 */
void schedule()
{
	static struct task_struct *nxt = NULL;
	struct task_struct *curr, *temp;
	long double min_burst, old_last_busrt, old_exp_burst, old_last_run_time; 
	long double max_WaitinigInRQ;
	long double goodness;
	long double min_goodness;
	
//	printf("In schedule\n");
//	print_rq();
	
	current->need_reschedule = 0; /* Always make sure to reset that, in case *
								   * we entered the scheduler because current*
								   * had requested so by setting this flag   */
								   
	old_last_busrt = current->last_burst;                  //keep the old values 
	old_exp_burst = current->exp_burst;
	old_last_run_time = current->last_run_time;
	
	current->last_burst = sched_clock() - current->start_running_time;
	current->exp_burst = (current->last_burst + 0.5*current->exp_burst)/1.5;
	if(sched_clock() > current->last_run_time)
		current->last_run_time = sched_clock();	

	if (rq->nr_running == 1) {
		context_switch(rq->head);
		//nxt = rq->head->next;
	}
	else {	
		//printf("\nIn schedule\n");
		//print_rq();
		
		temp = rq->head->next;                        //SJF algorithm - find the process with the minimum expected burst
		min_burst = temp->exp_burst;
		curr = temp;
		temp = temp->next;
		while(temp!=rq->head){
			if(temp->exp_burst < min_burst){
				min_burst = temp->exp_burst;
				curr = temp;
			}
			temp = temp->next;
		}

#ifdef GOODNESS_WITH_WAITINGINRQ

		temp = rq->head->next;                                                  //find the process with the maximum time waiting in the running queue
		max_WaitinigInRQ = sched_clock() - temp->last_run_time;
		temp = temp->next;
		while(temp!=rq->head){
			if((sched_clock() - temp->last_run_time) > max_WaitinigInRQ){
				max_WaitinigInRQ = sched_clock() - temp->last_run_time;
			}
			temp = temp->next;
		}
		
//goodness calculation using the expected burst and the time waiting in the running queue
		temp = rq->head->next;                                    
		min_goodness = ((1+temp->exp_burst)/(1+min_burst))*((1+max_WaitinigInRQ)/(1+sched_clock()-(temp->last_run_time)));
		curr = temp;
		temp = temp->next;
		while(temp!=rq->head){
			goodness = ((1+temp->exp_burst)/(1+min_burst))*((1+max_WaitinigInRQ)/(1+sched_clock()-(temp->last_run_time)));
			if(goodness < min_goodness){
				min_goodness = goodness;
				curr = temp;
			}
			temp = temp->next;
		}
#endif

//goodness calculation without using the time waiting in the running queue
#ifdef GOODNESS_WITHOUT_WAITINGINRQ
		temp = rq->head->next;                                    
		min_goodness = ((1+temp->exp_burst)/(1+min_burst));
		curr = temp;
		temp = temp->next;
		while(temp!=rq->head){
			goodness = ((1+temp->exp_burst)/(1+min_burst));
			if(goodness < min_goodness){
				min_goodness = goodness;
				curr = temp;
			}
			temp = temp->next;
		}
#endif
		
		if(current==curr){                                      //keep the old values if the same process gets the cpu again
				current->last_burst = old_last_busrt;
				current->exp_burst = old_exp_burst;
				current->last_run_time = old_last_run_time;
		}
		else{
			curr->start_running_time = sched_clock();
			
		}
		context_switch(curr);
	}
}


/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void sched_fork(struct task_struct *p)
{
	p->time_slice = 100;
	
	p->last_burst = 0;
	p->exp_burst = 0;
	p->last_run_time = 0;
	p->start_running_time = 0;
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{
	schedule();
}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 */
void wake_up_new_task(struct task_struct *p)
{	
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;
	
	if(sched_clock() > p->last_run_time)
		p->last_run_time = sched_clock();
	
	rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;

	if(sched_clock() > p->last_run_time)
		p->last_run_time = sched_clock();
	
	rq->nr_running++;
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = p->prev = NULL; /* Make sure to set them to NULL *
							   * next is checked in cpu.c      */

	rq->nr_running--;
}
