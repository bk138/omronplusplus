
this is OMRON++ - the simplistic peace-endowing warfare simulation.

![OMRON++ logo](logo.jpg "Omron++ Logo")


[![CI](https://github.com/bk138/omronplusplus/actions/workflows/ci.yml/badge.svg)](https://github.com/bk138/omronplusplus/actions/workflows/ci.yml) [![Build status](https://ci.appveyor.com/api/projects/status/cy24qkawvq8ldwxn/branch/master?svg=true)](https://ci.appveyor.com/project/bk138/omronplusplus/branch/master)



what is it
==========

in omron++, you have up to four armies of pixels fighting each other.
you can choose the number of soldiers on each side and some other parameters.
if you give them a certain "mobsize", they will form groups of pixels (called 
mobs) and go hunting together.





how to use it
=============
simply follow the menu promptings and enter your choices, then watch the whole 
thing going.

you can press 's' to take a screenshot, and 'f' to toggle between window and
fullscreen. press 'p' or 'pause' to pause the game when in battle. finally, you 
can toggle playing of sounds via 'm'.

hit 1,2,3 or key 4 to make your army retreat to the dropzone.


for 'advanced' usage type 'omron++ -h' to see the possible commandline 
arguments. (a special one is '-auto', which runs omron++ in some kind of 
standalone mode, without the need for user input. instead it randomly chooses
the parameters itself. just give it the percentage of the possible maximum 
values to adjust performance to your machine. makes a nice but resourcehungry 
screensaver ;-) )





how to compile
==============

the prerequisites:

- the usual c-compiler with headers and stuff
- libSDL devel package version >= 1.2.0
- libSDL-gfx devel package version >= 2.0.9
    
to compile:

```
mkdir build
cd build
cmake ..
cmake --build .
```

to install:
    - 'make install' as root 
    - or copy omron++ binary wherever you like to 
    
       
    



that's pretty much it, have fun !


---

comments, wishes and bug reports most welcome.                                   
christian beier <dontmind@freeshell.org>    

    
