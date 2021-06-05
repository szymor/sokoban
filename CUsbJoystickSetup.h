#ifndef CUSBJOYSTICK_H
#define CUSBJOYSTICK_H

#include <SDL/SDL.h>
#include <stdio.h>
#include "Functions.h"

#define MAXDEFINITIONS 50
#define DESCRIPTIONSIZE 1024






struct SJoystickButtonDefinition {
    char ButtonDescription[DESCRIPTIONSIZE];
    int DefaultButtonValue;
    int CurrentButtonValue;
    int DefaultKeyValue;
    int CurrentKeyValue;
    char KeyboardDisplayValue[15];
    char DefaultKeyboardDisplayValue[15];
};

class CUsbJoystickSetup {
    private:
       int PNrOfDefinitions;
       SJoystickButtonDefinition PJoystickButtons[MAXDEFINITIONS];
    public:
        bool AddDefinition(int Button, char* Definition, int value, int defaultValue,int keyValue, int defaultKeyValue, char *DisplayValue);
        int GetButtonValue(int Button);
        int GetKeyValue(int Button);
        bool SaveCurrentButtonValues(char *Filename);
        bool LoadCurrentButtonValues(char *Filename);
        void SetButtonValue(int Button, int Value);
        void SetKeyValue(int Button, int Value);
        char *GetKeyNameForDefinition(int Definition);
        void DrawCurrentSetup(SDL_Surface *Surface,TTF_Font* FontIn,int X, int Y, int XSpacing, int YSpacing ,int Selection, SDL_Color TextColor,SDL_Color SelectedTextColor,bool Keyboard);
        CUsbJoystickSetup();
        ~CUsbJoystickSetup();
        void ResetToDefaults();

};
#endif
