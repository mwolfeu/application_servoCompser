# application_servoCompser
<pre>

  This is a library I wrote for the Atmega328p microcontroller after building my first 
quadroped robot.  Essentially, it is an event subsystem for servo motor control.

Features:
  Create servo state events which trigger other servo movement or user defined callbacks
  Normalize midpoints/directions/movements/safe min-max among sets of servos.
  Servo speed control
  Use external timers / time domains (for speed control)
  Group servos and control N groups with a single event
  Set up and change between several sets of triggers
  Rapid prototyping on Linux 
  
Notes:
  All movements are relative to midpoint
  One object should be defined per action
  Group targets are only intra group
  Programattically generated actions are unlikely so event sets aren't modifiable/deletable
  Servos give no feedback, hence event triggering can't be exact.
  
  Currently not interrupt safe, meaning: There is no object deletion while the event sub-
 system is running.
</pre>
