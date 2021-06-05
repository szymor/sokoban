#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <windows.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL.h>
#include <SDL/SDL_Main.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>


const int NrOfRows = 15,
		  NrOfCols = 20,
		  MaxHistory = 1000,
		  TileWidth = 16,
		  TileHeight =16,
		  ZEmpty= 6,
		  ZPlayer = 5,
		  ZBox = 4,
		  ZWall = 3,
		  ZSpot = 2,
		  ZFloor = 1,
		  IDPlayer = 1,
		  IDBox = 2,
		  IDWall = 3,
		  IDSpot = 4,
		  IDEmpty = 5,
		  IDFloor = 6,
		  FPS = 40,
		  MaxLevelPacks=200,
		  MaxMusicFiles=26,
		  NrOfSounds=5,
		  SND_DROP=0,
		  SND_MENU=1,
		  SND_SELECT=2,
		  SND_ERROR=3,
		  SND_STAGEEND=4,
          WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;


using namespace std;

enum GameStates {GSTitleScreen,GSCredits,GSIntro,GSQuit,GSGame,GSStageClear,GSStageSelect,GSLevelEditor,GSLevelEditorMenu};

SDL_Surface *IMGBackground=NULL,*IMGFloor=NULL,*IMGPlayer=NULL,*IMGBox=NULL,*IMGSpot=NULL,*IMGEmpty=NULL,*IMGWall=NULL,*IMGTitleScreen=NULL;

class CWorldPart;

struct SPrevPoint
{
	int X,Y;
};



class CHistory
{
 private:
 	SPrevPoint Items[MaxHistory];
 	CWorldPart *Part;
	int ItemCount;
 public:
 	CHistory(CWorldPart *Partin);
 	void Add (int X, int Y);
 	void GoBack();
 	void Print();
};



class CWorldParts
{
 private:
 	void Sort();
 	bool DisableSorting;
 public:
 	CWorldPart *Items[NrOfRows*NrOfCols*3];
 	int ItemCount;
 	CWorldParts();
 	void Add(CWorldPart *WorldPart);
 	void Move();
 	void HistoryAdd();
 	void HistoryGoBack();
 	void Draw(SDL_Surface *Surface);
 	void Remove(int PlayFieldXin,int PlayFieldYin);
 	void Remove(int PlayFieldXin,int PlayFieldYin,int Type);
 	void RemoveAll();
	void Save(char *Filename);
	void Load(char *Filename);
 	~CWorldParts();
};

class CWorldPart
{
 private:
  	int MoveDelayCounter;
  	bool FirstArriveEventFired;
 protected:
 	int Type,MoveSpeed,MoveDelay,Xi,Yi,X,Y,AnimPhase,PlayFieldX,PlayFieldY,Z;
 	bool BHistory;
 	SDL_Surface * Image;

 public:
 	CWorldParts *ParentList;
 	bool IsMoving;
 	CHistory *History;
 	bool Selected;
 	CWorldPart(const int PlayFieldXin,const int PlayFieldYin,bool CreateHistory)
 	{
 		BHistory = CreateHistory;
 		if (BHistory)
 			History = new CHistory(this);
 		PlayFieldX=PlayFieldXin;
 		PlayFieldY=PlayFieldYin;
 		Xi=0;
 		Yi=0;
 		X=PlayFieldXin*TileWidth;
 		Y=PlayFieldYin*TileHeight;
 		Type=0;
 		MoveDelay=0;
 		MoveDelayCounter=0;
 		IsMoving = false;
 		MoveSpeed=0;
 		Image = NULL;
 		ParentList = 0;
 		AnimPhase=0;
 		Selected = false;
 		FirstArriveEventFired = false;
 		Z=0;
 	}
 	int GetType() {return Type;};
 	int GetX() {return X;}
 	int GetY() {return Y;}
 	int GetPlayFieldX() {return PlayFieldX;}
 	int GetPlayFieldY() {return PlayFieldY;}
 	int GetZ() {return Z;}
 	int GetAnimPhase() {return AnimPhase;}
 	bool HasHistory() { return BHistory;}
 	void HistoryAdd() { if (BHistory) History->Add(PlayFieldX,PlayFieldY); }
 	void HistoryGoBack() { if(BHistory) History->GoBack();}
 	void SetAnimPhase(int AnimPhaseIn) { AnimPhase = AnimPhaseIn;}
 	virtual void Event_ArrivedOnNewSpot() {}
 	virtual void Event_BeforeDraw() {}
 	virtual void Event_LeaveCurrentSpot() {}
 	void SetPosition(const int PlayFieldXin,const int PlayFieldYin)
 	{
 		if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
 		{
 			PlayFieldX=PlayFieldXin;
 			PlayFieldY=PlayFieldYin;
 			X=PlayFieldXin*TileWidth;
 			Y=PlayFieldYin*TileHeight;
 			Event_ArrivedOnNewSpot();
 		}
 	}
 	virtual void MoveTo(const int PlayFieldXin,const int PlayFieldYin,bool BackWards)
 	{
 		if(!IsMoving)
 		{
 			if((PlayFieldXin != PlayFieldX) || (PlayFieldYin != PlayFieldY))
 				if(this->CanMoveTo(PlayFieldXin,PlayFieldYin) || BackWards)
 				{
 					PlayFieldX = PlayFieldXin;
 					PlayFieldY = PlayFieldYin;
 					if(X < PlayFieldX*TileWidth)
 						Xi = MoveSpeed;
 					if(X > PlayFieldX*TileWidth)
 						Xi = -MoveSpeed;
 					if(Y > PlayFieldY*TileHeight)
 						Yi = -MoveSpeed;
 					if(Y < PlayFieldY*TileHeight)
 						Yi = MoveSpeed;
 					IsMoving = true;
 					Event_LeaveCurrentSpot();
 				}
 		}
 	}
 	virtual bool CanMoveTo(const int PlayFieldXin,const int PlayFieldYin) {return false;}
 	virtual void Move()
 	{
 		if (!FirstArriveEventFired)
 		{
			Event_ArrivedOnNewSpot();
			FirstArriveEventFired=true;
 		}
 		if (IsMoving)
		{
			if (MoveDelayCounter == MoveDelay)
			{
				X += Xi;
				Y += Yi;
				if ((X == PlayFieldX * TileWidth) && (Y == PlayFieldY * TileHeight))
				{
					IsMoving = false;
					Xi = 0;
					Yi = 0;
					Event_ArrivedOnNewSpot();
				}
				MoveDelayCounter = -1;
			}
			MoveDelayCounter++;
		}
 	}
 	virtual void Draw(SDL_Surface* Surface)
 	{
 		//printf("Start draw type:%d\n",Type);
 		if (Image)
 		{
 			Event_BeforeDraw();
 			SDL_Rect SrcRect,DstRect;
			SrcRect.x = AnimPhase * TileWidth;
			SrcRect.y = 0;
			SrcRect.w = TileWidth;
			SrcRect.h = TileHeight;
			DstRect.x = X;
			DstRect.y = Y;
			DstRect.w = TileWidth;
			DstRect.h = TileHeight;
			SDL_BlitSurface(Image,&SrcRect,Surface,&DstRect);
		}
		if (Selected)
		{
			boxRGBA(Surface,X,Y,X+TileWidth-1,Y+TileHeight-1,0,0,200,125);
			rectangleRGBA(Surface,X,Y,X+TileWidth-1,Y+TileHeight-1,0,0,255,125);
		}
		//printf("End draw type:%d\n",Type);
 	}
 	virtual ~CWorldPart()
 	{
 		if(BHistory)
			delete History;
 	}
};

class CEmpty : public CWorldPart
{
 public:
	CEmpty(const int PlayFieldXin,const int PlayFieldYin) : CWorldPart(PlayFieldXin,PlayFieldYin,false)
	{
		Image = IMGEmpty;
		Type = IDEmpty;
		Z = ZEmpty;
	}
};

class CFloor : public CWorldPart
{
 public:
	CFloor(const int PlayFieldXin,const int PlayFieldYin) : CWorldPart(PlayFieldXin,PlayFieldYin,false)
	{
		Image = IMGFloor;
		Type = IDFloor;
		Z = ZFloor;
	}
};


class CWall : public CWorldPart
{
 public:
 	CWall(const int PlayFieldXin,const int PlayFieldYin) : CWorldPart(PlayFieldXin,PlayFieldYin,false)
 	{
 		Image = IMGWall;
		Type = IDWall;
		Z = ZWall;
 	}

};



class CBox : public CWorldPart
{
 private:
 	void Event_ArrivedOnNewSpot();
 	bool CanMoveTo(const int PlayFieldXin,const int PlayFieldYin);
 	void Event_LeaveCurrentSpot();
 public:
	CBox(const int PlayFieldXin,const int PlayFieldYin) : CWorldPart(PlayFieldXin,PlayFieldYin,true)
	{
		Image = IMGBox;
		Type = IDBox;
		MoveDelay = 0;
		MoveSpeed = 2;
		Z = ZBox;
	}
};



void CBox::Event_LeaveCurrentSpot()
{
	if (ParentList)
	{
		int Teller;
		for (Teller=0;Teller< ParentList->ItemCount;Teller++)
		{
			if( ParentList->Items[Teller]->GetType() == IDSpot)
			{
				if ((ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldX) && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldY))
				{
					AnimPhase = 1;
					break;
				}
			}
		}
	}
}

bool CBox::CanMoveTo(const int PlayFieldXin,const int PlayFieldYin)
{
	int Teller;
	bool Result = true;
	if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
	{
		if (ParentList)
		{
			for (Teller=0;Teller<ParentList->ItemCount;Teller++)
				if((ParentList->Items[Teller]->GetType() == IDWall) || (ParentList->Items[Teller]->GetType() == IDBox))
					if((ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldXin) && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldYin))
					{
						Result = false;
						break;
					}
		}
	}
	else
		Result = false;
	return Result;
}

void CBox::Event_ArrivedOnNewSpot()
{
	int Teller;
	AnimPhase = 0;
	//printf("Arrive Event fired\n");
	if (ParentList)
	{
		//printf("Parent List Set\n");
		for (Teller=0;Teller< ParentList->ItemCount;Teller++)
		{
			if( ParentList->Items[Teller]->GetType() == IDSpot)
			{
				if ((ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldX) && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldY))
				{
					AnimPhase = 1;
					break;
				}
			}
		}
	}
}

class CSpot : public CWorldPart
{
 private:
 	void Event_ArrivedOnNewSpot();
 public:
 	CSpot(const int PlayFieldXin,const int PlayFieldYin) : CWorldPart(PlayFieldXin,PlayFieldYin,false)
 	{
		Image = IMGSpot;
		Type = IDSpot;
		Z = ZSpot;
 	}
};

void CSpot::Event_ArrivedOnNewSpot()
{
	int Teller;
	//printf("Arrive Event fired\n");
	if (ParentList)
	{
		//printf("Parent List Set\n");
		for (Teller=0;Teller< ParentList->ItemCount;Teller++)
		{
			if( ParentList->Items[Teller]->GetType() == IDBox)
			{
				if ((ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldX) && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldY))
				{
					ParentList->Items[Teller]->SetAnimPhase(1);
					break;
				}
			}
		}
	}
}

class CPlayer : public CWorldPart
{
 private:
 	int AnimCounter,AnimBase,AnimDelay,AnimDelayCounter,AnimPhases;
 	void Event_BeforeDraw();
 public:
 	bool CanMoveTo(const int PlayFieldXin,const int PlayFieldYin);
 	void MoveTo(const int PlayFieldXin,const int PlayFieldYin,bool BackWards);
 	CPlayer(const int PlayFieldXin,const int PlayFieldYin) : CWorldPart(PlayFieldXin,PlayFieldYin,true)
 	{
		Image=IMGPlayer;
		AnimBase=4;
		AnimPhase=4;
		AnimPhases=2;
		AnimCounter = 1;
		AnimDelay = 8;
		MoveDelay = 0;
		MoveSpeed = 2;
		AnimDelayCounter =0;
		Type = IDPlayer;
		Z = ZPlayer;
 	}
 	void MoveTo(const int PlayFieldXin,const int PlayFieldYin);
};


class CSelector
{
 private:
 	CWorldPart *Part;
 	int Selection;
 public:
	CSelector();
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void IncSelection();
	void DecSelection();
	void Draw(SDL_Surface *Surface);
	int GetSelection();
	int GetPlayFieldX();
	int GetPlayFieldY();
	~CSelector();
};

CSelector::CSelector()
{
	Selection = IDEmpty;
	Part = new CEmpty(9,7);
	Part->Selected = true;
}

void CSelector::MoveUp()
{
	Part->SetPosition(Part->GetPlayFieldX(),Part->GetPlayFieldY()-1);
}

void CSelector::MoveDown()
{
	Part->SetPosition(Part->GetPlayFieldX(),Part->GetPlayFieldY()+1);
}

void CSelector::MoveRight()
{
	Part->SetPosition(Part->GetPlayFieldX() +1,Part->GetPlayFieldY());
}

void CSelector::MoveLeft()
{
	Part->SetPosition(Part->GetPlayFieldX() -1,Part->GetPlayFieldY());
}

void CSelector::IncSelection()
{
	int X,Y;
	Selection++;
	if (Selection > 6)
		Selection = 1;
	X = Part->GetPlayFieldX();
	Y = Part->GetPlayFieldY();
	delete Part;
	switch (Selection)
	{
		case IDEmpty:
			Part = new CEmpty(X,Y);
			break;
		case IDBox:
			Part = new CBox(X,Y);
			break;
		case IDPlayer:
			Part = new CPlayer(X,Y);
			break;
		case IDSpot:
			Part = new CSpot(X,Y);
			break;
		case IDWall:
			Part = new CWall(X,Y);
			break;
		case IDFloor:
			Part = new CFloor(X,Y);
			break;
	}
	Part->Selected = true;
}

void CSelector::DecSelection()
{
	int X,Y;
	Selection--;
	if (Selection < 1)
		Selection = 6;
	X = Part->GetPlayFieldX();
	Y = Part->GetPlayFieldY();
	delete Part;
	switch (Selection)
	{
		case IDEmpty:
			Part = new CEmpty(X,Y);
			break;
		case IDBox:
			Part = new CBox(X,Y);
			break;
		case IDPlayer:
			Part = new CPlayer(X,Y);
			break;
		case IDSpot:
			Part = new CSpot(X,Y);
			break;
		case IDWall:
			Part = new CWall(X,Y);
			break;
		case IDFloor:
			Part = new CFloor(X,Y);
			break;
	}
	Part->Selected = true;
}

void CSelector::Draw(SDL_Surface *Surface)
{
	Part->Draw(Surface);
}

int CSelector::GetSelection()
{
	return Selection;
}

int CSelector::GetPlayFieldX()
{
	return Part->GetPlayFieldX();
}

int CSelector::GetPlayFieldY()
{
	return Part->GetPlayFieldY();
}

CSelector::~CSelector()
{
	delete Part;
}

bool CPlayer::CanMoveTo(const int PlayFieldXin,const int PlayFieldYin)
{
	int Teller;
	bool Result = true;
	if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
	{
		if (ParentList)
		{
			for (Teller=0;Teller<ParentList->ItemCount;Teller++)
				if((ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldXin) && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldYin))
				{
					if((ParentList->Items[Teller]->GetType() == IDWall))
					{
						Result = false;
						break;
					}
					if((ParentList->Items[Teller]->GetType() == IDBox))
					{
						if (PlayFieldX > PlayFieldXin)
						{
							Result = ParentList->Items[Teller]->CanMoveTo(PlayFieldXin-1,PlayFieldYin);
						}
						if (PlayFieldX < PlayFieldXin)
						{
							Result = ParentList->Items[Teller]->CanMoveTo(PlayFieldXin+1,PlayFieldYin);
						}
						if (PlayFieldY > PlayFieldYin)
						{
							Result = ParentList->Items[Teller]->CanMoveTo(PlayFieldXin,PlayFieldYin-1);
						}
						if (PlayFieldY < PlayFieldYin)
						{
							Result = ParentList->Items[Teller]->CanMoveTo(PlayFieldXin,PlayFieldYin+1);
						}
						break;
					}

				}
		}
	}
	else
		Result = false;
	return Result;
}
void CPlayer::Event_BeforeDraw()
{
	if (IsMoving)
	{
		AnimPhase = AnimBase + AnimCounter;
		AnimDelayCounter++;
		if (AnimDelayCounter ==AnimDelay)
		{
			AnimDelayCounter = 0;
			AnimCounter++;
			if (AnimCounter == AnimPhases)
				AnimCounter = 0;
		}
	}
/*	else
		AnimPhase = AnimBase;*/
}

void CPlayer::MoveTo(const int PlayFieldXin,const int PlayFieldYin,bool BackWards)
{
 	int Teller;
 	if(!IsMoving)
	{
		if(this->CanMoveTo(PlayFieldXin,PlayFieldYin) || BackWards)
		{
			PlayFieldX = PlayFieldXin;
			PlayFieldY = PlayFieldYin;
			if(X < PlayFieldX*TileWidth)
			{
				Xi = MoveSpeed;
				if(BackWards)
				{
					AnimBase = 0;
				}
				else
					if (ParentList)
					{
						for(Teller=0;Teller<ParentList->ItemCount;Teller++)
						{
							if(((ParentList->Items[Teller]->GetType() == IDBox) || (ParentList->Items[Teller]->GetType() == IDWall)) && ((ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldX) && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldY)))
							{
								ParentList->Items[Teller]->MoveTo(PlayFieldX+1,PlayFieldY,false);
								AnimBase = 6;
								break;
							}
							else
								AnimBase = 4;
						}
					}
					else
						AnimBase = 4;

			}
			if(X > PlayFieldX*TileWidth)
			{
				Xi = -MoveSpeed;
				if(BackWards)
				{
					AnimBase = 4;
				}
				else
					if (ParentList)
					{
						for(Teller=0;Teller<ParentList->ItemCount;Teller++)
						{
							if(((ParentList->Items[Teller]->GetType() == IDBox) || (ParentList->Items[Teller]->GetType() == IDWall)) && ((PlayFieldX == ParentList->Items[Teller]->GetPlayFieldX() )  && (ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldY)))
							{
								ParentList->Items[Teller]->MoveTo(PlayFieldX-1,PlayFieldY,false);
								AnimBase = 2;
								break;
							}
							else
								AnimBase = 0;
						}
					}
					else
						AnimBase = 0;
			}
			if(Y > PlayFieldY*TileHeight)
			{
				Yi = -MoveSpeed;
				if(BackWards)
				{
					AnimBase = 12;
				}
				else
					if (ParentList)
					{
						for(Teller=0;Teller<ParentList->ItemCount;Teller++)
						{
							if(((ParentList->Items[Teller]->GetType() == IDBox) || (ParentList->Items[Teller]->GetType() == IDWall)) && ((PlayFieldY == ParentList->Items[Teller]->GetPlayFieldY())  && (ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldX)))
							{
								ParentList->Items[Teller]->MoveTo(PlayFieldX,PlayFieldY-1,false);
								AnimBase = 10;
								break;
							}
							else
								AnimBase = 8;
						}
					}
					else
						AnimBase = 8;
			}
			if(Y < PlayFieldY*TileHeight)
			{
				Yi = MoveSpeed;
				if(BackWards)
				{
					AnimBase = 8;
				}
				else
					if(ParentList)
					{
						for(Teller=0;Teller<ParentList->ItemCount;Teller++)
						{
							if(((ParentList->Items[Teller]->GetType() == IDBox) || (ParentList->Items[Teller]->GetType() == IDWall)) && ((ParentList->Items[Teller]->GetPlayFieldY() == PlayFieldY)  && (ParentList->Items[Teller]->GetPlayFieldX() == PlayFieldX )))
							{
								ParentList->Items[Teller]->MoveTo(PlayFieldX,PlayFieldY+1,false);
								AnimBase = 14;
								break;
							}
							else
								AnimBase = 12;
						}
					}
					else
						AnimBase = 12;
			}
			IsMoving = true;
		}
		else
		{
			if (PlayFieldXin > PlayFieldX)
			{
				AnimBase= 6;
			}
			if (PlayFieldXin < PlayFieldX)
			{
				AnimBase = 2;
			}
			if (PlayFieldYin > PlayFieldY)
			{
				AnimBase = 14;
			}
			if (PlayFieldYin < PlayFieldY)
			{
				AnimBase = 10;
			}
			AnimPhase = AnimBase + AnimCounter;
			AnimDelayCounter++;
			if (AnimDelayCounter ==AnimDelay)
			{
				AnimDelayCounter = 0;
				AnimCounter++;
				if (AnimCounter == AnimPhases)
					AnimCounter = 0;
			}
		}

 	}
 }

CHistory::CHistory(CWorldPart *Partin)
{
	ItemCount = 0;
	Part = Partin;
}
void CHistory::Add(int X, int Y)
{
	SPrevPoint Temp;
	int Teller;
	Temp.X = X;
	Temp.Y = Y;
	if (ItemCount < MaxHistory)
	{
		Items[ItemCount] = Temp;
		ItemCount++;
	}
	else
	{
		for (Teller=0;Teller < ItemCount - 1 ;Teller++)
		{
			Items[Teller] = Items[Teller+1];
		}
		Items[ItemCount-1] = Temp;
	}
}

void CHistory::GoBack()
{
	if(ItemCount > 0)
	{
		ItemCount--;
		Part->MoveTo(Items[ItemCount].X,Items[ItemCount].Y,true);
	}
}


void CHistory::Print()
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
	{
		printf("History nr:%d X:%d Y:%d\n",Teller,Items[Teller].X,Items[Teller].Y);
	}
}

CWorldParts::CWorldParts()
{
	ItemCount = 0;
	DisableSorting = false;
}

void CWorldParts::RemoveAll()
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
	{
		delete Items[Teller];
		Items[Teller] = NULL;
	}
	ItemCount=0;
}
void CWorldParts::Remove(int PlayFieldXin,int PlayFieldYin)
{
	int Teller1,Teller2;
	for (Teller1=0;Teller1<ItemCount;Teller1++)
	{
		if ((Items[Teller1]->GetPlayFieldX() == PlayFieldXin) && (Items[Teller1]->GetPlayFieldY() == PlayFieldYin))
		{
			delete Items[Teller1];
			for (Teller2=Teller1;Teller2<ItemCount-1;Teller2++)
				Items[Teller2] = Items[Teller2+1];
			ItemCount--;
			Teller1--;
		}
	}
}

void CWorldParts::Remove(int PlayFieldXin,int PlayFieldYin,int Type)
{
	int Teller1,Teller2;
	for (Teller1=0;Teller1<ItemCount;Teller1++)
	{
		if ((Items[Teller1]->GetPlayFieldX() == PlayFieldXin) && (Items[Teller1]->GetPlayFieldY() == PlayFieldYin) && (Items[Teller1]->GetType() == Type))
		{
			delete Items[Teller1];
			for (Teller2=Teller1;Teller2<ItemCount-1;Teller2++)
				Items[Teller2] = Items[Teller2+1];
			ItemCount--;
			Teller1--;
		}
	}
}

void CWorldParts::HistoryAdd()
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
		Items[Teller]->HistoryAdd();
}

void CWorldParts::HistoryGoBack()
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
		Items[Teller]->HistoryGoBack();
}

void CWorldParts::Add(CWorldPart *WorldPart)
{
	if( ItemCount < NrOfRows*NrOfCols*3 )
	{
		WorldPart->ParentList = this;
		Items[ItemCount] = WorldPart;
		ItemCount++;
		Sort();
	}
}


void CWorldParts::Sort()
{
	int Teller1,Teller2,Index;
	CWorldPart *Part;
	if (!DisableSorting)
	{
		for (Teller1 = 1; Teller1 <ItemCount;Teller1++)
		{
			Index = Items[Teller1]->GetZ();
			Part = Items[Teller1];
			Teller2 = Teller1;
			while ((Teller2 > 0) && (Items[Teller2-1]->GetZ() > Index))
			{
				Items[Teller2] = Items[Teller2 - 1];
				Teller2--;
			}
			Items[Teller2] = Part;
		}
	}

}

void CWorldParts::Save(char *Filename)
{
	FILE *Fp;
	int Teller,BufferPosition=0;
	char *Buffer;
	Buffer = new char[3*ItemCount];
	for (Teller=0;Teller<ItemCount;Teller++)
	{
		Buffer[BufferPosition]= (char) Items[Teller]->GetType();
		Buffer[BufferPosition+1] = (char)Items[Teller]->GetPlayFieldX();
		Buffer[BufferPosition+2]= (char)Items[Teller]->GetPlayFieldY();
		BufferPosition += 3;
	}
	Fp = fopen(Filename,"wb");
	if (Fp)
	{
		fwrite(Buffer,1,3*ItemCount,Fp);
		fclose(Fp);



	}
	delete[] Buffer;
}

void CWorldParts::Load(char *Filename)
{
	int X,Y,Type;
	FILE *Fp;
	Uint32 StartTime=0;
	int BufferPosition=0;
	long FileSize;
	char *Buffer;
	CWorldPart *Part;
	Fp = fopen(Filename,"rb");
	if(Fp)
	{
		RemoveAll();
		DisableSorting=true;
		StartTime = SDL_GetTicks();
		fseek (Fp , 0 , SEEK_END);
  		FileSize = ftell (Fp);
  		rewind (Fp);
		Buffer = new char[FileSize];
		fread(Buffer,1,FileSize,Fp);
		while(BufferPosition < FileSize)
		{
			Type = (int)Buffer[BufferPosition];
			X =(int)Buffer[BufferPosition+1];
			Y = (int)Buffer[BufferPosition+2];
			BufferPosition +=3;
			switch(Type)
			{
				case IDPlayer:
					Add(new CPlayer(X,Y));
					break;
				case IDBox:
					Add(new CBox(X,Y));
					break;
				case IDSpot:
					Add(new CSpot(X,Y));
					break;
				case IDWall:
					Add(new CWall(X,Y));
					break;
				case IDFloor:
					Add(new CFloor(X,Y));
					break;

			}
		}
		delete[] Buffer;
		fclose(Fp);
		DisableSorting=false;
		Sort();
		//printf("Loading:%d\n",SDL_GetTicks() - StartTime);
	}

}

void CWorldParts::Move()
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
		Items[Teller]->Move();
}

void CWorldParts::Draw(SDL_Surface *Surface)
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
		Items[Teller]->Draw(Surface);
}

CWorldParts::~CWorldParts()
{
	int Teller;
	for (Teller=0;Teller<ItemCount;Teller++)
	{
		delete Items[Teller];
		Items[Teller] = NULL;
	}
	//printf("Deleted worldparts\n");
}

//----------------------------------------------------------------------------------------------------------------------------------------

unsigned char HashTable[1004] = {0xE4,0xF6,0xD7,0xA4,0xA5,0xC3,0xA3,0xE5,0xA4,0xF8,0xA3,0xF6,0xB5,0x98,0xA3,0xA7,0xB9,0xA0,0xD9,0xD6,0xE6,0x29,0x49,0xF8,0xB9,0xF0,0x79,0xA6,0x88,0xA8,0xC6,0xC2,0xF2,0xB7,0x79,0xC0,0xF0,0xD0,0xA8,0xE1,0xE1,0xF9,0xA5,0xE5,0xF1,0xE4,0xC7,0xE8,0xD9,0xB7,0xC0,0xE1,0xC0,0xB4,0xA9,0xF8,0xF8,0xF8,0xA1,0xA4,0xD0,0xC6,0x06,0xB9,0xA1,0xB0,0xF0,0xE8,0xB0,0xE5,0xC3,0xC8,0xA1,0xB2,0xF1,0xE2,0xE0,0xF0,0xF8,0xA5,0xE6,0xF2,0xB4,0xF7,0x79,0xB5,0x75,0xB0,0xF1,0xA6,0xC9,0xE2,0xB6,0x68,0xD9,0xA0,0xE7,0xD1,0xB4,0x51,0xD9,0xE9,0x96,0x61,0xD2,0xC2,0xC2,0x06,0x47,0x86,0x88,0xF4,0xC7,0x88,0xA0,0xA9,0xB1,0xD2,0xE9,0xF1,0x93,0xF4,0x35,0xC6,0xB2,0xC8,0xB8,0xB0,0xA8,0xE9,0xC1,0xA4,0xF5,0xD6,0xD1,0xC8,0xD3,0xA9,0xE9,0xA9,0xF4,0xB4,0xA3,0x57,0xE1,0xC5,0xF1,0xB7,0xB6,0xB2,0xE3,0x45,0x64,0xE3,0xD9,0xE5,0xB5,0xE2,0xE8,0xC8,0xB5,0xE7,0xD3,0xB0,0xD2,0xF7,0x81,0xB2,0xE4,0xA8,0x00,0xC9,0xD5,0xD4,0x86,0xC9,0xB1,0xA2,0xA0,0xD3,0xE3,0xD5,0xF7,0xE0,0xF9,0xE5,0xE2,0xD1,0xA5,0xD4,0xF4,0xC6,0xB4,0xD3,0xA2,0xD6,0xD3,0x35,0xE4,0xE3,0xC4,0xB6,0x18,0xD7,0xE7,0xC5,0xE2,0x59,0xA0,0xB5,0xA7,0xC5,0xF6,0xA7,0xC2,0xD7,0xA2,0xB5,0xB9,0xF6,0x81,0xA3,0xA5,0x00,0xA2,0xA8,0xE7,0xD5,0x87,0xC8,0xE9,0xD7,0xE6,0xA0,0x54,0xF9,0xF9,0xF4,0xB4,0xF4,0xE9,0xA7,0xD0,0x74,0xD0,0xE7,0x24,0xD9,0xB0,0xE8,0xE3,0xF1,0xF7,0xF7,0x88,0x97,0xC0,0xE0,0xD2,0xF3,0xC2,0xA1,0xC4,0xE1,0xF8,0xE8,0xB1,0x76,0x97,0xE7,0xB6,0x96,0xA7,0xD8,0xA4,0xF1,0xA0,0xF3,0xE8,0xE7,0x76,0xE3,0xB1,0xC9,0xE4,0xB4,0xF0,0xA4,0xD0,0x28,0xB8,0xF7,0xD9,0xD3,0xE5,0xC9,0x81,0xF7,0xC0,0xD9,0xC9,0xE3,0xA8,0xD6,0xB9,0xA7,0xF9,0xD0,0xD8,0x54,0xF6,0xF9,0xB8,0xC8,0xC9,0xB9,0xF0,0xF4,0xD1,0xA4,0xD0,0xE6,0xA8,0xB6,0xD2,0xB0,0xE9,0xE9,0xE9,0xB7,0xD9,0xD4,0xE6,0xF0,0xA4,0xC7,0xF1,0xC0,0xC2,0xA8,0xA7,0x14,0x25,0xA1,0xB6,0xB9,0xD5,0x78,0xC1,0xD1,0xC6,0x93,0xE8,0xE7,0xE7,0xA9,0xA2,0xA8,0x89,0xA1,0xA6,0xF9,0xA8,0xF7,0xE4,0xD8,0xA2,0xD2,0xB0,0x39,0xD1,0xC5,0x72,0xB0,0xD6,0xC6,0xA0,0xE9,0xE5,0xC1,0xC2,0xF9,0xF3,0xB2,0xC8,0xD0,0xD2,0xD1,0xC1,0xD7,0x53,0x93,0xB4,0xF9,0xB0,0xD0,0xD4,0xF5,0xE4,0xE4,0x13,0xB2,0xE9,0x70,0xB3,0xB2,0xD8,0xF0,0xC0,0xC1,0xB0,0xE8,0xA3,0xE7,0x33,0xB1,0xC0,0xA7,0x17,0xD4,0xA9,0xE5,0xF6,0xF3,0xF3,0xA3,0xE0,0xD5,0xE2,0x71,0xB7,0xB4,0xA9,0xD6,0xC1,0xC5,0xA3,0xB2,0xD7,0xB6,0xE8,0xC5,0xF0,0x10,0xA6,0xB0,0xC3,0xA3,0xA0,0xB2,0xF6,0xB9,0xF4,0xD8,0xD5,0xF8,0xB9,0xE3,0x70,0xD1,0xF1,0xA6,0xE6,0xE5,0xA8,0xD5,0xC9,0xC1,0xA5,0xE5,0x61,0xF4,0xB2,0xA9,0xA4,0xF2,0xE8,0xB7,0xD7,0xA1,0xD0,0xF4,0xF9,0xE4,0xE8,0xC6,0xB3,0xC4,0xB2,0xC9,0xE3,0x49,0xC3,0x62,0xA6,0xF3,0xB0,0x57,0x65,0xC4,0xF6,0xB9,0x23,0xB8,0xB3,0xA9,0xE6,0xB7,0xA5,0xE8,0xF6,0xC9,0xE7,0x63,0xF7,0xD9,0xA5,0xA5,0xD2,0xD4,0xA6,0xF2,0x94,0x04,0xB6,0x85,0xA3,0xD3,0x59,0xA3,0xA8,0x69,0xF1,0xB9,0xD1,0xD1,0xE5,0xE5,0x57,0xB0,0xD5,0x59,0xD4,0xD9,0x52,0xB4,0xA8,0x75,0xF0,0xE0,0x20,0xA1,0xE7,0x69,0xA2,0xC0,0xB1,0xA3,0xE5,0xB9,0xF5,0xB9,0xB5,0xE1,0xF3,0xB7,0x56,0xA1,0xA4,0xA9,0xA4,0xE7,0xC5,0xC7,0xC7,0x32,0xB7,0xA6,0xB4,0xB2,0xF6,0xB1,0xC4,0xB5,0xC4,0xC1,0xE1,0x97,0xB1,0xE3,0x37,0xA6,0xD8,0xC3,0xF1,0xD5,0xE9,0xF2,0xE2,0xF6,0xF3,0xF6,0xF4,0x51,0xE7,0xE9,0xC1,0xE8,0xA8,0x69,0xD3,0xB4,0xD8,0xD6,0xE8,0xD8,0x47,0xE1,0xA8,0xA0,0x94,0xB5,0xC2,0xD3,0xF6,0x28,0xF5,0x21,0xD7,0x48,0xC4,0xD8,0xF2,0xC7,0xC0,0xA7,0xF6,0xC7,0xE7,0x64,0xB5,0xD5,0x97,0xA0,0xD6,0xF7,0xC4,0xF7,0xD0,0xD7,0xA8,0xD7,0x28,0xC2,0xE5,0xD0,0xC2,0xF2,0xC8,0xF9,0xE5,0x08,0xD3,0xA0,0xA0,0xB2,0xD3,0x29,0xF0,0xB1,0xD0,0xB2,0xD7,0xF1,0x47,0xF2,0xD4,0xD3,0x53,0x92,0xC4,0xF9,0xA2,0xE8,0xC3,0xA9,0xF8,0xB1,0xE4,0x90,0xD0,0xE4,0x26,0x18,0xB4,0xF8,0xE7,0xC7,0xE0,0xF9,0xC1,0x36,0xE3,0xC2,0xE6,0xC9,0xC2,0xF8,0xA1,0x26,0xE7,0xC1,0x57,0xE9,0xD1,0xC8,0xC3,0xB2,0xE1,0x86,0xA5,0xE8,0xB4,0xA3,0xB5,0xD7,0x89,0xD5,0xF7,0xD0,0xD6,0xD7,0xB4,0x41,0xE1,0xA2,0xD5,0xF1,0xA8,0xF5,0xF7,0xC8,0xA3,0xB1,0xC0,0xC7,0x83,0x65,0xE6,0x46,0xA8,0xA3,0xE8,0xC5,0xF9,0xC2,0xD6,0xA5,0xD5,0xA5,0xA8,0xB6,0xE0,0xD0,0xC4,0xE1,0xF0,0x08,0x06,0x45,0xA7,0xF1,0xA1,0xE8,0xF3,0xF1,0xE2,0xC4,0xA6,0xF0,0xF3,0xD3,0xB2,0xE5,0xC5,0x85,0xA6,0xC7,0xD5,0xD4,0xD7,0xC2,0xC8,0xC8,0xD2,0xB8,0xA4,0x79,0x06,0xF7,0xE0,0xB6,0xF4,0xF4,0xC2,0xD1,0xB4,0xA4,0xB5,0xA8,0xC0,0xB2,0xB9,0xF7,0xC0,0xD6,0xD6,0xC3,0xF5,0xA3,0xF5,0xA7,0xE5,0xE1,0xB0,0xA5,0xE0,0xC3,0x41,0xD3,0xB2,0xF9,0xF2,0x19,0xB3,0xA7,0x58,0x40,0xB1,0x86,0xD5,0x71,0xF8,0xF6,0xD2,0xF2,0xA5,0xF4,0xB7,0x80,0xE5,0x03,0xF8,0x14,0xF5,0xE4,0xE7,0xE2,0xB9,0xA6,0xB5,0xE2,0xB5,0xD8,0xC7,0xE5,0xC5,0xF9,0xD4,0xD3,0xE5,0xD9,0xE8,0x79,0xD2,0xE3,0xF0,0xA1,0xF8,0xD8,0x31,0x78,0xB9,0xA9,0x41,0xC1,0xB1,0xC6,0x52,0xC5,0xA7,0xD0,0xF9,0xF5,0xA8,0xF6,0xA3,0xA3,0xD0,0xA5,0xF3,0xB1,0xB1,0x31,0xE0,0xC5,0xA7,0x29,0xC0,0xE1,0xD4,0xF7,0x15,0xC2,0xF3,0xA1,0xC9,0xF3,0xE0,0xA5,0xB6,0xE6,0xE5,0xB1,0xA3,0xB3,0xE7,0xF5,0xD8,0x93,0xF8,0xB2,0x94,0xC8,0xD2,0xE2,0xB0,0xF9,0xF1,0xB4,0xE0,0xD5,0x65,0x81,0xC4,0xC7,0xD0,0xE8,0xA7,0xA2,0xF7,0x45,0xA0,0xE9,0xC5,0xB6,0xE2,0xF2,0xE6,0xA7,0xE8,0x28,0xA6,0xE5,0x90,0xF0,0xD6,0xE1,0xC4,0xD2,0xD5,0xF0,0xA5,0xB1,0xC6,0xA0,0xC1,0xF5,0xB1,0xA4,0xD3,0xF3,0xA3,0xF3,0x10,0xE8,0xA9,0xC1,0xB2,0xB8,0xA5,0xD4,0x41,0xB4,0xC6,0xF7,0xC6,0xF9,0xD9,0xD0,0xC2,0xE9,0xD5,0xC7};
SDL_Surface* Screen,*Buffer,* tmp,*tmp1;
TTF_Font* font,* BigFont,*MonoFont;
SDL_Joystick *Joystick;
GameStates GameState = GSTitleScreen;
int MusicCount=0,SelectedMusic=0,InstalledLevelPacksCount=0,InstalledLevels=0,SelectedLevel=0,SelectedLevelPack=0,UnlockedLevels=1;
bool GlobalSoundEnabled = true,LevelEditorMode=false,LevelHasChanged=false,StageReload=false;
SDL_Color MenuBoxColor = {187,164,134,255}, MenuBoxBorderColor = {153,123,87,255}, MenuTextColor = {127,82,33,0};
CWorldParts WorldParts;
Uint32 NextTime=0;
char LevelPackName[21]= "";
char LevelPackFileName[21] = "";
char InstalledLevelPacks[MaxLevelPacks][21];
Mix_Music *Music[MaxMusicFiles];
Mix_Chunk *Sounds[NrOfSounds];

Uint32 WaitForFrame()
{
	Uint32 Result,Now;
	Now = SDL_GetTicks();
	if (Now >= NextTime)
	{
		Result = 0;
		NextTime = Now + 1000/FPS;
	}
	else
		Result = NextTime - Now;
	return Result;
}


void WriteText(SDL_Surface* Surface,TTF_Font* FontIn,char* Tekst,int NrOfChars,int X,int Y,int YSpacing,SDL_Color ColorIn)
{
	char List[100][255];
	int Lines,Teller,Chars;
	SDL_Rect DstRect;
	SDL_Surface* TmpSurface1,*TmpSurface2;
	Lines = 0;
	Chars = 0;
	for (Teller=0;Teller<NrOfChars;Teller++)
	{
		if(Lines > 100)
			break;
		if((Tekst[Teller] == '\n') || (Chars==255))
		{
			List[Lines][Chars]='\0';
			Lines++;
			Chars = 0;
		}
		else
		{
		 	List[Lines][Chars]=Tekst[Teller];
		 	Chars++;
		}
	}
	List[Lines][Chars] = '\0';
	for (Teller=0;Teller <= Lines;Teller++)
	{
		if(strlen(List[Teller]) > 0)
		{
			TmpSurface1 = TTF_RenderText_Blended(FontIn,List[Teller],ColorIn);
			DstRect.x = X;
			DstRect.y = Y + (Teller) * TTF_FontLineSkip(FontIn) + (Teller*YSpacing);
			DstRect.w = TmpSurface1->w;
			DstRect.h = TmpSurface1->h;
			SDL_BlitSurface(TmpSurface1,NULL,Surface,&DstRect);
			SDL_FreeSurface(TmpSurface1);
		}
	}
}

void AddUnderScores (char *string)
{
	int Teller;
	for(Teller=0;Teller<strlen(string);Teller++)
		if(string[Teller] == ' ')
			string[Teller] = '_';
}

void RemoveUnderScores (char *string)
{
	int Teller;
	for(Teller=0;Teller<strlen(string);Teller++)
		if(string[Teller] == '_')
			string[Teller] = ' ';
}

void SearchForMusic()
{
	struct dirent *Entry;
	DIR *Directory;
	struct stat Stats;
	int Teller;
	char FileName[FILENAME_MAX];
	Music[0] = Mix_LoadMUS("./music/title.mod");
	Teller=1;
	Directory = opendir("./music");
	if (Directory)
	{
		Entry=readdir(Directory);
		while(Entry)
		{
			sprintf(FileName,"./music/%s",Entry->d_name);
			stat(FileName,&Stats);
			if(!S_ISDIR(Stats.st_mode))
			{
				if(strncmp(".", Entry->d_name, 1) && (strcmp("title.mod",Entry->d_name) != 0) && (Teller< MaxMusicFiles))
				{
				//	printf("%s\n",Entry->d_name);
					Music[Teller] = Mix_LoadMUS(FileName);
					Teller++;
				}
			}
			Entry=readdir(Directory);
		}
		closedir(Directory);
	}
	MusicCount = Teller;
	if (MusicCount > 1)
		SelectedMusic =	1+rand()%(MusicCount-1);
	else
		SelectedMusic = 1;
}

void SearchForLevelPacks()
{
	struct dirent *Entry;
	DIR *Directory;
	struct stat Stats;
	int Teller=0;
	char FileName[FILENAME_MAX];
	Directory = opendir("./levelpacks");
	if (Directory)
	{
		Entry=readdir(Directory);
		while(Entry)
		{
			sprintf(FileName,"./levelpacks/%s",Entry->d_name);
			stat(FileName,&Stats);
			if(S_ISDIR(Stats.st_mode))
			{
				//printf("%s\n",Entry->d_name);
				if(strncmp(".", Entry->d_name, 1)  && (Teller< MaxLevelPacks) && (strlen(Entry->d_name) < 21))
				{
					sprintf(InstalledLevelPacks[Teller],"%s",Entry->d_name);
					RemoveUnderScores(InstalledLevelPacks[Teller]);
					Teller++;
				}
			}
			Entry=readdir(Directory);
		}
		closedir(Directory);
	}
	InstalledLevelPacksCount = Teller;
	SelectedLevelPack=0;
	if (InstalledLevelPacksCount > 0)
	{
		sprintf(LevelPackName,"%s",InstalledLevelPacks[SelectedLevelPack]);
		sprintf(LevelPackFileName,"%s",InstalledLevelPacks[SelectedLevelPack]);
		AddUnderScores(LevelPackFileName);

	}
	else
	{
		sprintf(LevelPackName,"%s","");
		sprintf(LevelPackFileName,"%s","");
	}
}

bool StageDone()
{
	bool Temp = true;
	int Teller,FilledSpots=0,Spots=0;
	for (Teller=0;Teller<WorldParts.ItemCount;Teller++)
		if (WorldParts.Items[Teller]->GetType() == IDBox)
		{
			if (WorldParts.Items[Teller]->GetAnimPhase() == 1)
                FilledSpots++;
		}
		else
            if (WorldParts.Items[Teller]->GetType() == IDSpot)
                Spots++;
    if (FilledSpots >= Spots)
        return true;
    else
        return false;
}

bool AskQuestion(char *Msg)
{
	SDL_Event Event;
	SDL_Color Color = {0,0,0,0};
	boxRGBA(Buffer,60,80,260,160,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
	rectangleRGBA(Buffer,60,80,260,160,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
	rectangleRGBA(Buffer,61,81,259,159,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
	WriteText(Buffer,font,Msg,strlen(Msg),65,85,2,MenuTextColor);
	tmp = zoomSurface(Buffer,2,2,0);
	tmp1 = SDL_DisplayFormat(tmp);
	SDL_BlitSurface(tmp1,NULL,Screen,NULL);
	SDL_Flip(Screen);
	SDL_FreeSurface(tmp);
	SDL_FreeSurface(tmp1);
	SDL_WaitEvent(&Event);
	{
		while (!((Event.type == SDL_KEYDOWN) && ((Event.key.keysym.sym == SDLK_a) || (Event.key.keysym.sym ==  SDLK_q) || (Event.key.keysym.sym ==  SDLK_n) || (Event.key.keysym.sym ==  SDLK_y) || (Event.key.keysym.sym ==  SDLK_BACKSPACE) || (Event.key.keysym.sym ==  SDLK_x) || (Event.key.keysym.sym == SDLK_SPACE) || (Event.key.keysym.sym == SDLK_RETURN) )))
			SDL_WaitEvent(&Event);
		if (((Event.type == SDL_KEYDOWN)&&((Event.key.keysym.sym == SDLK_a) || (Event.key.keysym.sym == SDLK_q) || (Event.key.keysym.sym == SDLK_SPACE) || (Event.key.keysym.sym == SDLK_RETURN) || (Event.key.keysym.sym ==  SDLK_y) )))
			return true;
		else
			return false;

	}
}

void PrintForm(char *msg)
{
	SDL_Event Event;
	SDL_Color Color = {0,0,0,0};
	boxRGBA(Buffer,60,80,260,160,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
	rectangleRGBA(Buffer,60,80,260,160,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
	rectangleRGBA(Buffer,61,81,259,159,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
	WriteText(Buffer,font,msg,strlen(msg),65,85,2,MenuTextColor);
	tmp = zoomSurface(Buffer,2,2,0);
	tmp1 = SDL_DisplayFormat(tmp);
	SDL_BlitSurface(tmp1,NULL,Screen,NULL);
	SDL_Flip(Screen);
	SDL_FreeSurface(tmp);
	SDL_FreeSurface(tmp1);
	SDL_WaitEvent(&Event);
	{
		while (!((Event.type == SDL_KEYDOWN) && ((Event.key.keysym.sym == SDLK_a) || (Event.key.keysym.sym == SDLK_q) || (Event.key.keysym.sym == SDLK_SPACE) || (Event.key.keysym.sym == SDLK_RETURN || (Event.key.keysym.sym == SDLK_y)) ) ))
			SDL_WaitEvent(&Event);
	}
}

void SaveUnlockData()
{
	FILE *Fp;
	int Teller;
	char Filename[FILENAME_MAX];
	unsigned char LevelHash[4];
	unsigned char Buffer[64];
	int CheckSum = 0;
	LevelHash[0] = HashTable[UnlockedLevels-1] ;
	LevelHash[1] = HashTable[UnlockedLevels];
	LevelHash[2] = HashTable[UnlockedLevels+1];
	LevelHash[3] = HashTable[UnlockedLevels+2];
	sprintf(Filename,"./%s.dat",LevelPackFileName);
	for (Teller=0;Teller<4;Teller++)
		LevelHash[Teller] = LevelHash[Teller] ^ LevelPackFileName[strlen(LevelPackFileName)-1];
	for (Teller=0;Teller<strlen(LevelPackFileName);Teller++)
		LevelHash[Teller%4] = LevelHash[Teller%4] ^ LevelPackFileName[Teller];
	LevelHash[0] = LevelHash[0] ^ LevelHash[2];
	LevelHash[1] = LevelHash[1] ^ LevelHash[0];
	LevelHash[2] = LevelHash[2] ^ LevelHash[3];
	LevelHash[3] = LevelHash[3] ^ LevelHash[2];
	for (Teller=0;Teller<64;Teller++)
		if ((Teller+1) % 16 == 0)
		{
			Buffer[Teller] = LevelHash[(Teller)/16];
			CheckSum += Buffer[Teller];
		}
		else
		{
			Buffer[Teller] = rand() % 256;
			CheckSum += Buffer[Teller];
		}
	CheckSum = CheckSum ^ 50;
	Fp = fopen(Filename,"wb");
	if (Fp)
	{
		fwrite(Buffer,1,64,Fp);
		fwrite(&CheckSum,sizeof(int),1,Fp);
		fclose(Fp);
	}
}

void SaveAllHashes()
{
	FILE *Fp;
	char Filename[FILENAME_MAX];
	char LevelHash[4];
	int Teller;
	sprintf(Filename,"./hashes.txt");
	Fp = fopen(Filename,"wt");
	if (Fp)
	{
		for (Teller=0;Teller<1000;Teller++)
		{
			LevelHash[0] = HashTable[Teller];
			LevelHash[1] = HashTable[Teller+1];
			LevelHash[2] = HashTable[Teller+2];
			LevelHash[3] = HashTable[Teller+3];

			fprintf(Fp,"%d|%d|%d|%d\n",(int)LevelHash[0],(int)LevelHash[1],(int)LevelHash[2],(int)LevelHash[3]);

		}
	fclose(Fp);
	}
}


void LoadUnlockData()
{
	FILE *Fp;
	unsigned char LevelHash[4];
	int Teller=0;
	unsigned char Buffer[64];
	char Filename[FILENAME_MAX];
	sprintf(Filename,"./%s.dat",LevelPackFileName);
	Fp = fopen(Filename,"rb");
	int CheckSum,ValidCheckSum=0;
	if (Fp)
	{
		fflush(Fp);
		fread(Buffer,1,64,Fp);
		fread(&CheckSum,sizeof(int),1,Fp);
		fclose(Fp);
		for (Teller = 0 ;Teller<64;Teller++)
		{
			ValidCheckSum += Buffer[Teller];

			if ((Teller+1)%16 == 0)
				{
					LevelHash[Teller/16] = Buffer[Teller];
				}
		}

		CheckSum = CheckSum ^ 50;
		if (CheckSum == ValidCheckSum)
		{
			LevelHash[3] = LevelHash[3] ^ LevelHash[2];
			LevelHash[2] = LevelHash[2] ^ LevelHash[3];
			LevelHash[1] = LevelHash[1] ^ LevelHash[0];
			LevelHash[0] = LevelHash[0] ^ LevelHash[2];
			for (Teller=0;Teller<strlen(LevelPackFileName);Teller++)
				LevelHash[Teller%4] = LevelHash[Teller%4] ^ LevelPackFileName[Teller];
			for (Teller=0;Teller<4;Teller++)
				LevelHash[Teller] = LevelHash[Teller] ^ LevelPackFileName[strlen(LevelPackFileName)-1];
			Teller=0;
			while ((LevelHash[0] != HashTable[Teller]) || (LevelHash[1] != HashTable[Teller+1]) || 	(LevelHash[2] != HashTable[Teller+2]) || (LevelHash[3] != HashTable[Teller+3]) && (Teller+3 < 1004))
			{
				Teller++;
			}
			if (Teller < InstalledLevels)
				UnlockedLevels = Teller+1;
			else
				UnlockedLevels = 1;
		}
		else
			UnlockedLevels = 1;
	}
	else
	 	UnlockedLevels = 1;

}

void Game()
{
	//SDL_Color black = {0,0,0,0};
	//Uint32 StartTime=0,EndTime=0;
	int Teller,Moves=0;
	char ChrDebug[200];
	char Msg[300];
	char FileName[FILENAME_MAX];
	SDL_Event Event;
	SDL_PollEvent(&Event);
	Mix_HaltMusic();
	Moves=0;
	Uint8 *KeyState;
	if (MusicCount > 0)
		if(GlobalSoundEnabled)
			Mix_PlayMusic(Music[SelectedMusic],-1);
	CWorldPart *Player=0;
	for (Teller=0;Teller<WorldParts.ItemCount;Teller++)
	{
		if (WorldParts.Items[Teller]->GetType() == IDPlayer)
		{
			Player = WorldParts.Items[Teller];
			break;
		}
	}
	//should never happen
	if(!Player)
	{
		Player = new CPlayer(0,0);
		WorldParts.Add(Player);
	}

	while (GameState == GSGame)
	{
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				GameState = GSQuit;
			if (Event.type == SDL_KEYDOWN)
			{
				switch (Event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						if(!LevelEditorMode)
							GameState = GSStageSelect;
						break;
					case SDLK_RETURN:
						if(LevelEditorMode)
							GameState=GSLevelEditor;
						break;
					case SDLK_PAGEUP:
					case SDLK_x:
					case SDLK_KP_PLUS:
						SelectedMusic++;
						if (SelectedMusic >= MusicCount)
							SelectedMusic = 1;
						Mix_HaltMusic();
						if(GlobalSoundEnabled)
							Mix_PlayMusic(Music[SelectedMusic],-1);
						break;
					case SDLK_PAGEDOWN:
					case SDLK_w:
					case SDLK_KP_MINUS:
					case SDLK_z:
						SelectedMusic--;
						if (SelectedMusic <= 0)
							SelectedMusic = MusicCount-1;
						Mix_HaltMusic();
						if(GlobalSoundEnabled)
							Mix_PlayMusic(Music[SelectedMusic],-1);
						break;
					case SDLK_r:
						SDL_BlitSurface(IMGBackground,NULL,Buffer,NULL);
						WorldParts.Draw(Buffer);
						if(AskQuestion("You are about to restart this level\nAre you sure you want to restart?\n\nPress (A) to Restart (X) to Cancel"))
						{
							sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
							WorldParts.Load(FileName);
							Moves=0;
							for (Teller=0;Teller<WorldParts.ItemCount;Teller++)
							{
								if (WorldParts.Items[Teller]->GetType() == IDPlayer)
								{
									Player = WorldParts.Items[Teller];
									break;
								}
							}
						}
						break;
					case SDLK_s:
						SDL_BlitSurface(IMGBackground,NULL,Buffer,NULL);
						WorldParts.Draw(Buffer);
						sprintf(Msg,"Level Pack: %s\nLevel: %d/%d - Moves: %d\n\nPress (A) To continue playing",LevelPackName,SelectedLevel,InstalledLevels,Moves);
						PrintForm(Msg);
						SDL_Delay(250);
						break;
					default:break;
				}
			}
		}
		//printf("Start while\n");
		if (!Player->IsMoving)
		{
			KeyState = SDL_GetKeyState(NULL);
			if (KeyState[SDLK_BACKSPACE] || KeyState[SDLK_a] || KeyState[SDLK_q] || KeyState[SDLK_SPACE])
			{
				WorldParts.HistoryGoBack();
				if (Moves > 0)
					Moves--;
			}
			if (KeyState[SDLK_RIGHT])
			{
				if (Player->CanMoveTo(Player->GetPlayFieldX() + 1, Player->GetPlayFieldY()))
				{
					WorldParts.HistoryAdd();
					Moves++;
				}
				Player->MoveTo(Player->GetPlayFieldX() + 1, Player->GetPlayFieldY(),false);

			}
			if (KeyState[SDLK_LEFT])
			{
				if (Player->CanMoveTo(Player->GetPlayFieldX() - 1, Player->GetPlayFieldY()))
				{
					WorldParts.HistoryAdd();
					Moves++;
				}
				Player->MoveTo(Player->GetPlayFieldX() - 1, Player->GetPlayFieldY(),false);
			}
			if (KeyState[SDLK_UP])
			{
				if (Player->CanMoveTo(Player->GetPlayFieldX() , Player->GetPlayFieldY()-1))
				{
					WorldParts.HistoryAdd();
					Moves++;
				}
				Player->MoveTo(Player->GetPlayFieldX(), Player->GetPlayFieldY() - 1,false);
			}
			if (KeyState[SDLK_DOWN])
			{
				if (Player->CanMoveTo(Player->GetPlayFieldX() , Player->GetPlayFieldY()+1))
				{
					WorldParts.HistoryAdd();
					Moves++;
				}
				Player->MoveTo(Player->GetPlayFieldX(), Player->GetPlayFieldY() +1,false);
			}
		}

		//SDL_FillRect(Screen,NULL,SDL_MapRGB(Screen->format,255,255,255));
		SDL_BlitSurface(IMGBackground,NULL,Buffer,NULL);

		//printf("voor Move all objects\n");
		WorldParts.Move();
		//printf("voor Draw all objects\n");
		WorldParts.Draw(Buffer);
		//StartTime = EndTime;
		//EndTime = SDL_GetTicks();
		//sprintf(ChrDebug,"fps %d",1000 / (EndTime-StartTime));
		//WriteText(Buffer,font,ChrDebug,strlen(ChrDebug),0,0,0,black);
		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		//printf("voor fps show: Endtime:%d StartTime:%d End-start:%d\n",EndTime,StartTime,EndTime-StartTime);
		//printf("Na Fps Show\n");
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);
		if (!Player->IsMoving && StageDone())
		{
			SDL_Delay(250);
			if (GlobalSoundEnabled)
				Mix_PlayChannel(-1,Sounds[SND_STAGEEND],0);
			if (LevelEditorMode)
			{
				if (AskQuestion("Congratulations !\nYou Succesfully Solved this level\nDo you want to return to the\nlevel editor ?\n(A) Leveleditor (X) Play Again"))
					GameState = GSLevelEditor;
				else
				{
					sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
					WorldParts.Load(FileName);
					Moves=0;
					for (Teller=0;Teller<WorldParts.ItemCount;Teller++)
					{
						if (WorldParts.Items[Teller]->GetType() == IDPlayer)
						{
							Player = WorldParts.Items[Teller];
							break;
						}
					}
				}

			}
			else
			{
				if (SelectedLevel == UnlockedLevels)
				{
					if ( UnlockedLevels < InstalledLevels)
					{
						sprintf(Msg,"Congratulations !\nYou Succesfully Solved Level %d/%d\nThe next level has now been unlocked!\n\nPress (A) to continue",SelectedLevel,InstalledLevels);
						PrintForm(Msg);
						UnlockedLevels++;
						SelectedLevel++;
						SaveUnlockData();
						GameState = GSStageSelect;
					}
					else
					{
						sprintf(Msg,"Congratulations !\nYou Succesfully Solved Level %d/%d\nlevelpack %s\nis now finished, try out another one!\n\nPress (A) to continue",SelectedLevel,InstalledLevels,LevelPackName);
						PrintForm(Msg);
						GameState = GSTitleScreen;
					}
				}
				else
				{
					sprintf(Msg,"Congratulations !\nYou Succesfully Solved Level %d/%d\n\nPress (A) to continue",SelectedLevel,InstalledLevels);
					PrintForm(Msg);
					GameState = GSStageSelect;

				}
			}
		}
		SDL_Delay(WaitForFrame());
		//printf("Start End while\n");
	}
	Mix_HaltMusic();
	SelectedMusic++;
	if (SelectedMusic >= MusicCount)
		SelectedMusic = 1;
}

char chr(int ascii)
{
	return((char)ascii);
}

int ord(char chr)
{
	return((int)chr);
}



char *GetString(char *NameIn,char *Msg)
{
	char *PackName = new char[21];
	bool End=false,SubmitChanges=false;
	int Teller,MaxSelection=0, Selection = 0,asci=97;
	SDL_Event Event;
	SDL_PollEvent(&Event);
	sprintf(PackName,"%s",NameIn);
	MaxSelection = strlen(NameIn);
	PackName[Selection+1]='\0';
	if (MaxSelection == 0)
		PackName[MaxSelection]=chr(asci);
	char Tekst[100];
	while (!End)
	{
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_KEYDOWN)
			{
				switch(Event.key.keysym.sym)
				{
					case SDLK_LEFT:
						if (Selection > 0)
						{	Selection--;
							asci = ord(PackName[Selection]);
						}
						break;
					case SDLK_RIGHT:
						if (Selection < 19)
						{
							Selection++;
							if (Selection > MaxSelection)
							{
								PackName[Selection] = chr(97);
								PackName[Selection+1] = '\0';
								MaxSelection=Selection;
							}
							asci = ord(PackName[Selection]);
						}
						break;
					case SDLK_UP:
						asci++;
						if (asci==123)
							asci=32;
						if (asci==33)
							(asci=48);
						if (asci==58)
							asci=97;
							PackName[Selection] = chr(asci);
						break;
					case SDLK_DOWN:
						asci--;
						if(asci==96)
								asci=57;
						if(asci==47)
							asci=32;
						if(asci==31)
							asci=122;
						PackName[Selection] = chr(asci);
						break;
					case SDLK_q:
					case SDLK_SPACE:
					case SDLK_RETURN:
					case SDLK_a:
						if (GlobalSoundEnabled)
							Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
						End = true;
						SubmitChanges=true;
						break;
					case SDLK_n:
					case SDLK_c:
					case SDLK_x:
						End=true;
						SubmitChanges=false;
						break;
					default:break;
				}
			}
		}
		SDL_BlitSurface(IMGTitleScreen,NULL,Buffer,NULL);
		boxRGBA(Buffer,60,80,260,160,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
		rectangleRGBA(Buffer,60,80,260,160,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		rectangleRGBA(Buffer,61,81,259,159,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		WriteText(Buffer,font,Msg,strlen(Msg),65,85,2,MenuTextColor);
		WriteText(Buffer,MonoFont,PackName,strlen(PackName),85,110,2,MenuTextColor);
		if (Selection > 0)
		{
			sprintf(Tekst," ");
			for (Teller=1;Teller<Selection;Teller++)
				sprintf(Tekst,"%s ",Tekst);
			sprintf(Tekst,"%s_",Tekst);
		}
		else
			sprintf(Tekst,"_");
		WriteText(Buffer,MonoFont,Tekst,strlen(Tekst),85,112,2,MenuTextColor);
		sprintf(Tekst,"Use Up,Down,Left,right. A = Ok X = Cancel" );
		WriteText(Buffer,font,Tekst,strlen(Tekst),65,135,2,MenuTextColor);
		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);
	}
	PackName[MaxSelection+1] = '\0';
	while ((PackName[0] == ' ') && (MaxSelection>-1))
	{
		for (Teller=0;Teller<MaxSelection;Teller++)
			PackName[Teller] = PackName[Teller+1];
		MaxSelection--;
	}
	if (MaxSelection>-1)
		while ((PackName[MaxSelection] == ' ') && (MaxSelection>0))
		{
			PackName[MaxSelection] = '\0';
			MaxSelection--;
		}
	if (!SubmitChanges)
		sprintf(PackName,"%s",NameIn);
	return PackName;
}



void StageSelect()
{
	SDL_Event Event;
	SDL_PollEvent(&Event);
	int Teller;
	char *FileName = new char[FILENAME_MAX];
	char Tekst[300];
	char Command[300],Command1[300];
	if(MusicCount > 0)
		if (! Mix_PlayingMusic())
			if(GlobalSoundEnabled)
				Mix_PlayMusic(Music[0],-1);
	if (SelectedLevel > 0)
	{
		sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
		WorldParts.Load(FileName);
	}
	else
		WorldParts.RemoveAll();
	while (GameState == GSStageSelect)
	{
		SDL_BlitSurface(IMGBackground,NULL,Buffer,NULL);
		WorldParts.Move();
		WorldParts.Draw(Buffer);
		boxRGBA(Buffer,0,0,319,13,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
		rectangleRGBA(Buffer,0,-1,319,13,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		if (SelectedLevel ==0)
			sprintf(Tekst,"Level Pack: %s -> %d Levels - (A) Create New Level",LevelPackName,InstalledLevels);
		else
			if(LevelEditorMode)
				sprintf(Tekst,"Level Pack: %s Level:%d/%d - (A) Edit Level (X) Delete Level",LevelPackName,SelectedLevel,InstalledLevels);
			else
				if(SelectedLevel <= UnlockedLevels)
					sprintf(Tekst,"Level Pack: %s Level:%d/%d - (A) Play Level",LevelPackName,SelectedLevel,InstalledLevels);
				else
					sprintf(Tekst,"Level Pack: %s Level:%d/%d - Level is locked!",LevelPackName,SelectedLevel,InstalledLevels);
		WriteText(Buffer,font,Tekst,strlen(Tekst),2,-4,0,MenuTextColor);
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				GameState = GSQuit;
			if(Event.type == SDL_KEYDOWN)
			{
				switch(Event.key.keysym.sym)
				{
					case SDLK_y:
					case SDLK_n:
					if(LevelEditorMode)
						{
							SelectedLevel = 0;
							WorldParts.RemoveAll();
							LevelHasChanged = false;
							GameState = GSLevelEditor;
						}
						break;
					case SDLK_ESCAPE:
						if(LevelEditorMode)
							GameState= GSLevelEditorMenu;
						else
							GameState= GSTitleScreen;
						WorldParts.RemoveAll();
						break;
					case SDLK_DELETE:
					case SDLK_x:
						if(LevelEditorMode && (SelectedLevel > 0))
						{
							sprintf(Tekst,"Are you sure you want to delete this level:\n%s - Level %d\n\nPress (A) to Delete (X) to Cancel",LevelPackName,SelectedLevel);
							if (AskQuestion(Tekst))
							{
								sprintf(Command,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
								DeleteFile(Command);
								//printf("%s\n",Command);
								for(Teller=SelectedLevel;Teller<InstalledLevels;Teller++)
								{
									sprintf(Command,"./levelpacks/%s/level%d.lev",LevelPackFileName,Teller+1);
									sprintf(Command1,"./levelpacks/%s/level%d.lev",LevelPackFileName,Teller);
									rename(Command,Command1);
									//printf("%s\n",Command);
								}



								InstalledLevels--;
								if (SelectedLevel > InstalledLevels)
									SelectedLevel = InstalledLevels;
								if (SelectedLevel==0)
									WorldParts.RemoveAll();
								else
								{
									sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
									WorldParts.Load(FileName);
								}
							}
						}
						break;
					case SDLK_q:
					case SDLK_a:
					case SDLK_RETURN:
					case SDLK_SPACE:
						if (GlobalSoundEnabled)
							Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
						if(LevelEditorMode)
						{
							LevelHasChanged = false;
							GameState = GSLevelEditor;
						}
						else
							if (SelectedLevel <= UnlockedLevels)
								GameState = GSGame;
							else
							{
								sprintf(Tekst,"This Level Hasn't been unlocked yet!\nDo you want to play the last unlocked\nlevel %d/%d\n\nPress (A) to Play (X) to Cancel",UnlockedLevels,InstalledLevels);
								if	(AskQuestion(Tekst))
								{
									SelectedLevel = UnlockedLevels;
									sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
									WorldParts.Load(FileName);
									GameState = GSGame;
								}
							}
						break;
					case SDLK_PAGEDOWN:
					case SDLK_w:
					case SDLK_z:
					case SDLK_KP_MINUS:
						SelectedLevel -= 5;
						if(LevelEditorMode)
						{
							if (SelectedLevel <= 0)
							{
								SelectedLevel = 0;
								WorldParts.RemoveAll();
							}
							else
							{
								sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
								WorldParts.Load(FileName);
							}
						}
						else
							if (SelectedLevel < 1)
								SelectedLevel = 1;
							sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
							WorldParts.Load(FileName);
						break;
					case SDLK_PAGEUP:
					case SDLK_KP_PLUS:
						SelectedLevel +=5;
						if (SelectedLevel > InstalledLevels)
								SelectedLevel = InstalledLevels;
						sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
						WorldParts.Load(FileName);
						break;
					case SDLK_LEFT:
						SelectedLevel--;
						if(LevelEditorMode)
						{
							if (SelectedLevel <= 0)
							{
								SelectedLevel = 0;
								WorldParts.RemoveAll();
							}
							else
							{
								sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
								WorldParts.Load(FileName);
							}
						}
						else
							if (SelectedLevel < 1)
								SelectedLevel = 1;
							sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
							WorldParts.Load(FileName);

						break;
					case SDLK_RIGHT:
						SelectedLevel++;
						if (SelectedLevel > InstalledLevels)
							SelectedLevel = InstalledLevels;
						sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
						WorldParts.Load(FileName);
						break;
					default:break;

				}
			}
		}

		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);

	}
	delete[] FileName;
}

bool FileExists(char * FileName)
{
	FILE *Fp;
	Fp = fopen(FileName,"rb");
	if (Fp)
	{
		fclose(Fp);
		return true;
	}
	else
		return false;
}

void FindLevels()
{
	int Teller=1;
	char *FileName = new char[FILENAME_MAX];
	InstalledLevels = 0;
	sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,Teller);
	while (FileExists(FileName))
	{
		Teller+=25;
		sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,Teller);
	}
	while (!FileExists(FileName) && (Teller >=1) )
	{
		Teller--;
		sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,Teller);
	}
	InstalledLevels=Teller;
	delete[] FileName;
}
void LoadGraphics()
{
	FILE *ColorsFile;
	SDL_Surface *Tmp;
	int R,G,B,A;
	char FileName[FILENAME_MAX];
	if(IMGBackground)
		SDL_FreeSurface(IMGBackground);
	if(IMGFloor)
		SDL_FreeSurface(IMGFloor);
	if(IMGPlayer)
		SDL_FreeSurface(IMGPlayer);
	if(IMGBox)
		SDL_FreeSurface(IMGBox);
	if(IMGSpot)
		SDL_FreeSurface(IMGSpot);
	if(IMGEmpty)
		SDL_FreeSurface(IMGEmpty);
	if(IMGWall)
		SDL_FreeSurface(IMGWall);
	if(IMGTitleScreen)
		SDL_FreeSurface(IMGTitleScreen);

	sprintf(FileName,"./levelpacks/%s/floor.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/floor.png");
	SDL_SetColorKey(Tmp,SDL_SRCCOLORKEY |SDL_RLEACCEL,SDL_MapRGB(Tmp->format,255,0,255));
	IMGFloor = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/wall.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/wall.png");
	SDL_SetColorKey(Tmp,SDL_SRCCOLORKEY |SDL_RLEACCEL,SDL_MapRGB(Tmp->format,255,0,255));
	IMGWall = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/box.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/box.png");
	SDL_SetColorKey(Tmp,SDL_SRCCOLORKEY |SDL_RLEACCEL,SDL_MapRGB(Tmp->format,255,0,255));
	IMGBox = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/spot.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/spot.png");
	SDL_SetColorKey(Tmp,SDL_SRCCOLORKEY |SDL_RLEACCEL,SDL_MapRGB(Tmp->format,255,0,255));
	IMGSpot = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/player.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/player.png");
	SDL_SetColorKey(Tmp,SDL_SRCCOLORKEY |SDL_RLEACCEL,SDL_MapRGB(Tmp->format,255,0,255));
	IMGPlayer = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/empty.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/empty.png");
	SDL_SetColorKey(Tmp,SDL_SRCCOLORKEY |SDL_RLEACCEL,SDL_MapRGB(Tmp->format,255,0,255));
	IMGEmpty = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/background.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/background.png");
	IMGBackground = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/titlescreen.png",LevelPackFileName);
	if (FileExists(FileName))
		Tmp = IMG_Load(FileName);
	else
		Tmp = IMG_Load("./graphics/titlescreen.png");
	IMGTitleScreen = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);

	sprintf(FileName,"./levelpacks/%s/colors.txt",LevelPackFileName);
	ColorsFile = fopen(FileName,"rt");
	if (ColorsFile)
	{
		fscanf(ColorsFile,"[TextColor]\nR=%d\nG=%d\nB=%d\n",&R,&G,&B);
		MenuTextColor.r = R;
		MenuTextColor.g = G;
		MenuTextColor.b = B;
		fscanf(ColorsFile,"[MenuBoxColor]\nR=%d\nG=%d\nB=%d\nA=%d\n",&R,&G,&B,&A);
		MenuBoxColor.r = R;
		MenuBoxColor.g = G;
		MenuBoxColor.b = B;
		MenuBoxColor.unused = A;
		fscanf(ColorsFile,"[MenuBoxBorderColor]\nR=%d\nG=%d\nB=%d\nA=%d\n",&R,&G,&B,&A);
		MenuBoxBorderColor.r = R;
		MenuBoxBorderColor.g = G;
		MenuBoxBorderColor.b = B;
		MenuBoxBorderColor.unused = A;
		fclose(ColorsFile);
	}
	else
	{
		MenuTextColor.r = 127;
		MenuTextColor.g = 82;
		MenuTextColor.b = 33;
		MenuBoxColor.r = 187;
		MenuBoxColor.g = 164;
		MenuBoxColor.b = 134;
		MenuBoxColor.unused = 255;
		MenuBoxBorderColor.r = 153;
		MenuBoxBorderColor.g = 123;
		MenuBoxBorderColor.b = 87;
		MenuBoxBorderColor.unused = 255;
	}
}

void Credits()
{
	SDL_Event Event;
	char *LevelPackCreator = new char[21];
	char FileName[FILENAME_MAX];
	FILE *Fp;
	SDL_PollEvent(&Event);
	char *Tekst = new char[500];
	sprintf(FileName,"./levelpacks/%s/credits.dat",LevelPackFileName);
	if(InstalledLevelPacks > 0)
	{
		Fp = fopen(FileName,"rt");
		if (Fp)
		{
			fscanf(Fp,"[Credits]\nCreator='%[^']'\n",LevelPackCreator);
			fclose(Fp);
			sprintf(Tekst,"Sokoban GP2X was created by\nWillems Davy - Willems Soft 2006.\nHttp://www.willemssoft.be\n\nLevelpack %s was created\nby %s.",LevelPackName,LevelPackCreator);
		}
		else
			sprintf(Tekst,"Sokoban GP2X was created by\nWillems Davy - Willems Soft 2006.\nHttp://www.willemssoft.be\n\nLevelpack %s was created\nby unknown person.",LevelPackName);
	}
	else
		sprintf(Tekst,"Sokoban GP2X was created by\nWillems Davy - Willems Soft 2006\nHttp://www.willemssoft.be");
	while (GameState == GSCredits)
	{
		SDL_BlitSurface(IMGTitleScreen,NULL,Buffer,NULL);
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				GameState = GSQuit;
			if(Event.type == SDL_KEYDOWN)
				GameState = GSTitleScreen;
		}
		boxRGBA(Buffer,60,80,260,160,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
		rectangleRGBA(Buffer,60,80,260,160,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		rectangleRGBA(Buffer,61,81,259,159,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		WriteText(Buffer,font,Tekst,strlen(Tekst),65,80,2,MenuTextColor);
		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);
	}
	delete[] Tekst;
	delete[] LevelPackCreator;
}


void TitleScreen()
{
	int Teller, Selection = 1;
	SDL_Event Event;
	char Command[FILENAME_MAX];
	SDL_PollEvent(&Event);
	char *Tekst = new char[300];
	if(MusicCount > 0)
		if (! Mix_PlayingMusic())
			if(GlobalSoundEnabled)
				Mix_PlayMusic(Music[0],-1);
	while (GameState == GSTitleScreen)
	{
		SDL_BlitSurface(IMGTitleScreen,NULL,Buffer,NULL);
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				GameState = GSQuit;
			if(Event.type == SDL_KEYDOWN)
			{
				switch(Event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						GameState = GSQuit;
						break;
					case SDLK_LEFT:
						if(Selection==3)
						 	if (InstalledLevelPacksCount > 0)
								if(SelectedLevelPack > 0)
								{
									SelectedLevelPack--;
									sprintf(LevelPackName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									sprintf(LevelPackFileName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									AddUnderScores(LevelPackFileName);
									LoadGraphics();
								}

						break;
					case SDLK_RIGHT:
						if (Selection==3)
							if (InstalledLevelPacksCount > 0)
								if(SelectedLevelPack < InstalledLevelPacksCount-1)
								{
									SelectedLevelPack++;
									sprintf(LevelPackName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									sprintf(LevelPackFileName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									AddUnderScores(LevelPackFileName);
									LoadGraphics();
								}
						break;
					case SDLK_UP:
						if (Selection > 1)
						{
							Selection--;
							if (GlobalSoundEnabled)
								Mix_PlayChannel(-1,Sounds[SND_MENU],0);
						}
						break;
					case SDLK_DOWN:
						if (Selection < 5)
						{
							Selection++;
							if (GlobalSoundEnabled)
								Mix_PlayChannel(-1,Sounds[SND_MENU],0);
						}
						break;
					case SDLK_q:
					case SDLK_a:
					case SDLK_SPACE:
					case SDLK_RETURN:
						switch(Selection)
						{
							case 1:
								if (InstalledLevelPacksCount >0)
								{
									FindLevels();
									LoadUnlockData();
									SelectedLevel=UnlockedLevels;
									LevelEditorMode=false;
									GameState=GSStageSelect;
									if (GlobalSoundEnabled)
										Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								}
								break;
							case 2:
								GameState=GSLevelEditorMenu;
								LevelEditorMode=true;
								if (GlobalSoundEnabled)
									Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								break;
							case 4:
								GameState=GSCredits;
								if (GlobalSoundEnabled)
									Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								break;
							case 5:
								GameState = GSQuit;
								if (GlobalSoundEnabled)
									Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								break;
						}
						break;
						default:break;

				}
			}
		}
		boxRGBA(Buffer,60,80,260,160,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
		rectangleRGBA(Buffer,60,80,260,160,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		rectangleRGBA(Buffer,61,81,259,159,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		sprintf(Tekst,"Play Selected LevelPack\nLevel Editor\n<%s>\nCredits\nQuit",LevelPackName);
		WriteText(Buffer,BigFont,Tekst,strlen(Tekst),90,77,2,MenuTextColor);
		if (Selection > 1)
		{
			sprintf(Tekst,"\n");
			for(Teller=2;Teller<Selection;Teller++)
				sprintf(Tekst,"%s%s",Tekst,"\n");
			sprintf(Tekst,"%s%s",Tekst,">>");
		}
		else
			sprintf(Tekst,">>");
		WriteText(Buffer,BigFont,Tekst,strlen(Tekst),65,77,2,MenuTextColor);
		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);
	}
	delete[] Tekst;
}

bool IsDots(const TCHAR* str)
{
   if(strcmp(str,".") && strcmp(str,"..")) return FALSE;
   return TRUE;
}


bool DeleteDirectory(const TCHAR* sPath)
{
   HANDLE hFind;    // file handle
   WIN32_FIND_DATA FindFileData;

   TCHAR DirPath[MAX_PATH];
   TCHAR FileName[MAX_PATH];

   strcpy(DirPath,sPath);
   strcat(DirPath,"\\*");    // searching all files
   strcpy(FileName,sPath);
   strcat(FileName,"\\");

   // find the first file
   hFind = FindFirstFile(DirPath,&FindFileData);
   if(hFind == INVALID_HANDLE_VALUE) return FALSE;
   strcpy(DirPath,FileName);

   bool bSearch = true;
   while(bSearch) {    // until we find an entry
      if(FindNextFile(hFind,&FindFileData)) {
         if(IsDots(FindFileData.cFileName)) continue;
         strcat(FileName,FindFileData.cFileName);
         if((FindFileData.dwFileAttributes &
            FILE_ATTRIBUTE_DIRECTORY)) {

            // we have found a directory, recurse
            if(!DeleteDirectory(FileName)) {
                FindClose(hFind);
                return FALSE;    // directory couldn't be deleted
            }
            // remove the empty directory
            RemoveDirectory(FileName);
             strcpy(FileName,DirPath);
         }
         else {
            if(FindFileData.dwFileAttributes &
               FILE_ATTRIBUTE_READONLY)
               // change read-only file mode
                  _chmod(FileName, _S_IWRITE);
                  if(!DeleteFile(FileName)) {    // delete the file
                    FindClose(hFind);
                    return FALSE;
               }
               strcpy(FileName,DirPath);
         }
      }
      else {
         // no more files there
         if(GetLastError() == ERROR_NO_MORE_FILES)
         bSearch = false;
         else {
            // some error occurred; close the handle and return FALSE
               FindClose(hFind);
               return FALSE;
         }

      }

   }
   FindClose(hFind);                  // close the file handle

   return RemoveDirectory(sPath);     // remove the empty directory

}



void LevelEditorMenu()
{
	FILE *ColorsFile,*Fp;
	int Teller, Selection = 1;
	SDL_Event Event;
	char *PackName,*CreatorName;
	char Command[FILENAME_MAX];
	char FileName[FILENAME_MAX];
	SDL_PollEvent(&Event);
	char *Tekst = new char[300];
	if(MusicCount > 0)
		if (! Mix_PlayingMusic())
			if(GlobalSoundEnabled)
				Mix_PlayMusic(Music[0],-1);
	while (GameState == GSLevelEditorMenu)
	{
		SDL_BlitSurface(IMGTitleScreen,NULL,Buffer,NULL);
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				GameState = GSQuit;
			if(Event.type == SDL_KEYDOWN)
			{
				switch(Event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						GameState = GSTitleScreen;
						break;
					case SDLK_LEFT:
						if(Selection==4)
						 	if (InstalledLevelPacksCount > 0)
								if(SelectedLevelPack > 0)
								{
									SelectedLevelPack--;
									sprintf(LevelPackName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									sprintf(LevelPackFileName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									AddUnderScores(LevelPackFileName);
									LoadGraphics();
								}

						break;
					case SDLK_RIGHT:
						if (Selection==4)
							if (InstalledLevelPacksCount > 0)
								if(SelectedLevelPack < InstalledLevelPacksCount-1)
								{
									SelectedLevelPack++;
									sprintf(LevelPackName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									sprintf(LevelPackFileName,"%s",InstalledLevelPacks[SelectedLevelPack]);
									AddUnderScores(LevelPackFileName);
									LoadGraphics();
								}
						break;
					case SDLK_UP:
						if (Selection > 1)
						{
							Selection--;
							if (GlobalSoundEnabled)
								Mix_PlayChannel(-1,Sounds[SND_MENU],0);
						}
						break;
					case SDLK_DOWN:
						if (Selection < 5)
						{
							Selection++;
							if (GlobalSoundEnabled)
								Mix_PlayChannel(-1,Sounds[SND_MENU],0);
						}
						break;
					case SDLK_RETURN:
					case SDLK_SPACE:
					case SDLK_a:
					case SDLK_q:
						switch(Selection)
						{
							case 1:
								if (GlobalSoundEnabled)
									Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								PackName = GetString("","Enter the Levelpack name:");
								if (strlen(PackName) > 0)
								{
									SDL_BlitSurface(IMGTitleScreen,NULL,Buffer,NULL);
									CreatorName = GetString("","Enter the Levelpack Creator name:");
									if(strlen(CreatorName)>0)
									{
										sprintf(LevelPackName,"%s",PackName);
										sprintf(LevelPackFileName,"%s",PackName);
										AddUnderScores(LevelPackFileName);
										sprintf(Command,"./levelpacks/%s",LevelPackFileName);
										CreateDirectory(Command,NULL);
										sprintf(FileName,"./levelpacks/%s/colors.txt",LevelPackFileName);
										ColorsFile = fopen(FileName,"wt");
										if (ColorsFile)
										{
											fprintf(ColorsFile,"[TextColor]\nR=127\nG=82\nB=33\n");
											fprintf(ColorsFile,"[MenuBoxColor]\nR=187\nG=164\nB=134\nA=255\n");
											fprintf(ColorsFile,"[MenuBoxBorderColor]\nR=153\nG=123\nB=87\nA=255\n");
											fclose(ColorsFile);
										}
										sprintf(FileName,"./levelpacks/%s/credits.dat",LevelPackFileName);
										Fp = fopen(FileName,"wt");
										if (Fp)
										{
											fprintf(Fp,"[Credits]\nCreator='%s'\n",CreatorName);
											fclose(Fp);
										}



										SearchForLevelPacks();
										for (Teller=0;Teller<InstalledLevelPacksCount;Teller++)
											if(strcmp(PackName,InstalledLevelPacks[Teller]) == 0)
											{
												SelectedLevelPack = Teller;
												sprintf(LevelPackName,"%s",InstalledLevelPacks[SelectedLevelPack]);
												sprintf(LevelPackFileName,"%s",InstalledLevelPacks[SelectedLevelPack]);
												AddUnderScores(LevelPackFileName);
											}
										LoadGraphics();
									}
									delete[] CreatorName;
								}
								delete[] PackName;
								break;
							case 2:
								if (InstalledLevelPacksCount >0)
								{
									FindLevels();
									SelectedLevel=0;
									GameState=GSStageSelect;
									if (GlobalSoundEnabled)
										Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								}
								break;
							case 3:
								if (InstalledLevelPacksCount >0)
								{
									if (GlobalSoundEnabled)
										Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
									sprintf(Tekst,"do you want to delete the selected level pack:\n\"%s\"\nAll Levels in Levelpack will be deleted !!!\n\nPress A to Delete, X to Cancel",InstalledLevelPacks[SelectedLevelPack]);
									if(AskQuestion(Tekst))
									{
										sprintf(Command,"./levelpacks/%s",LevelPackFileName);
										DeleteDirectory(Command);
										SearchForLevelPacks();
										LoadGraphics();
									}

								}
								break;
							case 5:
								if (GlobalSoundEnabled)
									Mix_PlayChannel(-1,Sounds[SND_SELECT],0);
								GameState = GSTitleScreen;
								break;
						}
						break;
						default:break;

				}
			}
		}
		boxRGBA(Buffer,60,80,260,160,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
		rectangleRGBA(Buffer,60,80,260,160,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		rectangleRGBA(Buffer,61,81,259,159,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
		sprintf(Tekst,"Create New Levelpack\nLoad Selected LevelPack\nDelete Selected Levelpack\n<%s>\nMain Menu",LevelPackName);
		WriteText(Buffer,BigFont,Tekst,strlen(Tekst),90,77,2,MenuTextColor);
		if (Selection > 1)
		{
			sprintf(Tekst,"\n");
			for(Teller=2;Teller<Selection;Teller++)
				sprintf(Tekst,"%s%s",Tekst,"\n");
			sprintf(Tekst,"%s%s",Tekst,">>");
		}
		else
			sprintf(Tekst,">>");
		WriteText(Buffer,BigFont,Tekst,strlen(Tekst),65,77,2,MenuTextColor);
		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);
	}
	delete[] Tekst;
}

bool LevelErrorsFound()
{
	int Teller,NumBoxes=0,NumSpots=0,NumPlayer=0,NumFilledSpots=0;
	for (Teller=0;Teller<WorldParts.ItemCount;Teller++)
	{
		if(WorldParts.Items[Teller]->GetType() == IDBox)
		{
			NumBoxes++;
			if (WorldParts.Items[Teller]->GetAnimPhase() == 1)
                NumFilledSpots++;
		}
		if(WorldParts.Items[Teller]->GetType() == IDSpot)
			NumSpots++;
		if(WorldParts.Items[Teller]->GetType() == IDPlayer)
			NumPlayer++;
	}
	if (NumPlayer==0)
	{
		if (GlobalSoundEnabled)
			Mix_PlayChannel(-1,Sounds[SND_ERROR],0);
		PrintForm("Can not save this level because there's\nno player in the level! Please add a Player\nand try again or cancel saving.\n\nPress 'A' to continue");
		return true;
	}
	if (NumBoxes < NumSpots)
	{
		if (GlobalSoundEnabled)
			Mix_PlayChannel(-1,Sounds[SND_ERROR],0);
		PrintForm("Can not save this level because the number\nof spots is greater then the number of boxes!\nPlease Add some more boxes and try again\nor cancel saving.\nPress 'A' to continue");
		return true;
	}
	if (NumSpots-NumFilledSpots == 0)
	{
		if (GlobalSoundEnabled)
			Mix_PlayChannel(-1,Sounds[SND_ERROR],0);
		PrintForm("Can not save this level because there are\nno empty spots in this level! Please Add at least\none empty spot and try again or cancel saving.\n\nPress 'A' to continue");
		return true;
	}
	return false;

}

void LevelEditor()
{
	int Teller,Teller2,MinX,MaxX,MinY,MaxY,Xi,Yi;
	bool ShowPosition=true,AnotherPartFound,SamePartFound,SpotFound;
	char Tekst[200];
	char *FileName = new char[FILENAME_MAX];
	SDL_Event Event;
	SDL_PollEvent(&Event);
	CWorldPart *Part;
	CSelector Selector;
	if(MusicCount > 0)
		if (! Mix_PlayingMusic())
			if(GlobalSoundEnabled)
				Mix_PlayMusic(Music[0],-1);
	if (StageReload)
	{
		WorldParts.Load("./temp.lev");
		DeleteFile("./temp.lev");



		StageReload=false;
	}
	while (GameState == GSLevelEditor)
	{
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				GameState = GSQuit;
			if(Event.type == SDL_KEYDOWN)
			{
				switch(Event.key.keysym.sym)
				{
					case SDLK_q:
					case SDLK_SPACE:
					case SDLK_a:
						SamePartFound = false;
						for(Teller=0;Teller<WorldParts.ItemCount;Teller++)
							if((WorldParts.Items[Teller]->GetPlayFieldX() == Selector.GetPlayFieldX()) &&
							   (WorldParts.Items[Teller]->GetPlayFieldY() == Selector.GetPlayFieldY()))
							{
							   	if(WorldParts.Items[Teller]->GetType() == Selector.GetSelection())
								{
									SamePartFound=true;
								}
								if(Selector.GetSelection() == IDEmpty)
								{
									LevelHasChanged=true;
									break;
								}
							}
						if(Selector.GetSelection() != IDEmpty)
							if(!LevelHasChanged)
								LevelHasChanged=!SamePartFound;
						switch(Selector.GetSelection())
						{
							case IDEmpty:
								for(Teller=0;Teller<WorldParts.ItemCount;Teller++)
								{
									if((WorldParts.Items[Teller]->GetPlayFieldX() == Selector.GetPlayFieldX()) &&
									   (WorldParts.Items[Teller]->GetPlayFieldY() == Selector.GetPlayFieldY()))
									  	if(WorldParts.Items[Teller]->GetType() == IDFloor)
									  	{
									  		AnotherPartFound = false;
									  		SpotFound = false;
									  		for(Teller2=Teller+1;Teller2<WorldParts.ItemCount;Teller2++)
									  			if((WorldParts.Items[Teller2]->GetPlayFieldX() == Selector.GetPlayFieldX()) &&
									   			   (WorldParts.Items[Teller2]->GetPlayFieldY() == Selector.GetPlayFieldY()))
									   			    {
														if(WorldParts.Items[Teller2]->GetType() == IDSpot)
															SpotFound = true;
														else
														{
															WorldParts.Remove(WorldParts.Items[Teller2]->GetPlayFieldX(),WorldParts.Items[Teller2]->GetPlayFieldY(),WorldParts.Items[Teller2]->GetType());
															AnotherPartFound = true;
															break;
														}

													}
											if(SpotFound && !AnotherPartFound)
												WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDSpot);
											else
												if (!SpotFound && !AnotherPartFound)
													WorldParts.Remove(WorldParts.Items[Teller]->GetPlayFieldX(),WorldParts.Items[Teller]->GetPlayFieldY(),IDFloor);
											break;
										}
									  	else
									  		if(WorldParts.Items[Teller]->GetType() == IDSpot)
									  		{
									  			AnotherPartFound = false;
									  			for(Teller2=Teller+1;Teller2<WorldParts.ItemCount;Teller2++)
									  				if((WorldParts.Items[Teller2]->GetPlayFieldX() == Selector.GetPlayFieldX()) &&
									   				   (WorldParts.Items[Teller2]->GetPlayFieldY() == Selector.GetPlayFieldY()) &&
									   				   (WorldParts.Items[Teller2]->GetType() != IDFloor))
														{
															WorldParts.Remove(WorldParts.Items[Teller2]->GetPlayFieldX(),WorldParts.Items[Teller2]->GetPlayFieldY(),WorldParts.Items[Teller2]->GetType());
															AnotherPartFound = true;
															break;
														}
													if (!AnotherPartFound)
														WorldParts.Remove(WorldParts.Items[Teller]->GetPlayFieldX(),WorldParts.Items[Teller]->GetPlayFieldY(),IDSpot);
												break;
											}
									  		else
									  			if(WorldParts.Items[Teller]->GetType() == IDBox)
									  			{
													WorldParts.Remove(WorldParts.Items[Teller]->GetPlayFieldX(),WorldParts.Items[Teller]->GetPlayFieldY(),IDBox);
													break;
												}
												else
													if(WorldParts.Items[Teller]->GetType() == IDPlayer)
													{
														WorldParts.Remove(WorldParts.Items[Teller]->GetPlayFieldX(),WorldParts.Items[Teller]->GetPlayFieldY(),IDPlayer);
														break;
													}
													else
													{
														WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY());
														break;
													}
								}
								break;
							case IDBox:
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDWall);
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDBox);
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDPlayer);
								WorldParts.Add(new CBox(Selector.GetPlayFieldX(),Selector.GetPlayFieldY()));
								break;
							case IDPlayer:
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDWall);
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDBox);
								for(Teller=0;Teller<WorldParts.ItemCount;Teller++)
								{
									if (WorldParts.Items[Teller]->GetType() == IDPlayer)
									   	WorldParts.Remove(WorldParts.Items[Teller]->GetPlayFieldX(),WorldParts.Items[Teller]->GetPlayFieldY(),IDPlayer);
								}
								WorldParts.Add(new CPlayer(Selector.GetPlayFieldX(),Selector.GetPlayFieldY()));
								break;
							case IDWall:
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY());
								WorldParts.Add(new CWall(Selector.GetPlayFieldX(),Selector.GetPlayFieldY()));
								break;
							case IDSpot:
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDWall);
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDSpot);
								WorldParts.Add(new CSpot(Selector.GetPlayFieldX(),Selector.GetPlayFieldY()));
								break;
							case IDFloor:
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDFloor);
								WorldParts.Remove(Selector.GetPlayFieldX(),Selector.GetPlayFieldY(),IDWall);
								WorldParts.Add(new CFloor(Selector.GetPlayFieldX(),Selector.GetPlayFieldY()));
								break;
						}
						break;
					case SDLK_RETURN:
						if(!LevelErrorsFound())
						{
							WorldParts.Save("./temp.lev");
							StageReload = true;
							GameState=GSGame;
						}
						break;
					case SDLK_RIGHT:
						Selector.MoveRight();
						break;
					case SDLK_LEFT:
						Selector.MoveLeft();
						break;
					case SDLK_UP:
						Selector.MoveUp();
						break;
					case SDLK_DOWN:
						Selector.MoveDown();
						break;
					case SDLK_PAGEDOWN:
					case SDLK_w:
					case SDLK_z:
					case SDLK_KP_MINUS:
						Selector.DecSelection();
						break;
					case SDLK_PAGEUP:
					case SDLK_x:
					case SDLK_KP_PLUS:
						Selector.IncSelection();
						break;
					case SDLK_p:
					case SDLK_b:
						ShowPosition = !ShowPosition;
						break;
					case SDLK_c:
						MinX = NrOfCols-1;
						MinY = NrOfRows-1;
						MaxX = 0;
						MaxY = 0;
						for (Teller = 0;Teller<WorldParts.ItemCount;Teller++)
						{
							if (WorldParts.Items[Teller]->GetPlayFieldX() < MinX)
								MinX = WorldParts.Items[Teller]->GetPlayFieldX();
							if (WorldParts.Items[Teller]->GetPlayFieldY() < MinY)
								MinY = WorldParts.Items[Teller]->GetPlayFieldY();
							if (WorldParts.Items[Teller]->GetPlayFieldX() > MaxX)
								MaxX = WorldParts.Items[Teller]->GetPlayFieldX();
							if (WorldParts.Items[Teller]->GetPlayFieldY() > MaxY)
								MaxY = WorldParts.Items[Teller]->GetPlayFieldY();
						}
						Xi = ((NrOfCols-1) / 2) - (MaxX + MinX) / 2;
						Yi = ((NrOfRows-1) / 2) - (MaxY + MinY) / 2;
						for (Teller = 0;Teller<WorldParts.ItemCount;Teller++)
						{
							WorldParts.Items[Teller]->SetPosition(WorldParts.Items[Teller]->GetPlayFieldX() + Xi,WorldParts.Items[Teller]->GetPlayFieldY() + Yi);
						}
						if (Xi != 0 || Yi!=0)
							LevelHasChanged = true;
						break;
					case SDLK_BACKSPACE:
					case SDLK_d:
					case SDLK_y:
					case SDLK_DELETE:
						if (WorldParts.ItemCount > 0)
							if(AskQuestion("You are about to delete all parts\nin this level, are you sure\nyou want to do this?\n\nPress (A) to delete, (X) to cancel"))
							{
								LevelHasChanged = true;
								WorldParts.RemoveAll();
							}
						break;
					case SDLK_ESCAPE:
						if (LevelHasChanged)
						{
							if(AskQuestion("The current level isn't saved yet!\nDo you want to save it now ?\n\nPress (A) to save, (X) to cancel saving"))
							{
								SDL_BlitSurface(IMGBackground,NULL,Buffer,NULL);
								WorldParts.Move();
								WorldParts.Draw(Buffer);
								Selector.Draw(Buffer);
								if (ShowPosition)
								{
									sprintf(Tekst,"X: %d - Y: %d",Selector.GetPlayFieldX(),Selector.GetPlayFieldY());
									boxRGBA(Buffer,265,0,319,13,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
									rectangleRGBA(Buffer,265,-1,319,13,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
									WriteText(Buffer,font,Tekst,strlen(Tekst),267,-4,0,MenuTextColor);
								}
								if (!LevelErrorsFound())
								{
									if (SelectedLevel==0)
											sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,InstalledLevels+1);
									else
										sprintf(FileName,"./levelpacks/%s/level%d.lev",LevelPackFileName,SelectedLevel);
									WorldParts.Save(FileName);
									FindLevels();
									if (SelectedLevel==0)
										SelectedLevel = InstalledLevels;
									LevelHasChanged=false;
									GameState = GSStageSelect;
								}
							}
							else
								GameState = GSStageSelect;
						}
						else
							GameState = GSStageSelect;
						break;
						default:break;
				}
			}
		}
		SDL_BlitSurface(IMGBackground,NULL,Buffer,NULL);
		WorldParts.Move();
		WorldParts.Draw(Buffer);
		Selector.Draw(Buffer);
		if (ShowPosition)
		{
			sprintf(Tekst,"X: %d - Y: %d",Selector.GetPlayFieldX(),Selector.GetPlayFieldY());
			boxRGBA(Buffer,265,0,319,13,MenuBoxColor.r,MenuBoxColor.g,MenuBoxColor.b,MenuBoxColor.unused);
			rectangleRGBA(Buffer,265,-1,319,13,MenuBoxBorderColor.r,MenuBoxBorderColor.g,MenuBoxBorderColor.b,MenuBoxBorderColor.unused);
			WriteText(Buffer,font,Tekst,strlen(Tekst),267,-4,0,MenuTextColor);
		}
		tmp = zoomSurface(Buffer,2,2,0);
		tmp1 = SDL_DisplayFormat(tmp);
		SDL_BlitSurface(tmp1,NULL,Screen,NULL);
		SDL_Flip(Screen);
		SDL_FreeSurface(tmp);
		SDL_FreeSurface(tmp1);
	}
	delete[] FileName;
}

void UnLoadGraphics()
{
	if(IMGBackground)
		SDL_FreeSurface(IMGBackground);
	if(IMGFloor)
		SDL_FreeSurface(IMGFloor);
	if(IMGPlayer)
		SDL_FreeSurface(IMGPlayer);
	if(IMGBox)
		SDL_FreeSurface(IMGBox);
	if(IMGSpot)
		SDL_FreeSurface(IMGSpot);
	if(IMGEmpty)
		SDL_FreeSurface(IMGEmpty);
	if(IMGWall)
		SDL_FreeSurface(IMGWall);
	if(IMGTitleScreen)
		SDL_FreeSurface(IMGTitleScreen);
}

void UnloadMusic()
{
	int Teller;
	Mix_HaltMusic();
	for (Teller=0;Teller < MusicCount;Teller++)
		if (Music[Teller])
			Mix_FreeMusic(Music[Teller]);
	//Mix_FreeMusic(Music[0]);
}

void LoadSounds()
{
	Sounds[SND_DROP] = Mix_LoadWAV("./sound/drop.wav");
	Sounds[SND_MENU] = Mix_LoadWAV("./sound/menu.wav");
	Sounds[SND_SELECT] = Mix_LoadWAV("./sound/select.wav");
	Sounds[SND_ERROR] = Mix_LoadWAV("./sound/error.wav");
	Sounds[SND_STAGEEND] = Mix_LoadWAV("./sound/stageend.wav");
}

void UnloadSounds()
{
	int Teller;
	for (Teller=0;Teller<NrOfSounds;Teller++)
		if(Sounds[Teller])
			Mix_FreeChunk(Sounds[Teller]);
}

int main(int argc, char **argv)
{
	srand((int) time(NULL));
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_AUDIO ) == 0)
	{
		printf("SDL Succesfully initialized\n");
		Screen = SDL_SetVideoMode( WINDOW_WIDTH, WINDOW_HEIGHT,16, SDL_SWSURFACE );
		if(Screen)
		{
            tmp = SDL_CreateRGBSurface(SDL_SWSURFACE,320,240,Screen->format->BitsPerPixel,Screen->format->Rmask,Screen->format->Gmask,Screen->format->Bmask,Screen->format->Amask);
			Buffer = SDL_DisplayFormat(tmp);
			SDL_FreeSurface(tmp);
			printf("Succesfully Set %dx%dx16\n",WINDOW_WIDTH,WINDOW_HEIGHT);
			SDL_ShowCursor(SDL_DISABLE);
			SDL_WM_SetCaption("SOKOBAN GP2X",0);
			if (Mix_OpenAudio(22050,AUDIO_S16,MIX_DEFAULT_CHANNELS,1024) < 0)
			{
				GlobalSoundEnabled = false;
				printf("Failed to initialise sound!\n");
			}
			else
			{
				printf("Audio Succesfully initialised!\n");
			}
			if (TTF_Init() == 0)
			{
				printf("Succesfully initialized TTF\n");
				font = TTF_OpenFont("./font.ttf",15);
				BigFont = TTF_OpenFont("./font.ttf",20);
				MonoFont = TTF_OpenFont("./font1.ttf",17);
				if (font && BigFont && MonoFont)
				{
					printf("Succesfully Loaded fonts\n");
					TTF_SetFontStyle(font,TTF_STYLE_NORMAL);
					SearchForLevelPacks();
					SearchForMusic();
					LoadSounds();
					LoadGraphics();

					while (GameState != GSQuit)
					{
						switch(GameState)
						{
							case GSIntro :

								break;
							case GSTitleScreen :
								TitleScreen();
								break;
							case GSCredits :
								Credits();
								break;
							case GSGame :
								Game();
								break;
							case GSStageClear:
								break;
							case GSStageSelect:
								StageSelect();
								break;
							case GSLevelEditor:
								LevelEditor();
								break;
							case GSLevelEditorMenu:
								LevelEditorMode = true;
								LevelEditorMenu();
								break;
						}
					}
					UnLoadGraphics();
					UnloadSounds();
					UnloadMusic();
					TTF_CloseFont(font);
					TTF_CloseFont(BigFont);
					TTF_CloseFont(MonoFont);
					MonoFont=NULL;
			    	font=NULL;
			    	BigFont=NULL;
				}
				else
				{
						printf("Failed to Load fonts\n");
				}
				TTF_Quit();
			}
			else
			{
				printf("Failed to initialize TTF\n");
			}
			SDL_FreeSurface(Screen);
			Screen=NULL;
			Mix_CloseAudio();
		}
		else
		{
			printf("Failed to Set Videomode %dx%dx32\n",WINDOW_WIDTH, WINDOW_HEIGHT);
		}
		SDL_Quit();
	}
	else
	{
		printf("Couldn't initialise SDL!\n");
	}
	//since the destructor isn't being called clean up the stuff ourselves
	WorldParts.RemoveAll();
	return 0;

}
