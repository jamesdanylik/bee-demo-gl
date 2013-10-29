Implmentation Notes
===================

I decided early on to iminate the approach show in section, and creat functions much
like drawGround to sole the problem.  I also tried to take advantage of the the nature
of problems like the stem by using the control flow constucts of C++, like recursive
function definitions and and object oreiented construction. I also took the specification
one step farther, at least in the flower, by creating a factory function which could
create and animate flowers with automatic slight variations from the main one in the spec.

Insofar as I am aware, the requirement is completely fufilled and several improvements
are present.
 
Overall, I'm very proud of this code, though I was delayed slightly due
to a nasty bug in my wing constucting function near the deadline.  Hopefully, the exta
factory method and resulting field of flowers make up for this somewhat! 

Original Templete README
========================

This is the template for assignment 1 of UCLA CS 174A, taught by professor
Demetri Terzoupolos.

The Angel folder contain the necessary math functions such as vector and matrix 
types and functions. The Ball* files are for mouse arcball control. The FrameSaver
files are for recording into frames, which will be used later in second project
for movie generation. the glsl files are shaders, for now, you do not need to understand
them. The majority(maybe all) of your coding should happen within anim.cpp.

The first step is to successfully run this template code. Second step is to study
the display function inside anim.cpp and try to play with it by adding now objects.
