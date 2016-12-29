/*
 * A játék menetét vezérlõ függvényeket tartalmazza.
 */

#include "bsp.h"

#define abs(x) ((x)<0 ? (-(x)) : (x))
#define IsKorFigure(x) (x>>1 == KOR_PLAYER)
#define IsNegyzetFigure(x) (x>>1 == NEGYZET_PLAYER)
#define IsMyFigure(x) (x>>1 == ActualPlayer)
#define IsEnemyFigure(x) (x>>1 == (ActualPlayer^1))

uint8_t ActualPlayer = NEGYZET_PLAYER;
#define OTHER_PLAYER (ActualPlayer^1)
uint8_t FigureNum[2] = {12,12}; // A játékosok megmaradt bábuinak száma.

uint8_t HitObligation = 0; // Ütéskényszert jelez.
uint8_t HitChain = 0; // Ütéssorozatot jelez.

uint8_t CursorX = 0; // Kurzor aktuális pozíciója.
uint8_t CursorY = 0;

FigureType CarriedFigure; // Tábláról felvett bábu.
uint8_t PrevX; // A felvett bábu eredeti helye.
uint8_t PrevY;

GameStateType GameState = Search; //A játék aktuális állapotát jelzi.

FigureType Table[8][8] =
{
		{URES,	KOR,	URES,	/**/URES,	URES,	/**/URES,	URES,	/**/URES},
		{NEGYZET,	URES,	NEGYZET,	URES,	/**/URES,	URES,	/**/URES,	URES},
		{URES,	/**/URES,	URES,	NEGYZET,	URES,	/**/URES,	URES,	KOR},
		{URES,	URES,	URES,	URES,	URES,	URES,	URES,	URES},
		{URES,	URES,	URES,	URES,	URES,	URES,	URES,	URES},
		{NEGYZET,	URES,	NEGYZET,	URES,	NEGYZET,	URES,	NEGYZET,	URES},
		{URES,	NEGYZET,	URES,	NEGYZET,	URES,	NEGYZET,	URES,	NEGYZET},
		{NEGYZET,	URES,	NEGYZET,	URES,	NEGYZET,	URES,	NEGYZET,	URES},
};

uint8_t Points[2] = {0,0};

// A játéktábla ("x","y") mezõjére helyezünk egy "figure" típusú bábut.
void SetFigure(uint8_t x, uint8_t y, FigureType figure)
{
	assert(IS_COORD_VALID(x,y));
	Table[y][x] = figure;
}

// Eseménykezelõ függvények.
// Mivel a táblát lépésenként fordítjuk, így mindkét játékoshoz különbözõ gombkioztás tartozik.
// (A belsõ Table tömböt nem forgatjuk, csak fordítva jelezzük ki.)
typedef void(*EventHandlerType)(void);
EventHandlerType EventHandler[3][5] = {
/*Search*/	{MoveCursorUp,MoveCursorLeft,MoveCursorRight,MoveCursorDown,SelectFigure}, // Mozgatandó bábu keresése.
/*Carry*/		{CarryFigureUp,CarryFigureLeft,CarryFigureRight,CarryFigureDown, PlaceFigure}, // Felvett bábu pozícionálása.
/*End*/		{RestartGame, RestartGame,RestartGame,RestartGame,RestartGame} // Új játék indítása.
			};
EventHandlerType ReverseEventHandler[3][5] = {
/*Search*/	{MoveCursorDown,MoveCursorRight,MoveCursorLeft,MoveCursorUp,SelectFigure},
/*Carry*/		{CarryFigureDown,CarryFigureRight,CarryFigureLeft,CarryFigureUp, PlaceFigure},
/*End*/		{RestartGame, RestartGame,RestartGame,RestartGame,RestartGame}
			};

int IsHitPossible(uint8_t x, uint8_t y);
// Megvizsgálja, hogy a jelenlegi játékos tud-e még lépni valahová.
int IsMovePossible()
{
	int x,y;
	FigureType fig;
	for(y = 0;y<8;y++) // Végigmegyünk minden soron,
		for(x = 1-(y&0x01) ;x<8;x+=2) // és minden második mezõn.
			if(IsMyFigure((fig =Table[y][x]))) // Ha saját bábut találtunk, megnézzük, hogy tudnánk-e lépni vele.
			{
				switch (fig) // Bábu típusonkénti felbontás.
				{
				case KOR:
					if( y+1<8 && ((x-1>=0 && Table[y+1][x-1] == URES) || (x+1<8 && (Table[y+1][x+1] == URES)))) return 1;
					break;
				case NEGYZET:
					if( y-1>=0 && ((x-1>=0 && Table[y-1][x-1] == URES) || (x+1<8 && (Table[y-1][x+1] == URES)))) return 1;
					break;
				case KOR_KIRALY:
				case NEGYZET_KIRALY:
					if( y+1<8 && ((x-1>=0 && Table[y+1][x-1] == URES) || (x+1<8 && (Table[y+1][x+1] == URES)))) return 1;
					if( y-1>=0 && ((x-1>=0 && Table[y-1][x-1] == URES) || (x+1<8 && (Table[y-1][x+1] == URES)))) return 1;
					break;
				default: assert(0);
				}

			// Ha lépni nem tudunk, tudnuk-e ütni?
			if(IsHitPossible(x,y)) return 1;
			}

	return 0; // Ha sem lépni, sem ütni nem tudunk, akkor nincs lehetséges lépés.
}
void ChangePlayer() // Aktuális játékost váltunk.
{
	ActualPlayer ^= 1;
	if(FigureNum[ActualPlayer] == 0 || !IsMovePossible()) // a játéknak vége, ha elfogytak a bábuim, vagy nem tudok lépni
	{
		ActualPlayer = OTHER_PLAYER; //Visszacseréljük a játékost, hogy a tábla jól álljon.
		Points[ActualPlayer]++; // Növeljük a pontszámot
		Text1 = " WIN ";
		Text2 = "    ";
		CursorX = CursorY = 8; // A kurzort eltüntetjük
		StartBlinkTimer(); //a kijelzõ villog a specifikáció szerint
		GameState = End;
	}
	else
	{
		GameState = Search;
		HitObligation = 0;
		// megnézi, lehetséges-e ütés az adott játékos számára
		int x,y;
		for(y=0;y<8 && !HitObligation;y++)
			for(x=0;x<8 && !HitObligation;x++)
				if(IsHitPossible(x,y)) HitObligation = 1;

		// Ha igen, akkor jelezzük az ütéskényszert.
		if(HitObligation)
		{
			Text1 = "\x1E""tn""\x1F""d";
			Text2 = "kell";
		}
		else
		{
			Text1 = "     "; // Az elõzõ üzenetet kitöröljük.
			Text2 = "    ";
		}
		DrawTable(PATTERN_ON,SCROLL); // Újra kirajzoljuk a táblát megfordítva.
	}

}
void ThrowBack() //A felvett bábut visszadobjuk az eredeti helyére.
{
	if(!HitChain) // Ütés sorozat esetén ugyan azzal a bábuval kell ütni, ütés nélkül nem tehetõ le.
	{
		SetFigure(PrevX,PrevY,CarriedFigure); // Visszamentjük a bábut a régi helyére
		DrawMaze(PrevX,PrevY,CarriedFigure,NO_INVERT); // Ki is jelezzük
		DrawMaze(CursorX,CursorY,Table[CursorY][CursorX],INVERT); // A kurzor alatt lévõ bábut ójra kirajzoljuk.
		GameState = Search;
	}
}

void ShowStep() // Letesszük a felvett bábut a kurzor által mutatott helyre.
{
	SetFigure(PrevX,PrevY,URES); // Kitörlöm a kiindulási pontot
	DrawMaze(PrevX,PrevY,URES,NO_INVERT); // Ennek megjelenítése
	SetFigure(CursorX,CursorY,CarriedFigure); // Beállítom a bábut az új helyre.
}

int  IsHitPossible(uint8_t x, uint8_t y) // Megvizsgálja, hogy az aktuális játékos számára ven-e ütési lehetõség.
{
if((ActualPlayer == KOR_PLAYER && IsKorFigure(Table[y][x])) ||(ActualPlayer == NEGYZET_PLAYER && Table[y][x] == NEGYZET_KIRALY) )
{
	if( 	 (x-2>=0 && y+2<8 && (Table[y+2][x-2] == URES)) && IsEnemyFigure(Table[y+1][x-1])) return 1;// ha balra le 2 mezõvel üres helyet találok, és közben egy ellenséges bábu van, akkor tudok ütni
	else if( (x+2<8 && y+2<8 && (Table[y+2][x+2] == URES) && IsEnemyFigure(Table[y+1][x+1]))) return 1;// ugyan ez jobbra le
}
if((ActualPlayer == NEGYZET_PLAYER && IsNegyzetFigure(Table[y][x])) ||(ActualPlayer == KOR_PLAYER && Table[y][x] == KOR_KIRALY))
{
	if((x-2>=0 && y-2>=0 && (Table[y-2][x-2] == URES) && IsEnemyFigure(Table[y-1][x-1]))) return 1; //balra fel
	else if((x+2<8 && y-2>=0 && (Table[y-2][x+2] == URES) && IsEnemyFigure(Table[y-1][x+1]))) return 1; // jobbra fel
}
return 0;
}


void SelectFigure() // A kiálasztott bábut felveszi.
{
	// ha lehetséges ütés, csak olyan bábut enged felvenni, ami üthet
	if(HitObligation && IsHitPossible(CursorX,CursorY)==0) return;
		FigureType figure = Table[CursorY][CursorX];
		if(IsMyFigure(figure)) // Csak saját bábut vehetek fel
		{
			CarriedFigure = figure;
			SetFigure(CursorX,CursorY,KIINDULASI_PONT); // A bábu eredeti helyét ponttal jelölöm.
			PrevX = CursorX;
			PrevY = CursorY;
			GameState = Carry;
		}
}

void PlaceFigure() // A felvett bábut leteszem.
{
	if(Table[CursorY][CursorX] != URES) {ThrowBack(); return;} // bábura nem léphetünk rá
	switch(CarriedFigure)
	{
	case KOR:
		// jobbra/balra fel lépés, ha nem kell ütnöm
		if((HitObligation == 0) && (CursorX == PrevX-1 || CursorX == PrevX+1) && CursorY == PrevY+1)
		{
			ShowStep();
			if(CursorY == 7) //Ha elértem a tábla szélét, akkor a bábu királlyá alakul.
			{
				Table[CursorY][CursorX]++;
				DrawMaze(CursorX,CursorY,KOR_KIRALY,INVERT);
			}
		}
		//átlós ütés
		else if((CursorX == PrevX-2 || CursorX == PrevX+2) && CursorY == PrevY+2) //ütõ lépés
		{
			uint8_t x_avg = (CursorX + PrevX) /2; // köztes mezõ x koordinátája
			uint8_t y_avg = CursorY-1;// köztes mezõ y koordinátája
			if(IsEnemyFigure(Table[y_avg][x_avg])) //Ha ellenséges bábut léptem át.
			{
				assert(HitObligation); // Ennek be kell állítva lennie, különben lépést is engedtünk volna.
				ShowStep(); // saját bábut léptetem
				SetFigure(x_avg,y_avg,URES); //leütött bábut leveszem
				DrawMaze(x_avg,y_avg,URES,NO_INVERT); //törlöm a belsõ tábláról
				FigureNum[NEGYZET_PLAYER]--;

				//megvizsgáljuk, lehetséges-e még ütés, ha igen a játékos léphet tovább
				if(IsHitPossible(CursorX,CursorY))
				{
					SetFigure(CursorX,CursorY,KIINDULASI_PONT);
					PrevX = CursorX;
					PrevY = CursorY;
					HitChain = 1;
					HitObligation = 1;
					return;
				}
				else
				{
					HitChain = 0;
					HitObligation = 0;
				}
				//ha az ütéssel elértem az utolsó sort, királlyá alakul
				if(CursorY == 7)
				{
					Table[CursorY][CursorX]++;
					DrawMaze(CursorX,CursorY,KOR_KIRALY,INVERT);
				}
			} else {ThrowBack(); return;} //Nem ellenséges bábut léptem át
		}else {ThrowBack(); return;} // Olyan helyre léptem, ami sem lépés, sem ütés nem lehet.

		ChangePlayer();
		return;

		break;
	case NEGYZET:
		// átlós lépés jobbra/balra le
		if((HitObligation == 0) && (CursorX == PrevX-1 || CursorX == PrevX+1) && CursorY == PrevY-1)
		{
			ShowStep();
			if(CursorY == 0) // Ha elértem az utolsó sort, a bábu átalakul királlyá
			{
				Table[CursorY][CursorX]++;
				DrawMaze(CursorX,CursorY,NEGYZET_KIRALY,INVERT);
			}
		}
		// átlós ütés
		else if((CursorX == PrevX-2 || CursorX == PrevX+2) && CursorY == PrevY-2)
		{
			uint8_t x_avg = (CursorX + PrevX) /2; // köztes mezõ x koordinátája
			uint8_t y_avg = CursorY+1;// köztes mezõ y koordinátája
			if(IsEnemyFigure(Table[y_avg][x_avg]))
			{
				assert(HitObligation);
				ShowStep(); // saját bábut léptetem
				SetFigure(x_avg,y_avg,URES); //leütött bábut leveszem
				DrawMaze(x_avg,y_avg,URES,NO_INVERT);
				FigureNum[KOR_PLAYER]--; //törlöm a statisztikából

				if(IsHitPossible(CursorX,CursorY)) // Ha még lehetséges ütés, akkor a játékos folytathatja a játékot.
				{
					SetFigure(CursorX,CursorY,KIINDULASI_PONT);
					PrevX = CursorX;
					PrevY = CursorY;
					HitChain = 1;
					HitObligation = 1;
					return;
				}
				else
				{
					HitChain = 0;
					HitObligation = 0;
				}

				if(CursorY == 0) // Ha ütéssel elértük az utolsó sort, akkor átalakulunk királlyá.
				{
					Table[CursorY][CursorX]++;
					DrawMaze(CursorX,CursorY,NEGYZET_KIRALY,INVERT);
				}
			}
			else {ThrowBack();return;}
		}
		else {ThrowBack(); return;}

		ChangePlayer();
		return;
		break;
	case NEGYZET_KIRALY:
	case KOR_KIRALY:
		// egyszerû lépés 4 irányban
		if(!HitObligation && (abs(CursorX-PrevX)== 1) && (abs(CursorY-PrevY) == 1))
		{
			ShowStep();
		}
		//ha kettõt léptünk átlósan, megnézzük, üthetünk-e
		else if(abs(CursorX-PrevX) == 2 && abs(CursorY-PrevY) == 2)
		{
			uint8_t x_avg = (CursorX + PrevX) /2; // köztes mezõ x koordinátája
			uint8_t y_avg = (CursorY + PrevY)/2; //köztes mezõ y koordinátája
			if(IsEnemyFigure(Table[y_avg][x_avg])) //ellenséges bábut ütöttünk-e le
			{
				ShowStep();
				SetFigure(x_avg,y_avg,URES);
				DrawMaze(x_avg,y_avg,URES,NO_INVERT);
				FigureNum[OTHER_PLAYER]--;

				if(IsHitPossible(CursorX,CursorY)) // ha még tudok ütni, folytathatom
				{
					SetFigure(CursorX,CursorY,KIINDULASI_PONT);
					PrevX = CursorX;
					PrevY = CursorY;
					HitChain = 1;
					HitObligation = 1;
					return;
				}
				else
				{
					HitChain = 0;
					HitObligation = 0;
				}
			}
			else {ThrowBack(); return;}
		}
		else {ThrowBack(); return;} //ha a lépés szabálytalan lenne

		ChangePlayer(); // szabályos lépés után játékoscsere
		return;
		break;
	default: assert(0);
	}//switch
	assert(0);
}


void RestartGame() // Újra indítjuk a játékot.
{
	StopBlinkTimer(); //Leállítjuk a villogást.
	ToggleSignal = 0; // Ha lenne függõ villogtató utasítás, leállítjuk.
	SetInstructionReg();
#ifdef TOGGLE_FULL_SCREEN
	SelectChip1();
	WriteData(DISPLAY_ON); // Ha a teljes képernyõ villogott, akkor azt újra bekapcsoljuk.
	SelectChip2();
	WriteData(DISPLAY_ON);
#endif

	//Újra inicializálom a táblát.
	int x,y;
	for(y = 0;y<3;y++)
		for(x = 1-(y&0x01);x<8;x+=2)
			Table[y][x] = KOR;
	for(y = 3;y<5;y++)
		for(x = 0;x<8;x++)
			Table[y][x] = URES;
	for(y = 5;y<8;y++)
		for(x = 1-(y&0x01);x<8;x+=2)
			Table[y][x] = NEGYZET;

	FigureNum[0] = 12;
	FigureNum[1] = 12;
	ActualPlayer = (Points[0]+Points[1])%2==0 ? NEGYZET_PLAYER : KOR_PLAYER; // A játékosok felváltva kezdenek -> a pontok összege a parti számát adja.
	GameState = Search;
	HitObligation = 0;
	HitChain = 0;
	CursorX = 0;
	CursorY = 0;

	SetupScreen();
}
