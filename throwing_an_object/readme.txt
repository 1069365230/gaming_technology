video link: https://youtu.be/rHQzHFd7jqk

I remove the plane for better visualiztion and added the bottom sky box instead of just black a black hole.

pressing space to throw cubes

I have both the translation caused by the impulse force and the rotation caused by torque.

I choose to have a consistant angular momentum, because the torque is an impulse and there is no gravity force applied.

But however I also test it when the angular momentum increases over time, it is very interesting to me. I always thought it will look more realistic when it increases.
(just need to change this->angularMomentum = torque * dt in my Mycube.h inside of getAngularVelocity() function.)

The impulse and torque is define in main.cpp inside of onFrameStarted(). And also to noticed that impulse need to set a time, 
in order to determine how long it will last on the object, and when the time passed, the only force left will be gravity.

For me, I have difficulties understanding the formulas, even though I rewatched the lecture multiple times. specially the dt, the relationships between linear momentum and Force.

files changed:
main.cpp
VEEventListenerGLFW.cpp
VEInclude.h

new files:
Mycube.h (please place it under Header Files dir) 







