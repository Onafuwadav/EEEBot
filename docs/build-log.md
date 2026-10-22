# Build log

Photos from my logbook, in build order.

## Front steering chassis

The servo is screwed to an acrylic plate on brass stand-offs, with plastic
6-way fittings either side for the wheels. The servo horn connects to a
short acrylic link (the hole in the horn had to be opened up first).

<p>
  <img src="images/steering-servo-horn.jpg" height="260" alt="Servo horn attached to the acrylic link">
  <img src="images/steering-assembly.jpg" height="260" alt="Steering mechanism on the front chassis">
</p>

## PCB

I soldered the whole board from the schematic with no rework needed.

<p>
  <img src="images/pcb-bare.jpg" height="300" alt="Bare PCB before soldering">
  <img src="images/pcb-soldered.jpg" height="300" alt="Fully soldered PCB">
</p>

<img src="images/pcb-schematic.jpg" width="100%" alt="EEEBot PCB schematic">

## Chassis, battery, motors

The front steering chassis bolts to the PCB on nylon spacers; the rear
chassis uses metal screws, lock nuts and 3D-printed spacers. The battery pack
was first soldered in reversed, so a pit-stop request was raised and it was
re-soldered the right way round.

<p>
  <img src="images/front-chassis-on-pcb.jpg" height="280" alt="Front chassis attached to the PCB">
  <img src="images/battery-pack.jpg" height="280" alt="Battery pack soldered to the PCB">
  <img src="images/motor-mount.jpg" height="280" alt="Motor screwed to the rear chassis">
</p>

## Encoders

The encoder stripboard sits in the rear chassis cradle; its wires go to the
encoder header on the PCB.

<p>
  <img src="images/encoder-stripboard.jpg" height="260" alt="Encoder stripboard">
  <img src="images/encoder-header.jpg" height="260" alt="Encoder wires in the PCB header">
</p>

## Level shifter and line sensor

<p>
  <img src="images/level-shifter.jpg" height="240" alt="Bi-directional voltage level shifter module">
  <img src="images/ir-sensor-board.jpg" height="240" alt="IR line sensor stripboard layout">
</p>

*Left: the 3.3 V / 5 V level shifter between the ESP32 and the Nano.
Right: IR emitter/receiver stripboard layout for line following.*
