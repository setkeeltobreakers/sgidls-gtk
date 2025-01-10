# SGIDLS: Shell Graphical Interface Description Language Scanner
Builds graphical interfaces for interacting with shell scripts from interface description files.

## Rationale
In a modern Unix system, there are two main UX philosophies that users will encounter, one being the traditional Unix toolbox philosophy, found in shell-based applications, 
and the other being Steve Jobs' 'computers as appliances' philosophy, found in graphical applications. One provides a hackable, extensible interface that allows users to
build up complex applications from simple building blocks, exactly suiting them to their purposes. The other provides an easy to use interface that allows users to simply
open up their computer, click a few buttons, and get the job done.

This program attempts to bridge the gap between these two philosophies by providing a way for users and developers to create simple graphical interfaces for shell-based 
applications, whether they be native shell apps or shell scripts. These interfaces are described using a text file, formatted in a JSON-derived description language, allowing
any technically inclined user to easily 'pop the hood' so to speak and modify an existing interface to better suit their purposes.

## Building
Fetch dependencies

On Debian or Debian-based distributions (Ubuntu, Linux Mint, Trisquel, etc.)

`sudo apt install build-essential libgtk-3-dev` <- Must be GTK 3

Build

`cd source/ && 
make && 
cd ../`

This program depends on close interaction with a Unix-like shell, and so it will probably not work on non-Unix-like systems.

## License
This project uses code from the clox interpreter, as described in the book [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.
This code is given under the following license:

```
    Copyright (c) 2015 Robert Nystrom

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
```

The remaining code is given under the following license:

```
    Copyright (c) 2025 setkeeltobreakers

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
```
