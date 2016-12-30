#ifdef SC_DBG_UNIX
#include "unixCompile.h"
#endif

#include <Servo.h>

#if defined(SC_DBG)
#define SC_DBG_PRINT(x)       Serial.print(x)
#define SC_DBG_PRINTLN(x)     Serial.println(x)
#define SC_DBG_PRINT_PTR(x)   Serial.print(reinterpret_cast<unsigned long>(x), HEX)
#define SC_DBG_PRINTLN_PTR(x) Serial.println(reinterpret_cast<unsigned long>(x), HEX)

#elif defined(SC_DBG_UNIX)
#define SC_DBG_PRINT(x)       cout << x
#define SC_DBG_PRINTLN(x)     cout << x << '\n'
#define SC_DBG_PRINT_PTR(x)   cout << x
#define SC_DBG_PRINTLN_PTR(x) cout << x << '\n'

#else
#define SC_DBG_PRINT(x)
#define SC_DBG_PRINTLN(x)
#define SC_DBG_PRINT_PTR(x)
#define SC_DBG_PRINTLN_PTR(x)
#endif

#define SC_END INVALID_SERVO
#define SC_GROUP 0xFF00
#define SC_GROUP_FILTER 0xFF
#define SC_NUM_SERVOS(x) sizeof(x)/sizeof(ServoComposer_servo)

// ServoComposer event message specifying trigger and action taken once triggered
typedef struct SERVO_EVENT {
  int triggerDegree;
  
  union {
    struct SERVO_REQUEST {
      int sr_idx;
      int req; // always as absolute degree
      int ticks; 
    } sr;
    struct CALLBACK_REQUEST {
      int sr_idx; // if INVALID_SERVO then use this part of the union
      void (*handler)(int, int, void *);
      void *cookie;
    } cr;
  } req;

  struct SERVO_EVENT *next;
} ServoComposer_event;

// basic state for describing a servo
typedef struct SERVO {
  int pin;
  int mid;
  int min;
  int max;
  bool fli;
  /* below here - internal use only */
  int reqMove; // always as absolute degree
  int ticks;
  int *group;
  Servo arduinoServoObj;
  ServoComposer_event *evBegin;
} ServoComposer_servo;

class ServoComposer {
  private:
    static ServoComposer_servo servos[];

    static int toAbsDegree (int idx, int deg);
    static void moveService (void);
    static void checkTriggers (int idx, int start, int end);
    
    static bool clearEventsTrigger;
    static bool servoStartTrigger;
    static bool endOfTick;
    
    static void userTick(void (*f)(void));
    
    ServoComposer_event *addEvent (ServoComposer_event *ev, int idx);
    ServoComposer_event *addGroupEvent (ServoComposer_event *ev, int idx, int *members);
    // static void objListTock (void);
    
  public:
    virtual void servoTickInit (void); // late (runtime) binding therefore overridable
    static int numServos;
    
    static void servoTick (void);  // public if you want to hook up your own timer
    static bool moveRequest (int idx, int req, int ticks);
    static void registerUserTickHandler(void (*f)(void));
    
    ServoComposer (void);
    void timerStart (void);
    bool moveToMid (void);
    void setGroup (int *members);
    
    ServoComposer_event *eventListen (ServoComposer_event *ev, int idx, int deg, int sr_idx, int req, int ticks, int *members = NULL);
    ServoComposer_event *eventListen (ServoComposer_event *ev, int idx, int deg, void (*handler)(int, int, void *), void *cookie, int *members = NULL);
    
    void clearEvents (void);
    void servosStart (void);
    void servosPause (void);
};
