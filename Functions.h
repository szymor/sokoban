#ifndef FUNCTION_H
#define FUNCTION_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#define SCREEN_BPP 16

#if (SCREEN_BPP == 16)
 typedef Uint16 pixel;
#else
#if (SCREEN_BPP == 8)
 typedef Uint8 pixel;
#else
 typedef Uint32 pixel;
#endif
#endif

#define SCREEN_BYTE sizeof(pixel)


void WriteText(SDL_Surface* Surface,TTF_Font* FontIn,char* Tekst,int NrOfChars,int X,int Y,int YSpacing,SDL_Color ColorIn);
SDL_Surface* ScaleSurface(SDL_Surface* Surface,int ScaleFactor);
bool FileExists(char * FileName);
void AddUnderScores (char *string);
void RemoveUnderScores (char *string);
char chr(int ascii);
int ord(char chr);
#endif
