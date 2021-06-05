#include "CInput.h"


CInput::CInput(int UpdateCounterDelay) {
    Reset();
    PNumJoysticks = SDL_NumJoysticks();
    //PNumJoysticks = 1;
    for (int teller=0;teller< PNumJoysticks;teller++)
        SDL_JoystickOpen(teller);
    SDL_JoystickEventState(SDL_ENABLE);
    PUpdateCounterDelay = UpdateCounterDelay;
    UpdateCounter = 0;
}

CInput::~CInput() {
}

void CInput::Update() {
    //printf("Cinput::update:0\n");
    SDL_Event Event;
    if (UpdateCounter > 0)
        UpdateCounter--;
    while(SDL_PollEvent(&Event))
    {
        if(Event.type == SDL_QUIT)
            SpecialsHeld[SPECIAL_QUIT_EV] = true;
        if(Event.type == SDL_KEYDOWN)
            KeyboardHeld[Event.key.keysym.sym] = true;
        if(Event.type == SDL_KEYUP)
            KeyboardHeld[Event.key.keysym.sym] = false;
        if(Event.type == SDL_JOYAXISMOTION)
            if(Event.jaxis.axis == 0)
            {
                if(Event.jaxis.value > JOYSTICKDEADZONE)
                {
#ifdef GP2X
                    //printf("Cinput::update:1\n");
                    if(Event.jaxis.which < MAXJOYSTICKS)
                        JoystickHeld[Event.jaxis.which][JOYSTICK_RIGHT] = true;
#else
                    if(Event.jaxis.which+1 < MAXJOYSTICKS)
                        JoystickHeld[Event.jaxis.which+1][JOYSTICK_RIGHT] = true;
#endif

                }
                else
                    if(Event.jaxis.value < -JOYSTICKDEADZONE)
                    {
#ifdef GP2X
                        //printf("Cinput::update:2\n");
                        if(Event.jaxis.which < MAXJOYSTICKS)
                            JoystickHeld[Event.jaxis.which][JOYSTICK_LEFT] = true;
#else
                        if(Event.jaxis.which +1< MAXJOYSTICKS)
                            JoystickHeld[Event.jaxis.which+1][JOYSTICK_LEFT] = true;
#endif
                    }
                    else
                    {
#ifdef GP2X
                        if(Event.jaxis.which < MAXJOYSTICKS)
                        {
                            //printf("Cinput::update:3\n");
                            JoystickHeld[Event.jaxis.which][JOYSTICK_LEFT] = false;
                            JoystickHeld[Event.jaxis.which][JOYSTICK_RIGHT] = false;
                        }
#else
                        if(Event.jaxis.which +1< MAXJOYSTICKS)
                        {

                            JoystickHeld[Event.jaxis.which+1][JOYSTICK_LEFT] = false;
                            JoystickHeld[Event.jaxis.which+1][JOYSTICK_RIGHT] = false;
                        }
#endif
                    }
            }
            else
                if(Event.jaxis.axis == 1)
                {
                    if(Event.jaxis.value > JOYSTICKDEADZONE)
                    {
#ifdef GP2X
                        //printf("Cinput::update:4\n");
                        if(Event.jaxis.which < MAXJOYSTICKS)
                            JoystickHeld[Event.jaxis.which][JOYSTICK_DOWN] = true;
#else
                        if(Event.jaxis.which +1< MAXJOYSTICKS)
                            JoystickHeld[Event.jaxis.which+1][JOYSTICK_DOWN] = true;
#endif

                    }
                    else
                        if(Event.jaxis.value < -JOYSTICKDEADZONE)
                        {
#ifdef GP2X
                            //printf("Cinput::update:5\n");
                            if(Event.jaxis.which < MAXJOYSTICKS)
                                JoystickHeld[Event.jaxis.which][JOYSTICK_UP] = true;
#else
                            if(Event.jaxis.which +1< MAXJOYSTICKS)
                                JoystickHeld[Event.jaxis.which+1][JOYSTICK_UP] = true;
#endif
                        }
                        else
                        {
#ifdef GP2X
                            //printf("Cinput::update:6\n");
                            if(Event.jaxis.which < MAXJOYSTICKS)
                            {
                                JoystickHeld[Event.jaxis.which][JOYSTICK_DOWN] = false;
                                JoystickHeld[Event.jaxis.which][JOYSTICK_UP] = false;
                            }
#else
                            if(Event.jaxis.which+1 < MAXJOYSTICKS)
                            {
                                JoystickHeld[Event.jaxis.which+1][JOYSTICK_DOWN] = false;
                                JoystickHeld[Event.jaxis.which+1][JOYSTICK_UP] = false;
                            }
#endif
                        }
                }
        if(Event.type == SDL_JOYBUTTONDOWN)
            if(Event.jbutton.button < MAXJOYSTICKBUTTONS)
            {
#ifdef GP2X
                //printf("Cinput::update:7\n");
                if(Event.jbutton.which < MAXJOYSTICKS)
                    JoystickHeld[Event.jbutton.which][Event.jbutton.button] = true;
#else
                if(Event.jbutton.which+1 < MAXJOYSTICKS)
                    JoystickHeld[Event.jbutton.which+1][Event.jbutton.button] = true;
#endif
            }
        if(Event.type == SDL_JOYBUTTONUP)
            if(Event.jbutton.button < MAXJOYSTICKBUTTONS)
            {
#ifdef GP2X
                //printf("Cinput::update:8\n");
                if(Event.jbutton.which < MAXJOYSTICKS)
                    JoystickHeld[Event.jbutton.which][Event.jbutton.button] = false;
#else
                if(Event.jbutton.which+1 < MAXJOYSTICKS)
                    JoystickHeld[Event.jbutton.which+1][Event.jbutton.button] = false;
#endif
            }
        if(Event.type == SDL_MOUSEBUTTONDOWN)
            if(Event.button.which < MAXMOUSES)
                if(Event.button.button < MAXMOUSEBUTTONS)
                    MouseHeld[Event.button.which][Event.button.button] = true;
        //printf("Cinput::update:9\n");
        if(Event.type == SDL_MOUSEBUTTONUP)
            if(Event.button.which < MAXMOUSES)
                if(Event.button.button < MAXMOUSEBUTTONS)
                    MouseHeld[Event.button.which][Event.button.button] = false;
        //printf("Cinput::update:10\n");

    }
}

void CInput::Reset() {
    //printf("Cinput::update:11\n");
    int x,y;
    for (x=0;x<MAXJOYSTICKS;x++)
        for (y=0;y<MAXJOYSTICKBUTTONS;y++)
            JoystickHeld[x][y] = false;
    for (x=0;x<MAXMOUSES;x++)
        for (y=0;y<MAXMOUSEBUTTONS;y++)
            MouseHeld[x][y] = false;
    for (x=0;x<SDLK_LAST;x++)
        KeyboardHeld[x] = false;
    for (x=0;x<MAXSPECIALKEYS;x++)
        SpecialsHeld[x] = false;
    //printf("Cinput::update:12\n");

}

