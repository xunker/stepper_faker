/*

Translate bipolar stepper commands to unipolar stepper board, like ULN2003.

On an atmega328 with three axis, all pins except serial TX and RX are are used
(including LED_BUILTIN).

Microstepping, enable and sleep are currently not supported because we're out
of pins.


Breadboarded atmega328 pins

RESET           1     (19/A5) 28  Stepper C IN4
RXD             2     (18/A4) 27  Stepper C IN3
TXD             3     (17/A3) 26  Stepper C IN2
Stepper A Dir   4 (2) (16/A2) 25  Stepper C IN1
Stepper A Step  5 (3) (15/A1) 24  Stepper C Step
Stepper A IN1   6 (4) (14/A0) 23  Stepper C Dir
VCC             7             22  GND
GND             8             21  AREF
OSC1            9             20  VCC
OSC2            10       (13) 19  Stepper B IN4
Stepper A IN2   11 (5)   (12) 18  Stepper B IN3
Stepper A IN3   12 (6)   (11) 17  Stepper B IN2
Stepper A IN4   13 (7)   (10) 16  Stepper B IN1
Stepper B Dir   14 (8)   (9)  15  Stepper B Step

Pins with atmega328 in etched adapter board

RESET           1     (19/A5) 28  Stepper A IN4
RXD             2     (18/A4) 27  Stepper A IN3
TXD             3     (17/A3) 26  Stepper A IN2
Stepper A Dir   4 (2) (16/A2) 25  Stepper A IN1
Stepper B Dir   5 (3) (15/A1) 24  Stepper B IN4
Stepper C Dir   6 (4) (14/A0) 23  Stepper B IN3
VCC             7             22  GND
GND             8             21  AREF
OSC1            9             20  VCC
OSC2            10       (13) 19  Stepper B IN2
Stepper A Step  11 (5)   (12) 18  Stepper B IN1
Stepper B Step  12 (6)   (11) 17  Stepper C IN4
Stepper C Step  13 (7)   (10) 16  Stepper C IN3
Stepper C IN1   14 (8)   (9)  15  Stepper C IN2


GRBL setting scratchpad (5v 28byj + uln2003 on 1mhz atmega328):
$0=200
$1=25
$20=1
$21=0
$22=1
$23=3
$27=2
$32=1
$100=92
$101=92
$102=2500
$110=250 (up to 350 with cooling, else skipped steps)
$111=ditto
$112=3 (up to 5, but may lose steps)
$130=124
$131=68

Travel Maximums:
X: 153.5mm
Y: 69mm
Z: 22mm

*/

#include <Stepper.h>
#include <PinChangeInterrupt.h>

/* Uncomment _one_ of the ones below to set the pin layout. BREADBOARDED_328
   divides the trigger pins between ports with theoretically will give slightly
   better preformance (but shouldn't make too much of a difference in real use.
   ADAPTER_328 groups the DIR and STEP pins together on sequential pins.
*/
// #define BREADBOARDED_328
#define ADAPTER_328

#define NUMBER_OF_STEPPERS 3
// There's no STEPPER_A define because it's assumed you'll have at leas one stepper
#define STEPPER_B // enable if NUMBER_OF_STEPPERS >= 2
#define STEPPER_C // enable if NUMBER_OF_STEPPERS >= 3

// Input pins
// Pull direction pin low to move "forward"
#ifdef ADAPTER_328
  #define STEPPER_A_DIR_PIN 2
#else
  #define STEPPER_A_DIR_PIN 2
#endif

#ifdef STEPPER_B
  #ifdef ADAPTER_328
    #define STEPPER_B_DIR_PIN 3
  #else
    #define STEPPER_B_DIR_PIN 8
  #endif
#endif

#ifdef STEPPER_C
  #ifdef ADAPTER_328
    #define STEPPER_C_DIR_PIN 4
  #else
    #define STEPPER_C_DIR_PIN A0
  #endif
#endif

// steps triggered when pin goes low
#ifdef ADAPTER_328
  #define STEPPER_A_STEP_PIN 5
#else
  #define STEPPER_A_STEP_PIN 3
#endif

#ifdef STEPPER_B
  #ifdef ADAPTER_328
    #define STEPPER_B_STEP_PIN 6
  #else
    #define STEPPER_B_STEP_PIN 9
  #endif
#endif

#ifdef STEPPER_C
  #ifdef ADAPTER_328
    #define STEPPER_C_STEP_PIN 7
  #else
    #define STEPPER_C_STEP_PIN A1
  #endif
#endif

/* Stepper library initialization */
/* STEPS_PER_REVOLUTION not technically needed since we're only moving one step at a time */
#define STEPS_PER_REVOLUTION 200

#ifdef ADAPTER_328
  Stepper stepperA(STEPS_PER_REVOLUTION, A2, A3, A4, A5);
#else
  Stepper stepperA(STEPS_PER_REVOLUTION, 4, 5, 6, 7);
#endif

#ifdef STEPPER_B
  #ifdef ADAPTER_328
    Stepper stepperB(STEPS_PER_REVOLUTION, 12, 13, A0, A1);
  #else
    Stepper stepperB(STEPS_PER_REVOLUTION, 10, 11, 12, 13);
  #endif
#endif

#ifdef STEPPER_C
  #ifdef ADAPTER_328
    Stepper stepperC(STEPS_PER_REVOLUTION, 8, 9, 10, 11);
  #else
    Stepper stepperC(STEPS_PER_REVOLUTION, A2, A3, A4, A5);
  #endif
#endif

// Used just for powering the steppers down when not in use
#ifdef ADAPTER_328
  const byte allStepperPins[] = {
    A2, A3, A4, A5,
  #ifdef STEPPER_B
    12, 13, A0, A1,
  #endif
  #ifdef STEPPER_C
    8, 9, 10, 11,
  #endif
  };
#else
  const byte allStepperPins[] = {
    4, 5, 6, 7,
  #ifdef STEPPER_B
    10, 11, 12, 13,
  #endif
  #ifdef STEPPER_C
    A2, A3, A4, A5,
  #endif
  };
#endif

Stepper* steppers[NUMBER_OF_STEPPERS] = {
  &stepperA,
#ifdef STEPPER_B
  &stepperB,
#endif
#ifdef STEPPER_C
  &stepperC,
#endif
};

const byte directionPins[NUMBER_OF_STEPPERS] = {
  STEPPER_A_DIR_PIN,
#ifdef STEPPER_B
  STEPPER_B_DIR_PIN,
#endif
#ifdef STEPPER_C
  STEPPER_C_DIR_PIN
#endif
};

const byte stepPins[NUMBER_OF_STEPPERS] = {
  STEPPER_A_STEP_PIN,
#ifdef STEPPER_B
  STEPPER_B_STEP_PIN,
#endif
#ifdef STEPPER_C
  STEPPER_C_STEP_PIN
#endif
};

// http://masteringarduino.blogspot.com/2013/10/fastest-and-smallest-digitalread-and.html
#define portOfPin(P)\
  (((P)>=0&&(P)<8)?&PORTD:(((P)>7&&(P)<14)?&PORTB:&PORTC))
#define ddrOfPin(P)\
  (((P)>=0&&(P)<8)?&DDRD:(((P)>7&&(P)<14)?&DDRB:&DDRC))
#define pinOfPin(P)\
  (((P)>=0&&(P)<8)?&PIND:(((P)>7&&(P)<14)?&PINB:&PINC))
#define pinIndex(P)((uint8_t)(P>13?P-14:P&7))
#define pinMask(P)((uint8_t)(1<<pinIndex(P)))

#define pinAsInput(P) *(ddrOfPin(P))&=~pinMask(P)
#define pinAsInputPullUp(P) *(ddrOfPin(P))&=~pinMask(P);digitalHigh(P)
#define pinAsOutput(P) *(ddrOfPin(P))|=pinMask(P)
#define digitalLow(P) *(portOfPin(P))&=~pinMask(P)
#define digitalHigh(P) *(portOfPin(P))|=pinMask(P)
#define isHigh(P)((*(pinOfPin(P))& pinMask(P))>0)
#define isLow(P)((*(pinOfPin(P))& pinMask(P))==0)
#define digitalState(P)((uint8_t)isHigh(P))

volatile signed int stepQueue[NUMBER_OF_STEPPERS];

boolean steppersPoweredDown = false;
unsigned int powerDownCounter = 0;
/* in cycles of loop(). On 1mhz atmega328 it's about 12 seconds. */
#define CHECK_FOR_POWER_DOWN_COUNTER_LIMIT 65534

void setup() {
  for(byte stepperNumber = 0; stepperNumber < NUMBER_OF_STEPPERS; stepperNumber++){
    pinMode(directionPins[stepperNumber], INPUT_PULLUP);
    pinMode(stepPins[stepperNumber], INPUT_PULLUP);

    /* 400 is good speed for 28byj steppers on uln2003 */
    steppers[stepperNumber]->setSpeed(400);

    stepQueue[stepperNumber] = 0;
  }

  attachPCINT(digitalPinToPCINT(STEPPER_A_STEP_PIN), readStepperA, FALLING);
  #ifdef STEPPER_B
    attachPCINT(digitalPinToPCINT(STEPPER_B_STEP_PIN), readStepperB, FALLING);
  #endif
  #ifdef STEPPER_C
    attachPCINT(digitalPinToPCINT(STEPPER_C_STEP_PIN), readStepperC, FALLING);
  #endif
}

void loop() {
  for(byte stepperNumber = 0; stepperNumber < NUMBER_OF_STEPPERS; stepperNumber++){
    if (stepQueue[stepperNumber] > 0) {
      powerDownCounter=0;
      steppers[stepperNumber]->step(1);
      stepQueue[stepperNumber]--;
    }

    if (stepQueue[stepperNumber] < 0) {
      powerDownCounter=0;
      steppers[stepperNumber]->step(-1);
      stepQueue[stepperNumber]++;
    }
  }

  if (powerDownCounter == 0) {
    steppersPoweredDown = false;
  }

  if (!steppersPoweredDown) {
    powerDownCounter++;

    if (powerDownCounter >= CHECK_FOR_POWER_DOWN_COUNTER_LIMIT) {
      powerDownSteppers();
    }
  }
}

void powerDownSteppers() {
  if (steppersPoweredDown)
    return;
  steppersPoweredDown = true;
  for(byte i = 0; i < sizeof(allStepperPins); i++){
    digitalLow(allStepperPins[i]);
  }
}

void readStepperA() {
  if (isLow(directionPins[0])) {
    stepQueue[0]++;
  } else {
    stepQueue[0]--;
  }
}

#ifdef STEPPER_B
  void readStepperB() {
    if (isLow(directionPins[1])) {
      stepQueue[1]++;
    } else {
      stepQueue[1]--;
    }
  }
#endif

#ifdef STEPPER_C
  void readStepperC() {
    if (isLow(directionPins[2])) {
      stepQueue[2]++;
    } else {
      stepQueue[2]--;
    }
  }
#endif
