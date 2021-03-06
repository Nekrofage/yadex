Yadex 1.7.1 (2010-21-02)
========================

WHAT IS YADEX ?
---------------

Yadex is a Doom level (wad) editor for Unix systems running X,
including Linux. It supports Doom, Doom II, Ultimate Doom, Final
Doom, Heretic, Doom press release pre beta and also, in a more or
less limited way, Hexen, Strife and Doom alpha. Furthermore it
supports the ZDoom Doom/Hexen hybrid engine. It is available
under the terms of the GPL.

PREREQUISITES
-------------

You need:

* Some flavour of Unix (known to work with FreeBSD, Linux, NetBSD, OpenBSD)
* X11R6/Xorg and a display of at least 640x480,
* a Doom/Doom II/Heretic/Strife iwad (shareware/demo is OK),
* GNU make (BSD Make won't do),
* a standard C compiler (ISO 9899:1990),
* a standard C++ compiler which supports C++11,
* `hypot()` and `nanosleep()`
* the links web browser to build the FAQ

BUILDING AND INSTALLING
-----------------------

To install in /usr/local :

    ./configure
    make
    su -c 'make install'

To install somewhere else, for example in ~/yadex :

    ./configure --prefix ~/yadex
    make
    make install

To force the use of a particular C/C++ compiler :

    ./configure --cc egcc --cxx eg++
    make
    make install

Got problems getting Yadex to compile? See doc/faq.html.
Got no problem? Read the FAQ anyway. See doc/faq.html.

Beware, the installation OVERWRITES the following files (assuming
you're installing in /usr/local) :

    /etc/yadex/1.7.1/yadex.cfg
    /usr/local/bin/yadex
    /usr/local/man/man6/yadex.6
    /usr/local/share/games/yadex/1.7.1/doom.ygd
    /usr/local/share/games/yadex/1.7.1/zdoom.ygd
    /usr/local/share/games/yadex/1.7.1/doom02.ygd
    /usr/local/share/games/yadex/1.7.1/doom04.ygd
    /usr/local/share/games/yadex/1.7.1/doom05.ygd
    /usr/local/share/games/yadex/1.7.1/doom2.ygd 
    /usr/local/share/games/yadex/1.7.1/doompr.ygd
    /usr/local/share/games/yadex/1.7.1/heretic.ygd
    /usr/local/share/games/yadex/1.7.1/hexen.ygd
    /usr/local/share/games/yadex/1.7.1/strife.ygd
    /usr/local/share/games/yadex/1.7.1/strife10.ygd

CONFIGURING AND RUNNING
-----------------------

Before you run Yadex, you need to tell it where to find your iwads.
Assuming you have installed in /usr/local, open
/etc/yadex/1.7.1/yadex.cfg with your favourite text editor and
insert the appropriate values for the parameters "iwad1", "iwad2",
etc. If you don't want Doom II to be the default iwad, also change
the value of the "game" parameter.

You can now run Yadex by typing :

    yadex

A "yadex:" prompt should show. At that prompt, type this :

    e map01

or this :

    e e1m1

Have fun !

DOCUMENTATION
-------------

There is a man page and quite a lot of documentation, most of it in
HTML format. Start at :

    doc/index.html

If you're upgrading from a previous version of Yadex, please read
carefully CHANGES.

STATUS
------

Yadex is work in progress. It still lacks important features like a
better interface, cut-and-paste, undo/redo, support for Boom and
many more. I know. They will come faster if you help. The source
code is a horrible mess. I'm not proud of it. Be indulgent.

LEGAL
-----

Yadex
:
    Parts copyright Gregor Best 2010-2016, GNU GPL v2
    Parts copyright Andrew Apted 2000-2001, GNU GPL v2
    Parts copyright Andr� Majorel 1997-2003, GNU GPL v2
    Parts copyright Matthew W. Miller 2000, GNU GPL v2
    Parts written by Rapha�l Quinet, public domain
    Parts written by Brendon Wyber, public domain

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Atclib
:
    The Yadex distribution includes a subset of Atclib.

    Atclib is copyright Andr� Majorel 1995-1999 and distributed under
    the terms of version 2 of the GNU Library General Public License.

CONTACT
-------

See doc/contact.html for addresses.

AYM 2003-12-28
