Buttons:
- start/stop
- mode switch
- submode switch

Animations:

 Points moving
  Controls:
  - brightness
  - hue
  - speed
  - width
  Submode: one direction/bounce/both directons at once

  2 points, with given speed.
  When a point reaches its end, generate new point.
   - one or both directions: generate with same direction
   - bounce: reverse point direction
   init:
   - one direction: disperse points
   - both directions: pair points with inverted speed
   - bounce: disperse points (?)

---

 Points appearing randomly
  Controls:
  - brightness
  - hue
  - frequency/speed
  - duration
  Submode: appear and disappear/appear and flow down/flow up and disappear/flow from point to point

  Generate points at set intervals, with random start and end position
  - appear and disappear: start pos = end pos = random(0%-100%)
  - appear and flow down: start pos = random(50%-100%), end pos = 100%, direction = random
  - flow up and disappear: start pos = 0%, end pos = random(0%-50%), direction = random
  - point to point: start pos = random(0%-100%) end pos = random(start pos - 100%), direction = random
  
---

 Point pulsing
  Controls:
  - brightness
  - hue
  - frequency
  - position
  Submode: just pulsing/streamers up/streamers down

  One point at fixed position, controlled by knob
  - streamers down: generate points at pulsing frequency high, start pos = max(position or 100%-position), end pos = 100%
  - streamers up: generate points at pulsing frequency speed time before high, start pos = 0%, end pos = min(position or 100%-position)
