#include "ServoComposer.h"

// Of the form: Pin#, Midpoint, Min, Max, FLI (increasing number == moving to front or left)
ServoComposer_servo ServoComposer::servos[] = {
  {2, 73, 8, 108, true},    {3, 73, 43, 113, true},  {4, 76, 116, 46, false},
  {5, 76, 156, 46, false},  {6, 80, 50, 120, true},  {7, 72, 32, 102, true},
  {8, 76, 7, 111, true},    {9, 67, 97, 27, false},  {10, 60, 80, 30, false},
  {11, 85, 180, 46, false}, {12, 57, 87, 17, false}, {44, 60, 20, 90, true}
};

int ServoComposer::numServos = SC_NUM_SERVOS(ServoComposer::servos);

/* TODO
tap into timer

writing configer
compile on both / osx too
example dir
*/

void handler (int idx, int deg, void *x) {
  cout << "got event callback!\n";
  exit (0);
}

void tickHandler(void) {
  cout << "In user tick handler!\n";
}

// Either use the class directly like this:
//ServoComposer walk;

// or derive a class (example below) to override servoTickInit using your own timer
// Timers are responsible for calling ServoComposer::servoTick() 50x/sec;
class SC_W_NewTimer : public ServoComposer {
  public:
    void servoTickInit (void) { ServoComposer::servoTickInit (); };
};

SC_W_NewTimer walk;

#define NUM_LEGS 4
#define NUM_LEG_SERVOS 3
// first two elements are Number of groups in array, and number of elements per group
// use index into ServoComposer::servos not pin numbers!
int legs[] = {NUM_LEGS, NUM_LEG_SERVOS, 0,1,2, 3,4,5, 6,7,8, 9,10,11}; 

// Allocate these however you wish
ServoComposer_event ev0, ev1, ev2, ev3;

void setup() {

  // Move all servos to midpoints ASAP
  walk.moveToMid();
  
  // Singleton Example: 0 in each case is an absolute index into the servo array!
  walk.eventListen (&ev0, 0,  10, 0, -15, 10);
  walk.eventListen (&ev1, 0, -10, 0,  15, 10);
  
  // Callback Example: Note: Exits program so you can see printed text.
  //walk.eventListen (&ev2, 0, 10, handler, NULL);
  
  // Group Example: 0 in each case refers to the 1st element of each group!
  // The following will therefore move 4 servos.
  //walk.setGroup(legs);
  //walk.eventListen (&ev0, 0,  10, 0, -15, 10, legs);
  //walk.eventListen (&ev1, 0, -10, 0,  15, 10, legs);
  
  // Direct servo movement example.
  //walk.moveRequest (0, 15, 10);
 
  // Start the show
  walk.servosStart();
  
  // if you want to know when ticks happen register a void(*) void handler here.
  //walk.registerUserTickHandler(tickHandler);
  
#if 0 // Example where all events above are cleared and new ones added

  walk.clearEvents();
  
  walk.servosPause();  // must pause movement before adding events
  walk.eventListen (&ev0, 0,  10, 0, -15, 10);
  walk.eventListen (&ev1, 0, -10, 0,  15, 10);
  walk.servosStart();  // restart after adding is done
  walk.moveRequest (0, 15, 10);  // trigger first event
#endif 

}

// the loop routine runs over and over again forever:
void loop() {
}

