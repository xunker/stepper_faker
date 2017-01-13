Stepper-Faker
=============

Arduino-based converter that lets unipolar steppers (e.g. 28BYJ-48 w/ ULN2003A)
pretend to be bipolar steppers for things like GRBL or Marlin. Built to work
with any Arduino-compatible atmega168/atmega328 board (ex: Arduino Uno or
Adafruit Pro Trinket). Also includes schematic for a custom carrier board that
makes connecting the steppers a snap!

Provides interface pins that work the same as the
[EasyDriver](http://www.schmalzhaus.com/EasyDriver/)
or [DRV8825/A4988 StepStick](https://www.pololu.com/product/1182) and can be
directly connected to a micro-controller running GRBL using the
[standard wiring](https://github.com/gnea/grbl/wiki/Connecting-Grbl).

Up to 3 steppers can be driven from one atmega328. Can be 5V or 12v, because
the original driver (e.g. ULN2003A) is still used.

The code is easily portable to other AVRs. Requires compatibility with the
[PinChangeInterrupt](https://github.com/NicoHood/PinChangeInterrupt) library and
the [Stepper class](https://www.arduino.cc/en/Reference/Stepper) included with
the Arduino IDE. Also requires that the micro-controller have at least 6 digital
pins available for each stepper (2-in, 4-out).

Will automatically power-down steppers after they have been idle for a period of
time. Does not yet support the SLEEP or ENABLE pins, but the RESET pin can be
used to a similar same effect.

Does not support micro-stepping, but may in the future.

### Wiring

General wiring requirement: connect the ground from your controller running
Stepper-Faker to the ground on the primary micro-controller running your
grbl/marlin/etc software.

If your primary controller has enough power to run your Stepper-Faker board and
all the steppers and your steppers are 5v, feel free to use that; otherwise you
will need to power those separately using the appropriate power source.

#### Stepper-Faker to steppers

Connect the "IN1/2/3/4" pins for each Stepper (A, B, or C) to the IN1/2/3/4 pins
on the included ULN2003A driver board. *Remember* that Stepper-Faker already
reverses the IN2 and IN3 pins, so you don't need to.

If your Stepper-Faker controller has enough power for all your steppers and they
are 5v, feel free to use that; otherwise you will need to power those separately
using the appropriate power source.

#### Stepper-Faker to GRBL

Follow the same [wiring directions as with EasyDriver](https://github.com/gnea/grbl/wiki/Connecting-Grbl#method-one-easydriver-v44).

#### Stepper-Faker as EasyDriver

Leave SLP, ENABLE, and RESET un-connected. Connect the the appropriate STEP and
DIR pins on your Stepper-Faker controller to the correct pins on your primary
controller running your software.

#### Stepper-Faker as DRV8825/A4988 StepStick

Leave SLEEP and RESET un-connected. Connect the the appropriate STEP and
DIR pins on your Stepper-Faker controller to the correct pins on your primary
controller running your software.

### Changes

Jan 13, 2016 - Add readme, update pins.

Jan 12, 2016 - Initial Commit

### License

GNU GPLv3

### Source and Author

Source code: https://github.com/xunker/stepper_faker

(C) 2017 Matthew Nielsen, xunker@pyxidis.org
