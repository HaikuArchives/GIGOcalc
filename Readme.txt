GIGOcalc

A minimalist's calculator, by Shamyl Zakariya & Sam Gutterman
zakariya@earthlink.net

Nothing more than a slim expression calculator, GIGOcalc was born one day when I was porting some old rasterizing code from win32 to BeOS and while debugging I needed a good calculator. I tried BeBits but the calculators were either too much or too little. Too scientific, or too much like the good old MacOS calculator (eg nothing). What I wanted was a slim calculator (no real-world style buttons, just text fields) and the ability to solve expressions, and do some simple programming style functions like binary shifts, and ands.

Usage is simple, just fire it up, type an equation into the upper field and hit enter. You can control wether the output is in decimal, binary, octal or hex by clicking on the popup menu on the right and selecting "Output in base" > (BIN, OCT, DEC, HEX), further you can select wether trig functions use degrees or radians by clicking the popup and selecting "Trig" > (decimal, radians)

It's a convenience function, but selecting "Select Answer" will cause GIGOcalc to highlight the answer field when you hit enter, so you can ctrl-c the answer and paste it elsewhere.

Functions supported:
sin, cos, tan, asin, acos, atan
+, -, *, /, ^
&, |, <<, >>, %
() parenthetizing
standard c style operator precedance


Why's it called GIGOcalc?
Well, as they say Garbage-in-garbage-out. The original GIGOcalc couldn't handle poorly parenthetized expressions, or bad syntax. 2.0 can, it has much better error handling, but the name stuck.

history
GIGOcalc 1.0 by shamyl zakariya & sam gutterman
GIGOcalc 2.0 by shamyl (just improvements to error checking and gui)