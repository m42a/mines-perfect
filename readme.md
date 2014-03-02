# Mines-Perfect - a minesweeper clone

This is an updated version of mines-perfect which runs on Linux and Windows
(and probably Mac OS X, although that hasn't been tested).  I started this port
because I really like this game, and it doesn't run properly in wine.  This
port will rely on several C++11 features to replace the code that doesn't work
on Linux, but won't add any new dependencies unless they're really necessary.

If anyone would like to help, I could really use some Windows testers in
addition to regular patchers, since I don't have a copy of Windows to test on.
Also, most of the code's comments are in German, so if someone could translate
them into English, that would be fantastic.

The project isn't very complicated to build; just run `make` and you'll have a
fully functioning executable named `mines-perfect` in the root directory.  Your
settings and scores get saved in your home directory on Linux and in the
registry on Windows.
