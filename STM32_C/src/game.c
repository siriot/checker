/*
 * A j�t�k menet�t vez�rl� f�ggv�nyeket tartalmazza.
 */

#include "bsp.h"

#define abs(x) ((x)<0 ? (-(x)) : (x))
#define IsKorFigure(x) (x>>1 == KOR_PLAYER)
#define IsNegyzetFigure(x) (x>>1 == NEGYZET_PLAYER)
#define IsMyFigure(x) (x>>1 == ActualPlayer)
#define IsEnemyFigure(x) (x>>1 == (ActualPlayer^1))

uint8_t ActualPlayer = NEGYZET_PLAYER;
#define OTHER_PLAYER (ActualPlayer^1)
uint8_t FigureNum[2] = {12,12}; // A j�t�kosok megmaradt b�buinak sz�ma.

uint8_t HitObligation = 0; // �t�sk�nyszert jelez.
uint8_t HitChain = 0; // �t�ssorozatot jelez.

uint8_t CursorX = 0; // Kurzor aktu�lis poz�ci�ja.
uint8_t CursorY = 0;

FigureType CarriedFigure; // T�bl�r�l felvett b�bu.
uint8_t PrevX; // A felvett b�bu eredeti helye.
uint8_t PrevY;

GameStateType GameState = Search; //A j�t�k aktu�lis �llapot�t jelzi.

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

// A j�t�kt�bla ("x","y") mez�j�re helyez�nk egy "figure" t�pus� b�but.
void SetFigure(uint8_t x, uint8_t y, FigureType figure)
{
	assert(IS_COORD_VALID(x,y));
	Table[y][x] = figure;
}

// Esem�nykezel� f�ggv�nyek.
// Mivel a t�bl�t l�p�senk�nt ford�tjuk, �gy mindk�t j�t�koshoz k�l�nb�z� gombkiozt�s tartozik.
// (A bels� Table t�mb�t nem forgatjuk, csak ford�tva jelezz�k ki.)
typedef void(*EventHandlerType)(void);
EventHandlerType EventHandler[3][5] = {
/*Search*/	{MoveCursorUp,MoveCursorLeft,MoveCursorRight,MoveCursorDown,SelectFigure}, // Mozgatand� b�bu keres�se.
/*Carry*/		{CarryFigureUp,CarryFigureLeft,CarryFigureRight,CarryFigureDown, PlaceFigure}, // Felvett b�bu poz�cion�l�sa.
/*End*/		{RestartGame, RestartGame,RestartGame,RestartGame,RestartGame} // �j j�t�k ind�t�sa.
			};
EventHandlerType ReverseEventHandler[3][5] = {
/*Search*/	{MoveCursorDown,MoveCursorRight,MoveCursorLeft,MoveCursorUp,SelectFigure},
/*Carry*/		{CarryFigureDown,CarryFigureRight,CarryFigureLeft,CarryFigureUp, PlaceFigure},
/*End*/		{RestartGame, RestartGame,RestartGame,RestartGame,RestartGame}
			};

int IsHitPossible(uint8_t x, uint8_t y);
// Megvizsg�lja, hogy a jelenlegi j�t�kos tud-e m�g l�pni valahov�.
int IsMovePossible()
{
	int x,y;
	FigureType fig;
	for(y = 0;y<8;y++) // V�gigmegy�nk minden soron,
		for(x = 1-(y&0x01) ;x<8;x+=2) // �s minden m�sodik mez�n.
			if(IsMyFigure((fig =Table[y][x]))) // Ha saj�t b�but tal�ltunk, megn�zz�k, hogy tudn�nk-e l�pni vele.
			{
				switch (fig) // B�bu t�pusonk�nti felbont�s.
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

			// Ha l�pni nem tudunk, tudnuk-e �tni?
			if(IsHitPossible(x,y)) return 1;
			}

	return 0; // Ha sem l�pni, sem �tni nem tudunk, akkor nincs lehets�ges l�p�s.
}
void ChangePlayer() // Aktu�lis j�t�kost v�ltunk.
{
	ActualPlayer ^= 1;
	if(FigureNum[ActualPlayer] == 0 || !IsMovePossible()) // a j�t�knak v�ge, ha elfogytak a b�buim, vagy nem tudok l�pni
	{
		ActualPlayer = OTHER_PLAYER; //Visszacser�lj�k a j�t�kost, hogy a t�bla j�l �lljon.
		Points[ActualPlayer]++; // N�velj�k a pontsz�mot
		Text1 = " WIN ";
		Text2 = "    ";
		CursorX = CursorY = 8; // A kurzort elt�ntetj�k
		StartBlinkTimer(); //a kijelz� villog a specifik�ci� szerint
		GameState = End;
	}
	else
	{
		GameState = Search;
		HitObligation = 0;
		// megn�zi, lehets�ges-e �t�s az adott j�t�kos sz�m�ra
		int x,y;
		for(y=0;y<8 && !HitObligation;y++)
			for(x=0;x<8 && !HitObligation;x++)
				if(IsHitPossible(x,y)) HitObligation = 1;

		// Ha igen, akkor jelezz�k az �t�sk�nyszert.
		if(HitObligation)
		{
			Text1 = "\x1E""tn""\x1F""d";
			Text2 = "kell";
		}
		else
		{
			Text1 = "     "; // Az el�z� �zenetet kit�r�lj�k.
			Text2 = "    ";
		}
		DrawTable(PATTERN_ON,SCROLL); // �jra kirajzoljuk a t�bl�t megford�tva.
	}

}
void ThrowBack() //A felvett b�but visszadobjuk az eredeti hely�re.
{
	if(!HitChain) // �t�s sorozat eset�n ugyan azzal a b�buval kell �tni, �t�s n�lk�l nem tehet� le.
	{
		SetFigure(PrevX,PrevY,CarriedFigure); // Visszamentj�k a b�but a r�gi hely�re
		DrawMaze(PrevX,PrevY,CarriedFigure,NO_INVERT); // Ki is jelezz�k
		DrawMaze(CursorX,CursorY,Table[CursorY][CursorX],INVERT); // A kurzor alatt l�v� b�but �jra kirajzoljuk.
		GameState = Search;
	}
}

void ShowStep() // Letessz�k a felvett b�but a kurzor �ltal mutatott helyre.
{
	SetFigure(PrevX,PrevY,URES); // Kit�rl�m a kiindul�si pontot
	DrawMaze(PrevX,PrevY,URES,NO_INVERT); // Ennek megjelen�t�se
	SetFigure(CursorX,CursorY,CarriedFigure); // Be�ll�tom a b�but az �j helyre.
}

int  IsHitPossible(uint8_t x, uint8_t y) // Megvizsg�lja, hogy az aktu�lis j�t�kos sz�m�ra ven-e �t�si lehet�s�g.
{
if((ActualPlayer == KOR_PLAYER && IsKorFigure(Table[y][x])) ||(ActualPlayer == NEGYZET_PLAYER && Table[y][x] == NEGYZET_KIRALY) )
{
	if( 	 (x-2>=0 && y+2<8 && (Table[y+2][x-2] == URES)) && IsEnemyFigure(Table[y+1][x-1])) return 1;// ha balra le 2 mez�vel �res helyet tal�lok, �s k�zben egy ellens�ges b�bu van, akkor tudok �tni
	else if( (x+2<8 && y+2<8 && (Table[y+2][x+2] == URES) && IsEnemyFigure(Table[y+1][x+1]))) return 1;// ugyan ez jobbra le
}
if((ActualPlayer == NEGYZET_PLAYER && IsNegyzetFigure(Table[y][x])) ||(ActualPlayer == KOR_PLAYER && Table[y][x] == KOR_KIRALY))
{
	if((x-2>=0 && y-2>=0 && (Table[y-2][x-2] == URES) && IsEnemyFigure(Table[y-1][x-1]))) return 1; //balra fel
	else if((x+2<8 && y-2>=0 && (Table[y-2][x+2] == URES) && IsEnemyFigure(Table[y-1][x+1]))) return 1; // jobbra fel
}
return 0;
}


void SelectFigure() // A ki�lasztott b�but felveszi.
{
	// ha lehets�ges �t�s, csak olyan b�but enged felvenni, ami �thet
	if(HitObligation && IsHitPossible(CursorX,CursorY)==0) return;
		FigureType figure = Table[CursorY][CursorX];
		if(IsMyFigure(figure)) // Csak saj�t b�but vehetek fel
		{
			CarriedFigure = figure;
			SetFigure(CursorX,CursorY,KIINDULASI_PONT); // A b�bu eredeti hely�t ponttal jel�l�m.
			PrevX = CursorX;
			PrevY = CursorY;
			GameState = Carry;
		}
}

void PlaceFigure() // A felvett b�but leteszem.
{
	if(Table[CursorY][CursorX] != URES) {ThrowBack(); return;} // b�bura nem l�phet�nk r�
	switch(CarriedFigure)
	{
	case KOR:
		// jobbra/balra fel l�p�s, ha nem kell �tn�m
		if((HitObligation == 0) && (CursorX == PrevX-1 || CursorX == PrevX+1) && CursorY == PrevY+1)
		{
			ShowStep();
			if(CursorY == 7) //Ha el�rtem a t�bla sz�l�t, akkor a b�bu kir�lly� alakul.
			{
				Table[CursorY][CursorX]++;
				DrawMaze(CursorX,CursorY,KOR_KIRALY,INVERT);
			}
		}
		//�tl�s �t�s
		else if((CursorX == PrevX-2 || CursorX == PrevX+2) && CursorY == PrevY+2) //�t� l�p�s
		{
			uint8_t x_avg = (CursorX + PrevX) /2; // k�ztes mez� x koordin�t�ja
			uint8_t y_avg = CursorY-1;// k�ztes mez� y koordin�t�ja
			if(IsEnemyFigure(Table[y_avg][x_avg])) //Ha ellens�ges b�but l�ptem �t.
			{
				assert(HitObligation); // Ennek be kell �ll�tva lennie, k�l�nben l�p�st is engedt�nk volna.
				ShowStep(); // saj�t b�but l�ptetem
				SetFigure(x_avg,y_avg,URES); //le�t�tt b�but leveszem
				DrawMaze(x_avg,y_avg,URES,NO_INVERT); //t�rl�m a bels� t�bl�r�l
				FigureNum[NEGYZET_PLAYER]--;

				//megvizsg�ljuk, lehets�ges-e m�g �t�s, ha igen a j�t�kos l�phet tov�bb
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
				//ha az �t�ssel el�rtem az utols� sort, kir�lly� alakul
				if(CursorY == 7)
				{
					Table[CursorY][CursorX]++;
					DrawMaze(CursorX,CursorY,KOR_KIRALY,INVERT);
				}
			} else {ThrowBack(); return;} //Nem ellens�ges b�but l�ptem �t
		}else {ThrowBack(); return;} // Olyan helyre l�ptem, ami sem l�p�s, sem �t�s nem lehet.

		ChangePlayer();
		return;

		break;
	case NEGYZET:
		// �tl�s l�p�s jobbra/balra le
		if((HitObligation == 0) && (CursorX == PrevX-1 || CursorX == PrevX+1) && CursorY == PrevY-1)
		{
			ShowStep();
			if(CursorY == 0) // Ha el�rtem az utols� sort, a b�bu �talakul kir�lly�
			{
				Table[CursorY][CursorX]++;
				DrawMaze(CursorX,CursorY,NEGYZET_KIRALY,INVERT);
			}
		}
		// �tl�s �t�s
		else if((CursorX == PrevX-2 || CursorX == PrevX+2) && CursorY == PrevY-2)
		{
			uint8_t x_avg = (CursorX + PrevX) /2; // k�ztes mez� x koordin�t�ja
			uint8_t y_avg = CursorY+1;// k�ztes mez� y koordin�t�ja
			if(IsEnemyFigure(Table[y_avg][x_avg]))
			{
				assert(HitObligation);
				ShowStep(); // saj�t b�but l�ptetem
				SetFigure(x_avg,y_avg,URES); //le�t�tt b�but leveszem
				DrawMaze(x_avg,y_avg,URES,NO_INVERT);
				FigureNum[KOR_PLAYER]--; //t�rl�m a statisztik�b�l

				if(IsHitPossible(CursorX,CursorY)) // Ha m�g lehets�ges �t�s, akkor a j�t�kos folytathatja a j�t�kot.
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

				if(CursorY == 0) // Ha �t�ssel el�rt�k az utols� sort, akkor �talakulunk kir�lly�.
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
		// egyszer� l�p�s 4 ir�nyban
		if(!HitObligation && (abs(CursorX-PrevX)== 1) && (abs(CursorY-PrevY) == 1))
		{
			ShowStep();
		}
		//ha kett�t l�pt�nk �tl�san, megn�zz�k, �thet�nk-e
		else if(abs(CursorX-PrevX) == 2 && abs(CursorY-PrevY) == 2)
		{
			uint8_t x_avg = (CursorX + PrevX) /2; // k�ztes mez� x koordin�t�ja
			uint8_t y_avg = (CursorY + PrevY)/2; //k�ztes mez� y koordin�t�ja
			if(IsEnemyFigure(Table[y_avg][x_avg])) //ellens�ges b�but �t�tt�nk-e le
			{
				ShowStep();
				SetFigure(x_avg,y_avg,URES);
				DrawMaze(x_avg,y_avg,URES,NO_INVERT);
				FigureNum[OTHER_PLAYER]--;

				if(IsHitPossible(CursorX,CursorY)) // ha m�g tudok �tni, folytathatom
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
		else {ThrowBack(); return;} //ha a l�p�s szab�lytalan lenne

		ChangePlayer(); // szab�lyos l�p�s ut�n j�t�koscsere
		return;
		break;
	default: assert(0);
	}//switch
	assert(0);
}


void RestartGame() // �jra ind�tjuk a j�t�kot.
{
	StopBlinkTimer(); //Le�ll�tjuk a villog�st.
	ToggleSignal = 0; // Ha lenne f�gg� villogtat� utas�t�s, le�ll�tjuk.
	SetInstructionReg();
#ifdef TOGGLE_FULL_SCREEN
	SelectChip1();
	WriteData(DISPLAY_ON); // Ha a teljes k�perny� villogott, akkor azt �jra bekapcsoljuk.
	SelectChip2();
	WriteData(DISPLAY_ON);
#endif

	//�jra inicializ�lom a t�bl�t.
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
	ActualPlayer = (Points[0]+Points[1])%2==0 ? NEGYZET_PLAYER : KOR_PLAYER; // A j�t�kosok felv�ltva kezdenek -> a pontok �sszege a parti sz�m�t adja.
	GameState = Search;
	HitObligation = 0;
	HitChain = 0;
	CursorX = 0;
	CursorY = 0;

	SetupScreen();
}
