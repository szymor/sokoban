#ifndef CINPUT_H
#define CINPUT_H
#define MAXJOYSTICKS 10
#define MAXJOYSTICKBUTTONS 37
#define MAXSPECIALKEYS 1
#define MAXMOUSEBUTTONS 32
#define MAXMOUSES 10

#define SPECIAL_QUIT_EV 0

#define JOYSTICKDEADZONE 3200

#define JOYSTICK_LEFT MAXJOYSTICKBUTTONS-2
#define JOYSTICK_UP MAXJOYSTICKBUTTONS-3
#define JOYSTICK_RIGHT MAXJOYSTICKBUTTONS-4
#define JOYSTICK_DOWN MAXJOYSTICKBUTTONS-5
#define JOYSTICK_NONE MAXJOYSTICKBUTTONS-1

#include <SDL/SDL_joystick.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL.h>

class CInput {
    public:
        bool JoystickHeld[MAXJOYSTICKS][MAXJOYSTICKBUTTONS];
        bool SpecialsHeld[MAXSPECIALKEYS];
        bool KeyboardHeld[SDLK_LAST];
        bool MouseHeld[MAXMOUSES][MAXMOUSEBUTTONS];
        CInput(int UpdateCounterDelay);
        ~CInput(void);
        void Update();
        void Reset();
        bool Ready(){ return (UpdateCounter == 0);};
        void Delay(){ UpdateCounter = PUpdateCounterDelay;};
        int NumJoysticks() { return PNumJoysticks;};
    private:
       int PNumJoysticks;
       int UpdateCounter;
       int PUpdateCounterDelay;

};

#endif
