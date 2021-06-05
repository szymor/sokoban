arm-linux-g++ -static -s sokoban.o -o sokoban.gpe -Lc:\devkitgp2x\lib -lSDLmain -lSDL_image -lpng -ljpeg -lz -lc -lm -lgcc -lSDL_ttf -lfreetype -lSDL_mixer -lvorbisidec -lmikmod -lSmpeg -lSDL -lSDL_gfx -lpthread  
arm-linux-strip sokoban.gpe
del *.o
