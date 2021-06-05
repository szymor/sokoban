g++ -s sokobanwin.o -o sokoban.exe -Lc:\mingw\lib -mwindows -lmingw32 -lSDLmain -lSDL_image -lpng -lz -lm -lgcc -lSDL_ttf -lfreetype -lSDL_mixer -lvorbis -lvorbisfile -logg -lSDL -lSDL_gfx  
del sokobanwin.o
