Midlive
=======

***A programmable midi sequencer and performance environment concept***

Midlive is the working title for a live performance environment that is
designed to make it easy to develop and improvise musical patterns.  It
will generate midi and be a live environment, and my first performance
with it (if there ever is one) will probably be a crisis, so the name
works for now.

The initial plan is to develop a language that can express the intent of
the project. Such a language could form the basis for a live-coding
environment, though I imagine the final environment to be some kind of
GUI.  It's useful however to start with a language that can be written
down, as this provides a nice way to experiment with ideas and iterate
them quickly, and I can start with a terminal app that generates midi
files and go from there.

Hacking
-------

This project is coded on rainy (and sunny) weekends for fun.

Contributions are welcome but please treat this code more like C with
bells on than C++.  I mostly use C++ for function overloading, pass by
ref, const, and a few other things.

Anything that significantly increases compile time is eschewed, so
templates and the linker are out. So are namespaces, or anything else
that obfuscates search with simple command-line tools like grep or ag,
and so are most OOP features, such as private members, RAII, and so on.

These aren't hard and fast rules.  Read the code to better understand
it.

There are aliases available to help with compiling for both clang and gcc.

Licence
-------

Copyright 2019 David Marrs,

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
