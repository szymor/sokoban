.PHONY: all clean

TARGET=sokoban
SRC=sokoban.cpp Functions.cpp CUsbJoystickSetup.cpp CInput.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	g++ $(SRC) -o $@ -I/usr/include/SDL -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lSDL_gfx

clean:
	rm -rf $(TARGET)

