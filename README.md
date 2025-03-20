# SandBocks

![SandBocks](https://github.com/user-attachments/assets/98e91b13-ce79-404c-91d0-3197cc6880b9)

I used to love the powder toy, played a lot of it as a child. How cool would it be then if you could have a little box or panel which runs something like that, which you could use a touch-screen to play around with in real-time?
That is the overall purpose of this project. I want to create a grid of LED lights and use a touch screen to manipulate the game. I have the hardware ready but will discuss that later..

Written in python, my first attempt (SANDbOCKS_v1.py) is really shitty and unoptimised. It uses Pygame to display and play the game (with the mouse + spacebar), but I want to move away from this to create something physical.

Second attempt (sandbox.py) is going well, but I have tested and it's still too slow for my microcontroller with MicroPython. I will continue to develop this version into a polished state. It still uses Pygame for display purposes, as the physical display isn't capable of interfacing yet. But currently to draw each element you have to hardcode it either in or before the main loop. I will likely add mouse support, but a much simpler version considering a touch screen only boolean sense of touch (reliably). Some features will be missing from this version too as I deem them unnecessary to complete the project. If I really need things like Teleport or Barriers I can add them in later (begrudgingly).

The plan is lastly to convert the polished game to C or C++ to make this speedy code for an ESP32-S3 microcontroller. This will be used to light up a grid of LED lights per pixel to have a physical interractable decoration with a touch screen.
