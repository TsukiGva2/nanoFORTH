\mainpage
simple and useful FORTH for Arduino Nano
==================================================
**You unboxed the Arduino UNO** from Amazon with excitement, opened the awaiting Arduino IDE, carefully followed one of the numerous available online tutorials, found and compiled the Blink sketch in the included examples, hit the -> button and seconds later, the orange light started blinking! Success! You told yourself that "I think I can be pretty good at this"!

**What comes next** is weeks of ego-boosting of new component badges you collected. From the simple humidity-temperature (DHT), ultrasound (HC-SR04), to how-do-i-read-the-resistor-code LEDs, why-NEC infrared remote (HX1838), flimsy SG90 servos, and finally the damn gyro (MPU6050). OK, that was a pretty good run, you thought. Kids were facinated, and even my typically unconcerning wife seemed to be impressed.

**3 more months has gone by**. Hundreds of compilation/uploading later, though your faithful UNO runs flawlessly blinking all the LEDs you gave him, somewhere in the back of your mind the flash memory's 100K write cycle thing arised. Maybe it's time to give the little brother, Nano, a try. Well, while we're at it, for $20, lets get a pack of five of these little guys might as well.

**So, our journey started.** A long time ago in the digital galaxy far, far away....a simple yet extensible programming language, FORTH, was often found on memory constrained space crafts or remote probes. With the ability to not only compile and run given code but also interact with the operator via a simple serial console. In Earth today's term, it's a complete REPL programming environment, and possiblly can be controlled via wireless connection.

**Jump back to our Nanos**. The same old cycles of your C coding in the Arduino IDE. Compile/upload, compile/upload,... until you're more or less ran out of ideas. Of the 32K flash equiped with Nano, you have never used over 10% and ocassically wonder how many lines of code it takes to fill it up. Over Google, some said 20% with over 30K lines of code. However, they also said, to do anything more meaningful, the 2K RAM will quickly becomes the limiting factor. You hop from example project to projects, wonder in the empty space for a long while. Eventually, feeling sense of lost, your went back to watching Netflex and the once vibrant mind sunk into the daily mondane.

One day, **ESP shows up** on you radar. It can do everything you've imagined thus far. Web serving, mesh-network, LoRa, even MySQL,... Programming-wise, there're C, microPython, even Ruby after some hard digging. Soon, the universe started to expand and what came into view is a Raspberry Pi. The final frontier suddenly became bondless. Your imagination was pushed far beyond the black holes and the vast void. **Life became meaningful** and sure got excited again. MotionEyes, OctaPi, robot buggy, and the plan to try some AI stuffs, ... Your heart were filled with joy and temptations. The limitation of CPU resource or storage is already a distant memory.

**One night**, while you felt the new path is set and the final destination is in sight, you were driving home with the cresting moon hanging over the horizon, a thought came through your mine. **What happened to the Nanos?**

\pagebreak
Compling code in the IDE and upload directory via the tethered USB cable has been the way of life in Arduino universe. Makers get feedback directly from Serial Monitor. This self-contained round-trip development cycle from the comfort of all inside one IDE is the major reason making this platform so popular. FORTH, a simple yet extensible interactive language, arms embedded platforms with REPL coding/debugging process, makes it a natual candidate for microcollers. Its interactive shell can eliminate the bulk of the repetitive compile/upload cycles. Currenly, there are AmForth and FlashForth avaiable for Arduino. Though no direct support from the IDE yet, they demonstrated the value of such on tiny systems. However, both of them also required to overwrite Arduino bootloader whcich needs an additional burner (or called programmer). The additional process not only is an entry barier for beginners but also render your kit a 'non-Arduino' which can leaves a "your warrenty is void" taste in your mouth even if done correctly. An often asked question is "how do I turn it back?".

So, may the FORTH be with you, and **here comes nanoFORTH**!

### With the following assumptions for our Nanos,
* more than 80% of Arduino makers are using UNO or Nano.
* most of the makers do not need the full blown FORTH vocabularies,
* most of them are not familer with standard FORTH words, so I can use abbriviation for words
* the meta-compiler is unlikely needed either, i.e. not to create a new type of Forth from within nanoForth
* only a small set of core primitive words are needed for the most of Arduino projects

### The requirements for myself are:
* be as simple to use as any example Sketch that comes with the IDE (no bootloader burning)
* provide a REPL development/operating environment for Arduino
* provide core Arduino functions (i.g. pinMode, digitalRead,Write, analogRead,Write, millis, delay)
* provide hardware thread(s) in addition to nanoFORTH thread so new components can be added (ig. Bluetooth)
* provide at least 1K RAM dictionary for resonablly size of work
* provide EEPROM persisted storage for new words which can be automatically reloaded on next start-up
* optionally show byte-code stream while assembled to help beginers understand FORTH internal
* optionally show execution tracing to help debugging, also provision for single-stepping
* optionally implemented as an Arduino library that developers can include easily

