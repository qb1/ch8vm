ch8vm
=====

A simple CHIP-8 virtual machine (or emulator?) in C & SDL (with no sound yet, but you don't miss much)

## What is that?

CHIP-8 is an interpreted ASM-like language & VM specification created in the late 70s; its purpose was to help the creation of games. You can find more relevant information on the the [wikipedia page](http://en.wikipedia.org/wiki/CHIP-8). Note that this virtual machine doesn't support the CHIP-8 descendant, SCHIP.

Since this architecture is very simple (simpler than a gameboy, and a bit more geeky), it is a nice choice for a first VM projet -- which this is.

## How can I use it?

First you need to get CHIP-8 binary programs, which you can find at [http://devernay.free.fr/hacks/chip8/](http://devernay.free.fr/hacks/chip8/) or anywhere else on the Internet.

Then get your hands on SDL-dev for your OS. The makefile is intended for clang, but for now gcc would work as well.

Then, compile the program and run it:

	# make && ./ch8wm progs/TICTAC.ch8

And you'll get something like:
![](https://raw.github.com/qb1/ch8vm/master/screenshot.png)

The CHIP-8 keyboard is loosely mapped on the numpad so that it's as natural as possible; no game has a manual anyway so you'll have to try all numpad keys to find how to play a game.

## The games run slow/too fast, why?

Well, I had to set the emulation speed to certain value, but I couldn't find any specification on that so I did as I liked. The timers are specified to be 60Hz though; I respected this so games that use those feel as they should.

But this whole speed limit is a bit crude anyway, feel free to patch it into something better.

## This code could be improved!

Yeah, I know -- global variables and all that. This VM is intented to be the base work for a small experiment, so I cut short anywhere I could to get to the point. Just don't use it to learn proper industry-standard coding practices.