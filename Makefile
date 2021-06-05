.PHONY: all clean

TARGET=sokoban

all: $(TARGET)

$(TARGET):
	g++ sokoban.cpp Functions.cpp CUsbJoystickSetup.cpp CInput.cpp -o $@ -I/usr/include/SDL -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lSDL_gfx

clean:
	rm -rf $(TARGET)
