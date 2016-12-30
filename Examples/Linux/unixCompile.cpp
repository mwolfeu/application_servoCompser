#include "unixCompile.h"
#include "ServoComposer.h"

#ifndef SC_NO_TIMER_INIT
void timerHandler (int sig, siginfo_t *si, void *uc) {
  ServoComposer::servoTick();
}

timer_t timer;

void ServoComposer::servoTickInit (void) { // wow... >20 lines for a friggin timer. just. wow.
  static const char *name = "posix timer";
  timer_t *timerID = &timer;
  int expireMS = 100; // fire every .1 sec for debug purposes
  int intervalMS = 100;
  
  static struct sigevent         te;
  static struct itimerspec       its;
  static struct sigaction        sa;
  int sigNo = SIGUSR1;

  // Set up signal handler.
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = timerHandler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(sigNo, &sa, NULL) == -1) {
    SC_DBG_PRINTLN("Error: setup failed\n");
    exit(0);
  }

  // Set up alarm
  te.sigev_notify = SIGEV_SIGNAL;
  te.sigev_signo = sigNo;
  te.sigev_value.sival_ptr = timerID;
  timer_create(CLOCK_REALTIME, &te, timerID);

  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = intervalMS * 1000000;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = expireMS * 1000000;
  timer_settime(*timerID, 0, &its, NULL);
}

#define TICKS_TO_SLEEP 20  // assuming 10 ticks a sec = 2 sec
void my_sleep (void) {
  int counter = 0;
  while (counter++ < TICKS_TO_SLEEP)
    pause(); //wait for signal
}
#endif

void nothing(void){
  return;
}
void nothing1(int x){
  return;
}

//ersatz servo methods for linux

// as above but also sets min and max values for writes.
char Servo::attach(int pin, int min, int max){
  sv_pin = pin;
  sv_min = min;
  sv_max = max;
  cout << "ATTACH: Servo pin: " << sv_pin << "\n";
}

// if this is a memory leak for linux (in comparison w arduino) I don't yet care
void Servo::detach() {
  cout << "DETACH: Servo pin: " << sv_pin << "\n";
}

// if value is < 200 its treated as an angle, otherwise as pulse width in microseconds 
void Servo::write(int value) {
  cout << "WROTE: Servo pin: " << sv_pin << " Absolute Value: " << value << "\n";
  sv_current = value;
}

// returns current pulse width as an angle between 0 and 180 degrees
int Servo::read(void) {
  cout << "READ:  Servo pin: " << sv_pin << " Absolute Value: " << sv_current << "\n";
  return (sv_current);
}  
   
int main (void) {

  setup ();
  
  while (true) {
    my_sleep();
    loop();
    my_sleep(); 
  }

}
