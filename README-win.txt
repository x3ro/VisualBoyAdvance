Welcome to VisualBoyAdvance version 1.2 (in progress).

Compiling the sources
---------------------

See the INSTALL file for compiling instructions. Please note the following
requisites to compile:

- GCC must be 3.x or greater in order to compile GBA.cpp with -O2. Earlier
  versions have a problem during optimization that requires an absurd
  ammount of memory and usually ends up crashing the compiler/computer
- On Windows, Microsoft Visual C++ 6 or later is needed. Please note that
  some of the source code will not compile with the shipped header files.
  You will need to install the most recent Platform SDK from Microsoft.

Support
-------

Please support VisualBoyAdvance by making a donation. You can donate money
using PayPal (www.paypal.com). Send donations to vb@emuhq.com. If you want
to make other kind of donations (hardware, etc...), please contact me.

Default keys (can be edited in the Options menu)
------------------------------------------------

Arrow keys - direction
Z          - Button A
X          - Button B
A          - Button L
S          - Button R
Enter      - Start
Backspace  - Select
Speedup    - Space
Capture    - F12

You can change the configuration above to use a joystick. Go to
Options->Joypad->Configure... menu.

System requirements
-------------------

Fast computer (Pentium III 500 Mhz recommended) and Microsoft DirectX 7 or
greater.

Translations
------------

Translations can be done as long as you have Microsoft Visual VC++ on
your computer.

If you just want to use a translation, place the translation .DLL on
the same directory as the emulator. From the Options->Language menu,
select Other... and type the three letter (or two) language name from
.DLL. For example, VBA_PTB.DLL: type PTB on the dialog.

These translation files are only for VisualBoyAdvance GUI and messages.
Games will not be translated and cannot be translated by the emulator.

FAQ
---

See online FAQ for more information: http://vboy.emuhq.com/faq.shtml

Please don't email about what you think it is problem before consulting
the FAQ.

Reporting a crash
-----------------

If VisualBoyAdvance crashes, please do the following:

1. Win 95/98/ME: start DrWatson (drwatson.exe) and reproduce the crash.
DrWatson will capture the crash information in a log file (.wlg) file that
needs to be sent to me. Please also open the .wlg file on your machine by
double-clicking and copy the details section into the email. Microsoft
made life harder when you migrate to WinXP (or NT or 2000) by not allowing
DrWatson to read its old file format.

2. Win NT/2000/XP: make sure DrWatson is the default debugger by executing
drwtsn32.exe -i and then recreate the crash. DrWatson will generate a log file
that needs to be sent to me (usually in c:\Documents and Settings\All Users\
Documents\DrWatson). Depending on your system configuration, you may be asked
if you want to generate a log file. If so, please click on yes.

LICENSE
-------

    VisualBoyAdvance - a Gameboy and GameboyAdvance emulator
    Copyright (C) 1999-2002 by Forgotten

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Special Thanks
--------------

Costis for his help fixing some of the graphics bugs.
Snes9x developers for the great emulator and source code.
Gollum for some help and tips.
Kreed for his great graphic filters
And all users who kindly reported problems.

Contact
-------

Please don't email unless you found some bug. Requests will be ignored and
deleted. Also, be descriptive when emailing. You have to tell me what version
of the emulator you are writing about and a good description of the problem.
Remember, there is a SDL version, a Windows version, a Linux version and a
BeOS version.
Also, there are still people writing about the old VisualBoy which is no longer
supported. Also remember I am not paid to work on VisualBoyAdvance.

This is just a hobby.

Forgotten (vb@emuhq.com)
http://vboy.emuhq.com
