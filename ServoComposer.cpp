#include "ServoComposer.h"

/* ServoComposer Ver 0.1
Copyright (C) 2015 Michael Wolf

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

ServoComposer provides the ability to:
  create servo state events which trigger other servo movement or user defined callbacks
  normalize midpoints/directions/movements/safe min-max among sets of servos.
  have speed control
  use external timers / time domains (for speed control)
  group servos and control N groups with a single event
  set up and change between several sets of triggers
  rapid prototyping on linux 
  
Notes:
  All movements are relative to midpoint
  One object should be defined per action
  Group targets are only intra group
  Programattically generated actions are unlikely so event sets aren't modifiable/deletable
  Servos give no feedback, hence event triggering can't be exact.
  
  Currently not interrupt safe, meaning:
    no del(if I write it) while event checking.

Future:

  moverequest or start/stop not int safe - so use...
  Mutex using: ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    http://jhaskellsblog.blogspot.com/2011/11/demonstration-atomic-access-and.html
  Include direction in trigger
  Event delete if needed
  Sort event list
*/
bool ServoComposer::clearEventsTrigger = false;
bool ServoComposer::servoStartTrigger = false;
bool ServoComposer::endOfTick = true;

ServoComposer::ServoComposer (void) {
  // this->objListAdd(this);  // enable if you need tock service
  // userEventIdx = ServoComposer::numServos;
  
  for (int idx=0; idx < ServoComposer::numServos; idx++) {  // init all the servos
    SC_DBG_PRINT("Init servo #: ");
    SC_DBG_PRINTLN(idx);
    ServoComposer::servos[idx].arduinoServoObj.attach(
      ServoComposer::servos[idx].pin, ServoComposer::servos[idx].min, ServoComposer::servos[idx].max);
    servos[idx].reqMove = -1;
    servos[idx].evBegin = NULL;
  }
  
  ServoComposer::servoTickInit();
}

// move all servos to midpoints
bool ServoComposer::moveToMid (void) {
  bool rv;
  for (int idx=0; idx < ServoComposer::numServos; idx++) {
    rv = ServoComposer::moveRequest (idx, 0, 1); // move to mid ASAP
    if (!rv) {
      SC_DBG_PRINT("moveToMid: Failed for servo #: ");
      SC_DBG_PRINTLN(idx);
      return (false);
    }
  }
  return (true);
}

// convert relative degree to abs for a given servo
int ServoComposer::toAbsDegree (int idx, int deg) {
  deg = deg* -1;
  deg = deg + ServoComposer::servos[idx].mid;
  return (deg);
}

// check servo triggers only so far
void ServoComposer::checkTriggers (int idx, int start, int end) {
  ServoComposer_event *curr;
  
  curr = ServoComposer::servos[idx].evBegin;
  
  // assuming not sorted to avoid a bug where already added event memory is reused thus unsorting the list
  // TODO: upon insert, autodelete entries w same event* as you.
  while(curr != NULL) {  

    // specify for this servo
    int td = toAbsDegree (idx, curr->triggerDegree);
    
    if (start > end) {  
      int swap = start;
      start = end;
      end = swap;
    }
    
    if (td > start && td <= end) {  // if triggered
      if ((curr->req.cr.sr_idx & SC_GROUP_FILTER) != INVALID_SERVO) { // move a servo
        int sr_idx = curr->req.sr.sr_idx;
        if (sr_idx & SC_GROUP) {  // change servo idx relative to group
          sr_idx = ServoComposer::servos[idx].group[sr_idx & SC_GROUP_FILTER];
        }
        ServoComposer::moveRequest (sr_idx, curr->req.sr.req, curr->req.sr.ticks);
      } else // call an external function
        curr->req.cr.handler (idx, td, curr->req.cr.cookie);
    }
    curr = curr->next;
  }
}

// Called x times/sec to update servo movement.  This is the main mechanism behind speed control
void ServoComposer::moveService (void) {  

  for (int idx=0; idx < ServoComposer::numServos; idx++) {
    int curr = ServoComposer::servos[idx].arduinoServoObj.read(); // absolute current degree
    int req = ServoComposer::servos[idx].reqMove;
    if (req != -1 && req != curr) {
      int step;
         
      if (ServoComposer::servos[idx].ticks < 1) {
        step = req;  // if no more time, just make em equal
      } else {       // otherwise, move a fraction of total request
        step = ((req - curr) / ServoComposer::servos[idx].ticks) + curr;  
        SC_DBG_PRINT("INFO: Servo ticks left: ");
        SC_DBG_PRINTLN(ServoComposer::servos[idx].ticks);
        ServoComposer::servos[idx].ticks--;
      }
      
      ServoComposer::servos[idx].arduinoServoObj.write(step);
      ServoComposer::checkTriggers (idx, curr, step);
    }
  }
}


// Sanity check values and start movement
bool ServoComposer::moveRequest (int idx, int req, int ticks) {

  req = ServoComposer::toAbsDegree (idx, req);
  
  int min = ServoComposer::servos[idx].min;
  int max = ServoComposer::servos[idx].max;
  if (min > max) {  
      int swap = min;
      min = max;
      max = swap;
  }
  
  if (req < min || req > max) {
    SC_DBG_PRINT("WARNING: Servo degree requested out of range for index: ");
    SC_DBG_PRINT(req);
    SC_DBG_PRINT("/");
    SC_DBG_PRINTLN(idx);
    return (false);
  }
  
  if (idx < -1 || idx >= ServoComposer::numServos) {
    SC_DBG_PRINT("WARNING: Servo index out of range: ");
    SC_DBG_PRINTLN(idx);
    return (false);
  }

  ServoComposer::servos[idx].reqMove = req;
  ServoComposer::servos[idx].ticks = ticks;
  return (true);
}

#if 0
// The idea was to give you access to object specific 'this'
// Deleted for lack of use. May re-enable.

ServoComposer* ServoComposer::objList = NULL;  // enable if you need tock service

// This is the back half of ServoComposer::servoTick callback.
// This hacks around the annoying C++ prohibition of non-static callbacks fcns.
void ServoComposer::objListTock(void){
  
  ServoComposer *curr;
  
  curr = ServoComposer::objList;
  
  while (curr != NULL) {
    SC_DBG_PRINT("Object at: ");
    SC_DBG_PRINTLN_PTR(curr);
    curr = curr->next;
  }
}

void ServoComposer::objListAdd(ServoComposer *s){
  ServoComposer *prev, *curr;
    
  prev = curr = ServoComposer::objList;
  if (curr == NULL) {  // if inserted at top of list
    s->next = curr;
    ServoComposer::objList = s;
    return;
  }
  
  curr = curr->next;
  while(true) {
    if (curr == NULL){  // if somewhere inside the list
      prev->next = s;
      s->next = curr;
      return;
    }
    prev = prev->next;
    curr = curr->next;
  }
}
#endif

// Timer callback
void ServoComposer::servoTick (void) {

  if (ServoComposer::clearEventsTrigger) {  
    for (int idx=0; idx < ServoComposer::numServos; idx++) {
      ServoComposer::servos[idx].evBegin = NULL;
    }
    ServoComposer::clearEventsTrigger = false;
  }
  
  // if moveService is running, will only block further calls
  if (ServoComposer::servoStartTrigger) {  
    ServoComposer::moveService(); // update all servos
  }
//  objListTock();
  ServoComposer::userTick(NULL);
  ServoComposer::endOfTick = true;
}

// If you want to be informed, you can register a callback here.
void ServoComposer::registerUserTickHandler (void (*f)(void)) {
  if (f == NULL) {
    SC_DBG_PRINTLN("Registered NULL UserTick handler.");
    return;
  }

  ServoComposer::userTick(f);
}

// Calls a user function on Tick if registered.
void ServoComposer::userTick(void (*f)(void)) {
  static void (*handler)(void) = NULL;

  if (f != NULL)
    handler = f;
  else
    handler();  
}

// Clear added events for all servos
void ServoComposer::clearEvents (void) {
  ServoComposer::clearEventsTrigger = true;
  
  // could do a callback here but wanted to make it simple
  while (ServoComposer::clearEventsTrigger) { // block till cleared
    continue;
  }
}

// Start movement
void ServoComposer::servosStart (void) {
  ServoComposer::servoStartTrigger = true;
}

// Pause movement
void ServoComposer::servosPause (void) {
  ServoComposer::servoStartTrigger = false;
  ServoComposer::endOfTick = false;
  
  while (not ServoComposer::endOfTick) { // block till cleared
    continue;
  }
}

// Add a new trigger event to servo
ServoComposer_event *ServoComposer::addEvent (ServoComposer_event *ev, int idx) {
  ServoComposer_event *prev, *curr;
  
  if (idx >= INVALID_SERVO)
    return (NULL);
    
  prev = curr = ServoComposer::servos[idx].evBegin;
  /* if (idx < ServoComposer::numServos)
    prev = curr = ServoComposer::servos[idx].evBegin;
  else
    prev = curr = userEvBegin; */
    
  if (curr == NULL) {  // if inserted at top of list
    ev->next = curr;
    ServoComposer::servos[idx].evBegin = ev;
    /* if (idx < ServoComposer::numServos)
      ServoComposer::servos[idx].evBegin = ev;
    else
      userEvBegin = ev; */

    return (ev);
  }
  
  curr = curr->next;
  while(true) {  // TODO: insert sorted so we dont have to brute force events from now on
    if (curr == NULL){  // if somewhere inside the list
      prev->next = ev;
      ev->next = curr;
      return (ev);
    }
    prev = prev->next;
    curr = curr->next;
  }
}

// Add a new trigger event to group of servos
ServoComposer_event *ServoComposer::addGroupEvent (ServoComposer_event *ev, int idx, int *members) {
  int numGroups = members[0];
  int numServos = members[1];
  int *mptr = &(members[2]);
  
  for (int gidx=0; gidx < numGroups*numServos; gidx+=numServos) {
    this->addEvent (ev, mptr[gidx+idx]);  //TODO check retval
  }
  
  return (ev);
}

// For each servo, attach list of all servos in group
void ServoComposer::setGroup (int *members) {
  int numGroups = members[0];
  int numServos = members[1];
  int *mptr = &(members[2]);
  
  // give each servo a list of members in its group
  for (int idx=0; idx<numGroups*numServos; idx++) {
    ServoComposer::servos[idx].group = &(mptr[(idx/numServos)*(numServos)]);
  }
}

// Listen for event (servo degree being hit) and initiate another servo movement
ServoComposer_event *ServoComposer::eventListen (ServoComposer_event *ev, int idx, int deg, int sr_idx, int req, int ticks, int *members) {

  // make target servo index + ServoComposer::numServos
  // when you hit an index > deal with as a relative index
  ev->triggerDegree = deg; 
  ev->req.sr.sr_idx = members==NULL ? sr_idx : sr_idx + SC_GROUP;
  ev->req.sr.req = req;
  ev->req.sr.ticks = ticks;
  
  if (members==NULL)
    return (this->addEvent (ev, idx));
  else
    return (this->addGroupEvent (ev, idx, members));
}

// Listen for event (servo degree being hit) and call a handler
ServoComposer_event *ServoComposer::eventListen (ServoComposer_event *ev, int idx, int deg, void (*handler)(int, int, void *), void *cookie, int *members) {
  
  ev->triggerDegree = deg;
  ev->req.cr.sr_idx = members==NULL ? INVALID_SERVO : INVALID_SERVO + SC_GROUP;  // this is how we tell which part of union to use
  ev->req.cr.handler = handler;
  ev->req.cr.cookie = cookie;
  
  if (members==NULL)
    return (this->addEvent (ev, idx));
  else
    return (this->addGroupEvent (ev, idx, members));
}

