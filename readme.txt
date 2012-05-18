
            Mines-Perfect - a minesweeper-clone
            -----------------------------------

See readme.md for up-to-date information

Introduction:

  The sources contain all files which are necessary to build
  the mineperf.exe file.
  The following files are only in the precompiled distribution(s):

  - additional boards
  - help-files
  - changes-file


File-structure:

  - core:  all files with have nothing to do with a GUI
  - gui:   all files which represent a control but use no extern library
  - wxwin: all files which use the wxWindows-library


You need for build:

  - Visual C++ 6.0 (use wxwin/mineperf.dsw)
  - wxwindows 2.40 (Sourceforge)

  or

  - Dev-C++ 4.9.7/gcc 2.95 (Sourceforge)  (use wxwin/mineperf.dev)
  - wxwindows 2.29 (Sourceforge)  (there exist a plug-in for Dev-C++)

  or

  - mingw/gcc 2.95 (use wxwin/makefile.win)
  - wxwindows 2.29 (Sourceforge)


!! You cannot send highscores with a selfcompiled mineperf.exe !!


General:
  homepage:        www.czeppi.de
  e-mail:          mail@czeppi.de
  sourceforge.net: search for mines-perfect.

