#define INVALID_SERVO         255

//ersatz servo class for linux
class Servo {
  private:
    int sv_pin;
    int sv_min;
    int sv_max;
    int sv_current;
  public:
    char attach(int pin, int min, int max); // as above but also sets min and max values for writes.
    void detach();
    void write(int value);     // if value is < 200 its treated as an angle, otherwise as pulse width in microseconds 
    int read();                // returns current pulse width as an angle between 0 and 180 degrees
};

