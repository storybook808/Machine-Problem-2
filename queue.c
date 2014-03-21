/*
 * This program simulates a single queue with space for one customer.
 */

#include <stdio.h>
#include <stdlib.h>

#define CAPACITY  6 // Maximum number of customers in coffee shop
                    // But this program doesn't use this definition
                    // The coffee shop has capacity 1. But you should
                    // use this in your modification.
#define MAXSERVER 2 // Maximum number of servers.
#define ACTIVE    0 // Customer currently being serviced.
#define NEXT      1 // Next customer in line. 

//#define DEBUG     // Uncomment to activite debug lines.
//#define SCRIPT    // Uncomment to activite event text.
#define TRUE      1
#define FALSE     0

typedef struct item{ // Customer data
   int arrival;      // Arrival time
   int service;      // Service time
   struct item *next;
   } Item;

typedef struct serverdata{  // Server data
   int busy;                // Whether server is occupied
   int count;               // Amount of customer within the queue.
   int full;                // Whether server is filled.
   int departure[CAPACITY]; // Departure time.  Valid only if busy = 1
   int arrival[CAPACITY];   // Arrival time of customer into the queue
   } Serverdata;

typedef struct stats { // Statistics measured by the simulation
   int arrived;     // Total # customers that arrived (blocked & accepted)
   int totaldelay;  // Sum of delays of all accepted customers 
   int blocked;     // Total # customers that were blocked
   int accepted;    // Total # customers that were accepted 
   } Stats;

Item * getcustomers(void);

main(void)
{

   Stats      simstats;          // Statistics of the simulation
   Serverdata server[MAXSERVER]; // The server data
   Item       *arrivalhead;      // Points to the list of arriving customer
   Item       *olditem;          // Old list element to discard
   int        currtime;          // Current time in the simulation
   int        serviceDepart;     // Departure time
   double     x, y;              // Temp variables.
   int        i, j;              // More temp variables.
   int        numServer;         // Number of servers working this shift.
   int        masterBusy;        // Busy flag for the coffee shop as a whole.
   int        masterFull;        // Full flag for the coffee shop as a whole.

   // Get customers
   arrivalhead = getcustomers();

   if (arrivalhead==NULL){ 
      printf("No arriving customers.\n");
      return;
   }

   // Prompt for the number of servers.
   do { // Check if the numbe of servers are within valid range.
      printf("Enter the number of server (1 - %d): ", MAXSERVER);
      scanf("%d", &numServer);
   } while(numServer<1||numServer>MAXSERVER);

   // Initialize
   simstats.arrived = 0;
   simstats.totaldelay = 0; 
   simstats.blocked = 0;
   simstats.accepted = 0;       // This was missing from Sasaki's original program.
   currtime=0;                  // Current time
   masterBusy = FALSE;          // Shop starts not busy nor full.
   masterFull = FALSE;

   for(i=0; i<numServer; i++) {
      server[i].count = 0;      // Server starts empty.
      server[i].busy  = FALSE;  // Server starts not busy.
      server[i].full  = FALSE;  // Server starts empty.
   }

   // Keep the program going if there are still customers within the queue even
   // if the arrival list is empty

   while (masterBusy==TRUE||arrivalhead!=NULL) {

      // Process arriving customers, which arrive at the beginning of
      // the time slot.
   
      if (arrivalhead!=NULL) {  // Check the next arrival
         if (arrivalhead->arrival==currtime) { // An arrival occurred
            simstats.arrived++; // update statistics
            
            // Check to see if any of the queues are currenty open for a customer.
            if (!masterFull) { // Customer is accepted into the server

	       // Customer will join the line with the least amount of people.
	       // Target the first line as the short line. When there is another line
	       // that is short, replace the current target with the new target.
	       // Keep in mind that this is all under the assumption that one of the
	       // lines isn't full.
               j = 0;
	       for(i=1; i<numServer; i++)
                  if(server[i].count<server[j].count)
                     j = i; // j is the target line for the customer.

               // Add customer to server and update the state of the server
               server[j].arrival[server[j].count] = arrivalhead->arrival;
               
               // If there are customers lined up already, then the next patron
               // will have to wait their service time after the patron in front
               // of them leaves.
               
               // Used to shorten the lines below.
               i = arrivalhead->service-1;
               
               if(server[j].count>0) // Another patron is in line already.
                  serviceDepart = server[j].departure[server[j].count-1]+i;
               else // First in line.
                  serviceDepart = currtime+i;
               
               // Update customer departure time information in the queue.
               server[j].departure[server[j].count] = serviceDepart;
               
               // Update busy flag, because a customer has entered the line. If
	       // the flag was already busy, then nothing will change to the flag.
               server[j].busy = TRUE;
               
               // Updated the number of patrons within the line.
               server[j].count++;

	       #ifdef SCRIPT
	          printf("Customer lines up to server %d\n", j);
	       #endif

	       // Update full flag for current server if needed.
	       if(server[j].count==CAPACITY)
	          server[j].full = TRUE;

	       // Update the master full flag.
	       // First, masterFull will be set equal to the first server's full flag.
	       // Next, the masterFull will be ANDed to every other server's full flag.
	       // Only when all of the flags are full, then the masterFull will also be full.
	       // Otherwise, masterFull will be FALSE.
	       masterFull = server[0].full;
	       for(i=0; i<numServer; i++)
	          masterFull = masterFull&server[i].full;
               
               // Update statistics
               simstats.accepted++;
            }

            else { // Customer is blocked 
               // Update statistics
               simstats.blocked++;

	       #ifdef SCRIPT
                  printf("Customer walks away sad! Oh no!\n");
	       #endif
            }

            // Delete arrived customer from the list
            olditem = arrivalhead;
            arrivalhead = arrivalhead->next;
            free(olditem);
         }
      }

      // Process any departures which occur at the end of the time slot.
      // Keeps track of the customers within the queue and shifts patrons
      // forward in an orderly fashion. This will also scroll through each
      // server and check to see if their active patron is ready to depart.
      for(j=0; j<numServer; j++) {
         if(server[j].busy==1) { // Make sure something can depart
            if(server[j].departure[ACTIVE]==currtime) { // Customer will depart
               simstats.totaldelay += (currtime-server[j].arrival[ACTIVE])+1;
                              // The right hand side is the customer's delay
	    
	       #ifdef DEBUG
                  printf("Total Delay: %d\n", simstats.totaldelay);
               #endif

	       // Shift customers forward in line for service.
               for(i=NEXT; i<server[j].count; i++) {
                  server[j].departure[i-1] = server[j].departure[i];
                  server[j].arrival[i-1] = server[j].arrival[i];
               }
            
               // Reduce the number of people in the line.
               server[j].count--;

	       // Update server full flag. If any customer leaves the line, then
	       // we know that the line cannot be full.
	       server[j].full = FALSE;

	       #ifdef SCRIPT
	          printf("Customer leaves the shop happy from server %d\n", j);
	       #endif
            
               // If there are no customers in line, then the server must not be
	       // busy.
               if(server[j].count==0)
                  server[j].busy = FALSE;
            }
         }
      } // End of for-loop.

      // Update masterBusy flag. Keeps through all of the servers to see if any
      // of them are still processing customers.
      masterBusy = server[0].busy;
      for(i=1; i<numServer; i++) {
         masterBusy = masterBusy&server[i].busy;
      }

      // Update masterFull flag. This is outside of the for loop since it would
      // be more efficient to process the full flags after handling all of the
      // departures.
      masterFull = server[0].full;
      for(i=1; i<numServer; i++)
         masterFull = masterFull&server[i].full;

      currtime++;  // Increment current time
   } // end of while-loop

   printf("Statistics:\n");
   printf("Number of customers that arrived = %d\n",simstats.arrived);
   printf("Number of customers blocked= %d\n",simstats.blocked);
   x = simstats.blocked;
   y = simstats.arrived;

   #ifdef DEBUG
      printf("Arrive vs. blocked\n");
      printf("x: %f\ny: %f\n", x, y);
   #endif

   printf("Fraction of customers blocked = %f\n",x/y);
   x = simstats.totaldelay;
   y = simstats.accepted;

   #ifdef DEBUG
      printf("Time delay\n");
      printf("x: %f\ny: %f\n", x, y);
      printf("Total Delay Final: %d\n", simstats.totaldelay);
      printf("Accepted Customers: %d\n", simstats.accepted);
   
      printf("Number of servers selected: %d\n", numServer);
   #endif

   if (y > 0) 
      printf("Average delay of accepted customers = %.1f\n",x/y);
   else printf("No accepted customers\n");
}

//
// Queries user for a data file of customer information
// Reads the data per customer and puts it in a queue (linked list)
// that it creates.  It returns a pointer to the head of
// the queue.  If the queue is empty, it returns NULL 
// 
Item * getcustomers(void)
{
FILE *fp;
char filename[200];
int i;
int numcust;   // Number of customers
int arrival;   // Arriving time of customer
int service;   // Service time of customer
Item *head;    // Head of the queue
Item *tail;    // Tail of queue
Item *newitem;

printf("Enter file with arriving customer data: ");
scanf("%s",filename);
fp = fopen(filename,"r");
if (fp==NULL) {
   printf("File '%s' not found or can't be opened\n",filename);
   return(NULL); 
   }
fscanf(fp,"%d",&numcust);
// printf("Number of arriving customers = %d\n",numcust);

// Fill queue with customer data
head=tail=NULL;  // Initialize queue
for (i=0; i<numcust; i++) {
   // Load a new customer and create an item for it
   fscanf(fp,"%d %d",&arrival,&service);
   newitem = (Item*) malloc(sizeof(Item));
   newitem->arrival = arrival;
   newitem->service = service;
   newitem->next = NULL;
   // Insert into queue
   if (head==NULL) { // Insert into empty queue
      head=newitem;
      tail=newitem;
      }
   else {
      tail->next = newitem; // Insert into nonempty queue at tail
      tail=newitem;
      }
//   printf("Customer %i: %d %d\n",i,tail->arrival,tail->service);
//   Above is used to debug -- check if everything was loaded properly
   }

fclose(fp);

if (head==NULL) printf("No customers in the data file\n");
return(head);
}
