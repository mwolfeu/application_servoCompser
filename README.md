<pre>
# application_servoCompser

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
    no del while event checking.
</pre>
