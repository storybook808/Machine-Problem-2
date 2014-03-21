/*  To compile, type gcc trafficgen.c
 *  To run, type ./a.out > traffic.dat
 *  which will direct the output to a file
 *  "traffic.dat"
 */

#include<stdlib.h>
#include<stdio.h>

#define MAXTIME 10800
#define ARRIVALRATE 0.0086
#define FREQ_1 0.4
#define FREQ_2 0.3
#define FREQ_3 0.1
#define FREQ_4 0.2
#define DURATION_1 30
#define DURATION_2 120
#define DURATION_3 90
#define DURATION_4 180

typedef struct item{
   int atime;   // Arrival time of customer
   int stime;   // Service time
   struct item *next;  // Pointer to next item
   } Item;

int arrival(double p);
int service(void);

main()
{
int numcust=0;  // Number of customers
int time;
int i;
Item *head;     // Head of linked list
Item *tail;     // Tail of linked list
Item *newitem;  // Pointer to a new item in linked list 
Item *curritem; // Current item

// Initialize
srand48(3);     // Seed the pseudorandom number generator
head = NULL;
tail = NULL;

// Generate the new customers and store them in a
// linked list
for (time=0; time<MAXTIME; time++) {
   if (arrival(ARRIVALRATE) == 1) {
      // Load arrival time and service time for new customer
      newitem = (Item *)malloc(sizeof(Item));
      newitem->atime = time; // Arrival time is current time
      newitem->stime = service();
      newitem->next = NULL;
      // Update the head of the list if it is empty
      if (numcust==0) {
         head = newitem;
         tail = newitem;
         }
      else { // Update the tail of the queue
         tail->next = newitem;
         tail = newitem;
         }
      // Update the number of customers
      numcust++;
      }
   }
// Print out the times
printf("%d\n",numcust);
curritem = head;
for (i=0; i<numcust; i++) {
   printf("%d %d\n",curritem->atime, curritem->stime);
   free(curritem);  // free space of customer already displayed
   curritem=curritem->next;
   }
}

int arrival(double p)
{ if (drand48() < p) return 1;
  else return 0;
}

int service(void)
{
double x;
x = drand48();
if (x < FREQ_1) return(DURATION_1);
else if (x < FREQ_1+FREQ_2) return(DURATION_2);
else if (x < FREQ_1+FREQ_2+FREQ_3) return(DURATION_3);
else return(DURATION_4);
}


