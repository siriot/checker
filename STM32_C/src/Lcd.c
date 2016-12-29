#include "bsp.h"

/*
 * LCD koordin�t�k:
 *
 * |----------|-----------|
 * |          |           |
 * |  chip 1  |   chip2   |
 * |          |           ^
 * |----------|-------<---| page = 0
 *                       ^
 *                       |
 *                    Column = 0
 *
 */

/*
 * LCD-re rajzol� f�ggv�nyek.
 */
void InitLcd()
{
	LcdReset(1);
	LcdReset(0);

	SetInstructionReg();
	SetDataDirOut();
	SelectChip1();
	WriteData(DISPLAY_ON); // Bekapcsoljuk a kijelz�t.
	SelectChip2();
	WriteData(DISPLAY_ON);
}

void TogglePieces()
{
	static uint8_t DisplayState = 1;
	DisplayState = !DisplayState;

#ifdef TOGGLE_FULL_SCREEN
	SetInstructionReg();
	SelectChip1();
	WriteData(DISPLAY_OFF + DisplayState);
	SelectChip2();
	WriteData(DISPLAY_OFF + DisplayState);
#else
	if(DisplayState) DrawTable(PATTERN_ON,NO_SCROLL); // A b�buk villogtat�s�val, cs�sztat�s n�lk�l �jrarajzoljuk a t�bl�t.
	else DrawTable(PATTERN_OFF,NO_SCROLL);
#endif
}



char Pattern[6][8]= // B�buk mint�zata
{
		{0xFF,0x81,0x99,0xBD,0xBD,0x99,0x81,0xFF}, //k�r
		{0xFF,0x81,0x99,0xA5,0xA5,0x99,0x81,0xFF}, // k�r kir�ly
		{0xFF,0x81,0xBD,0xBD,0xBD,0xBD,0x81,0xFF}, // n�gyzet
		{0xFF,0x81,0xBD,0xA5,0xA5,0xBD,0x81,0xFF}, //n�gyzet kir�ly
		{0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF}, //�res mez�
		{0xFF,0x81,0x81,0x99,0x99,0x81,0x81,0xFF}  // kiindulasi pont
};

void SetupScreen()
{
	// Teljesen kit�r�lj�k a k�perny� tartalm�t.
	int i,j, chip;
	SetDataDirOut();
	for(chip=1;chip<3;chip++) // Mindk�t k�perny�f�lre ugyan azt hajtjuk v�gre.
	{
		if(chip==1) SelectChip1();
		else SelectChip2();
		// �res byte-ok kivitele.
		for(i=0;i<8;i++)
		{
			SetInstructionReg();
			WriteData(PAGE_ADDRESS+i);
			WriteData(COLUMN_ADDRESS);
			SetDataReg();
			for(j=0;j<64;j++)
				if(i==0) WriteData(0x01);
				else if(i==7) WriteData(0x80);
				else WriteData(0x00);
		}
	}

	// Be�ll�tjuk a sz�veg mez�ket
	Text1 = "     ";
	Text2 = "    ";
	DrawTable(PATTERN_ON,SCROLL); 	// Kirajzoljuk a j�t�kteret a k�toldali szimb�lumokkal �s sz�vegekkel egy�tt.
									// A b�bukat megjelen�tj�k, �s enged�lyezz�k a cs�sztat� anim�ci�t.
}
#define SZIM_NEGYZET 32-4
#define SZIM_KOR 32-3
char PlayerSignal[6] = "     ";
void DrawTable(uint8_t PatternOn, uint8_t Scroll)
{
	/*
	 * PatternOn: PATTERN_OFF �rt�k eset�n csak a t�bla rajzol�dik ki b�buk n�lk�l
	 * Scroll: NO_SCROLL eset�n nincs cs�sztat� anim�ci�
	 */
	// Reverse: Azt adja meg, hogy melyik j�t�kos szemsz�g�b�l kell kirajzolni a t�bl�t.
	int i,j,k;
	SetDataDirOut();

	for(i=0;i<8;i++) 	// soronk�nt kirajzoljuk a t�bl�t �s az esetleges oldals� sz�vegeket
						// A b�buk helyzet�t a game.c-ben tal�lhat� Table t�mb tartalmazza.
	{
		if(i == 7-PLAYER_SIGNAL_LINE) {PlayerSignal[2] = ActualPlayer == KOR_PLAYER ? SZIM_KOR : SZIM_NEGYZET; ShowMessage(PLAYER_SIGNAL_LINE,1,PlayerSignal,5,SIDE_BOTH);}
		else if(i==7-MESSAGE1_LINE) ShowMessage(MESSAGE1_LINE,1,Text1,5,SIDE_BOTH);
		else if(i==7-MESSAGE2_LINE) ShowMessage(MESSAGE2_LINE,1,Text2,4,SIDE_BOTH);
		else if(i == 1)
		{
			char tmp[5]; // Ki�rjuk a j�t�k �ll�s�t a k�perny� k�t oldal�ra
			tmp[4] = 0;
			tmp[3] = '0' + Points[NEGYZET_PLAYER]%10;
			tmp[2] = '0' +(Points[NEGYZET_PLAYER]/10)%10;
			tmp[1] = ':';
			tmp[0] = SZIM_NEGYZET;
			ShowMessage(POINT_LINE,4,tmp,4,SIDE_LEFT);
			tmp[3] = '0' + Points[KOR_PLAYER]%10;
			tmp[2] = '0' + (Points[KOR_PLAYER]/10)%10;
			tmp[0] = SZIM_KOR;
			ShowMessage(POINT_LINE,4,tmp,4,SIDE_RIGHT);
		}

		// Chip2
		SelectChip2();
		SetInstructionReg();
		WriteData(PAGE_ADDRESS+i);
		WriteData(COLUMN_ADDRESS+32);// Kurzor poz�cion�l�sa
		if(Scroll) WriteData(DISPLAY_START_LINE+(8+i*8)%64); // A kezd� sor kijel�l�se a mem�ri�ban.
		SetDataReg();
		for(j=0;j<4;j++)
		{
			FigureType  id= PatternOn ? (Reverse ? Table[i][j] :  Table[7-i][7-j]) : URES;
			for(k=0;k<8;k++) WriteData(Pattern[id][k]);
		}
		// Chip1
		SelectChip1();
		SetInstructionReg();
		WriteData(PAGE_ADDRESS+i);
		WriteData(COLUMN_ADDRESS);
		if(Scroll) WriteData(DISPLAY_START_LINE+(8+i*8)%64);
		SetDataReg();
		for(j=0;j<4;j++)
		{
			FigureType  id= PatternOn ? (Reverse ? Table[i][4+j] : Table[7-i][3-j]) : URES;
			for(k=0;k<8;k++) WriteData(Pattern[id][k]);
		}
		// Cs�sztat�s
		if(Scroll){
		volatile int kk,jj,m;
		//Cs�sztat�s
		for(m=1;m<8 && i<7 ;m+=3)
		{
			for(kk=0;kk<60000;kk++)
				for(jj=0;jj<20;jj++);/*
			SetInstructionReg();
			SelectChip1();
			WriteData(DISPLAY_START_LINE+(8+i*8+m)%64);
			SelectChip2();
			WriteData(DISPLAY_START_LINE+(8+i*8+m)%64);*/
		}// for
		}// if
	}
	MoveCursor(CursorX,CursorY);
}

// Egy adott mez�t rajzolunk ki az LCD-re
// x,y: koordin�t�k. A bel fels� sarok a (0,0)
// figure: a mez�be rajzoland� b�bu t�pusa
// Mask: A b�bu mint�zat�b�l �s ebb�l a b�jtb�l XOR m�velettel k�pzett biteket �rjuk ki a kijelz�re. (A kurzort jelezz�k ezzel.)
void DrawMaze(uint8_t x, uint8_t y, FigureType figure, uint8_t Mask)
{
	assert(IS_COORD_VALID(x,y));

	if(!Reverse)
	{
		if(x<4) SelectChip1();
		else SelectChip2();
	}
	else
	{
		if(x<4) SelectChip2();
		else SelectChip1();
	}

	SetInstructionReg();
	if(!Reverse)
	{
		WriteData(PAGE_ADDRESS+7-y);
		if(x<4) WriteData(COLUMN_ADDRESS+24-8*x);
		else WriteData(COLUMN_ADDRESS +56-8*(x-4));
	}
	else
	{
		WriteData(PAGE_ADDRESS+y);
		if(x<4) WriteData(COLUMN_ADDRESS +32+8*x);
		else WriteData(COLUMN_ADDRESS+8*(x-4));
	}

	SetDataReg();
	int k;
	for(k=0;k<8;k++) WriteData(Pattern[figure][k]^Mask);
}

void MoveCursor(uint8_t x, uint8_t y)
{
	if(x>7 || y>7) return;
	DrawMaze(CursorX,CursorY,Table[CursorY][CursorX],NO_INVERT); //A r�gi mez�t �jra kirajzolom invert�l�s n�lk�l.
	DrawMaze(x,y,Table[y][x],INVERT); // Az �j mez�t invert�lva friss�tem.

	CursorX = x; // �t�rjuk a kurzor �j poz�ci�j�t.
	CursorY = y;
}

void CarryFigure(uint8_t x, uint8_t y) // A kurzorral egy felvett b�but visz�nk, �gy az jelenik meg a kiv�lasztott helyen.
{
	if(x>7 || y>7) return;
	DrawMaze(CursorX,CursorY,Table[CursorY][CursorX],NO_INVERT); //Az el�z�leg kiv�lasztott mez�t �jra rajzolom a saj�t tartalm�val.
	DrawMaze(x,y,CarriedFigure,INVERT); // Az �j mez�re a felvett b�but rajzolom invert�lva.
	CursorX = x;
	CursorY = y;
}
void MoveCursorRight()
{
	MoveCursor(CursorX+1,CursorY);
}
void MoveCursorLeft()
{
	MoveCursor(CursorX-1,CursorY);
}

void MoveCursorUp()
{
	MoveCursor(CursorX,CursorY-1);
}

void MoveCursorDown()
{
	MoveCursor(CursorX,CursorY+1);
}

void CarryFigureRight()
{
	CarryFigure(CursorX+1,CursorY);
}
void CarryFigureLeft()
{
	CarryFigure(CursorX-1,CursorY);
}

void CarryFigureUp()
{
	CarryFigure(CursorX,CursorY-1);
}

void CarryFigureDown()
{
	CarryFigure(CursorX,CursorY+1);
}

/* 5x8-as karaktermint�k
 * Mivel az LCD fejjel lefel� van a panelon, �gy a byte-ok t�kr�zve lettek, tov�bb� az oszlopos sorrendje is fel van cser�lve.
 */

#define ASCII_START 28 // Csak a 32-ik bejegyz�st�l val�s�tjuk meg az ASCII t�bl�t, de a t�bla elej�n 4 egy�ni karakter  is van.
#define CHAR_SPACE 1 // Ennyi oszlop helyet hagyunk a karakterek k�z�tt.
const char ASCII[] = {
		0x00,0x3C,0x3C,0x3C,0x3C,	   // negyzet
		0x00,0x18,0x3C,0x3C,0x18,	   // kor
		0x3c,0x82,0x02,0x82,0x3c,      // �
		0x1c,0xA2,0x22,0xA2,0x1c,      // �
		0x00,0x00,0x00,0x00,0x00,      //
		0x00,0x00,0xfa,0x00,0x00,      //!
		0x00,0xc0,0x00,0xc0,0x00,      //"
		0x28,0x7c,0x28,0x7c,0x28,      //#
		0x48,0x54,0xfe,0x54,0x24,      //$
		0x86,0x66,0x10,0xcc,0xc2,      //%
		0x0a,0x44,0xaa,0x92,0x6c,      //&
		0x00,0x00,0xc0,0xa0,0x00,      //'
		0x00,0x82,0x44,0x38,0x00,      //(
		0x00,0x38,0x44,0x82,0x00,      //)
		0x28,0x10,0x7c,0x10,0x28,      //*
		0x10,0x10,0x7c,0x10,0x10,      //+
		0x00,0x00,0x0c,0x0a,0x00,      //,
		0x10,0x10,0x10,0x10,0x10,      //-
		0x00,0x00,0x06,0x06,0x00,      //.
		0x40,0x20,0x10,0x08,0x04,      ///
		0x7c,0xa2,0x92,0x8a,0x7c,      //0
		0x00,0xfe,0x40,0x20,0x00,      //1
		0x62,0x92,0x8a,0x86,0x42,      //2
		0x6c,0x92,0x92,0x82,0x44,      //3
		0x08,0xfe,0x48,0x28,0x18,      //4
		0x9c,0xa2,0xa2,0xa2,0xe4,      //5
		0x4c,0x92,0x92,0x92,0x7c,      //6
		0xe0,0x90,0x8e,0x80,0x80,      //7
		0x6c,0x92,0x92,0x92,0x6c,      //8
		0x7c,0x92,0x92,0x92,0x64,      //9
		0x00,0x00,0x6c,0x6c,0x00,      //:
		0x00,0x00,0x6c,0x6a,0x00,      //;
		0x00,0x82,0x44,0x28,0x10,      //<
		0x28,0x28,0x28,0x28,0x28,      //=
		0x10,0x28,0x44,0x82,0x00,      //>
		0x60,0x90,0x8a,0x80,0x40,      //?
		0x7a,0xaa,0x9a,0x82,0x7c,      //@
		0x7e,0x90,0x90,0x90,0x7e,      //A
		0x6c,0x92,0x92,0x92,0xfe,      //B
		0x44,0x82,0x82,0x82,0x7c,      //C
		0x7c,0x82,0x82,0x82,0xfe,      //D
		0x82,0x92,0x92,0x92,0xfe,      //E
		0x80,0x90,0x90,0x90,0xfe,      //F
		0x5c,0x92,0x82,0x82,0x7c,      //G
		0xfe,0x10,0x10,0x10,0xfe,      //H
		0x00,0x82,0xfe,0x82,0x00,      //I
		0xfc,0x02,0x02,0x02,0x0c,      //J
		0x82,0x44,0x28,0x10,0xfe,      //K
		0x02,0x02,0x02,0x02,0xfe,      //L
		0xfe,0x40,0x30,0x40,0xfe,      //M
		0xfe,0x10,0x20,0x40,0xfe,      //N
		0x7c,0x82,0x82,0x82,0x7c,      //O
		0x60,0x90,0x90,0x90,0xfe,      //P
		0x7a,0x84,0x84,0x84,0x78,      //Q
		0x6e,0x90,0x90,0x90,0xfe,      //R
		0x4c,0x92,0x92,0x92,0x64,      //S
		0x80,0x80,0xfe,0x80,0x80,      //T
		0xfc,0x02,0x02,0x02,0xfc,      //U
		0xf8,0x04,0x02,0x04,0xf8,      //V
		0xfe,0x04,0x08,0x04,0xfe,      //W
		0x82,0x44,0x38,0x44,0x82,      //X
		0xe0,0x10,0x0e,0x10,0xe0,      //Y
		0xc2,0xa2,0x92,0x8a,0x86,      //Z
		0x00,0x00,0x82,0xfe,0x00,      //[
		0x04,0x08,0x10,0x20,0x40,      //'\'
		0x00,0xfe,0x82,0x00,0x00,      //]
		0x20,0x40,0x80,0x40,0x20,      //^
		0x02,0x02,0x02,0x02,0x02,      //_
		0x00,0x20,0x40,0x80,0x00,      //`
		0x1e,0x2a,0x2a,0x2a,0x04,      //a
		0x1c,0x22,0x22,0x22,0xfe,      //b
		0x22,0x22,0x22,0x22,0x1c,      //c
		0xfe,0x22,0x22,0x22,0x1c,      //d
		0x18,0x2a,0x2a,0x2a,0x1c,      //e
		0xa0,0xa0,0x7e,0x20,0x20,      //f
		0x3c,0x2a,0x2a,0x2a,0x10,      //g
		0x1e,0x20,0x20,0x10,0xfe,      //h
		0x00,0x02,0xbe,0x22,0x00,      //i
		0x00,0xbc,0x22,0x02,0x04,      //j
		0x00,0x22,0x14,0x08,0xfe,      //k
		0x00,0x02,0xfe,0x82,0x00,      //l
		0x1e,0x20,0x1e,0x20,0x3e,      //m
		0x1e,0x20,0x20,0x10,0x3e,      //n
		0x1c,0x22,0x22,0x22,0x1c,      //o
		0x10,0x28,0x28,0x28,0x3e,      //p
		0x3e,0x28,0x28,0x28,0x10,      //q
		0x20,0x20,0x10,0x3e,0x00,      //r
		0x04,0x2a,0x2a,0x2a,0x12,      //s
		0x22,0x22,0xfc,0x20,0x20,      //t
		0x3e,0x04,0x02,0x02,0x3c,      //u
		0x38,0x04,0x02,0x04,0x38,      //v
		0x3c,0x02,0x0c,0x02,0x3c,      //w
		0x22,0x14,0x08,0x14,0x22,      //x
		0x3c,0x0a,0x0a,0x0a,0x30,      //y
		0x22,0x32,0x2a,0x26,0x22,      //z
		0x82,0x82,0x6c,0x10,0x00,      //{
		0x00,0x00,0xfe,0x00,0x00,      //|
		0x00,0x10,0x6c,0x82,0x82,      //}
		0x40,0x20,0x40,0x80,0x40      //~
};

/*
 * Sz�veget jelen�t meg a k�perny� k�t oldals� r�sz�n.
 * Line: A c�l sor sz�ma. Fel�lr�l sz�moljuk 0-t�l kezdve.
 * Offset: Balr�l �resen hagyott oszlopok sz�ma.
 * Text: Sz�veg
 * Length: A kijelzend� karakterek sz�ma.
 */
void ShowMessage(uint8_t Line, uint8_t Offset,char *Text, uint8_t Length, uint8_t Side)
{
	int ch,seg,m;
	for(m=0;m<2;m++) // Mindk�t oldalra ki�rjuk a sz�veget.
	{
		if(m==0 && (Side & SIDE_LEFT)) SelectChip1();
		else if(m == 1 && (Side & SIDE_RIGHT))  SelectChip2();
		else continue;

		SetInstructionReg();
		// Kurzor poz�cion�l�sa.
		WriteData(PAGE_ADDRESS+7-Line);
		WriteData(COLUMN_ADDRESS+64-32*m -Offset - Length*(5+CHAR_SPACE));
		SetDataReg();
		for(ch=0;ch<Length;ch++) //V�gigmegy�nk a sz�veg els� megadott sz�m� karakter�n.
		{
			for(seg=0;seg<CHAR_SPACE;seg++) WriteData(0x00); // Karakterek k�zti sz�netek.
			for(seg=0;seg<5;seg++) // A karaktermint�zat oszlopai
				WriteData(ASCII[ (Text[Length-ch-1]-ASCII_START)*5+seg]); // A sz�vege h�tulr�l kell ki�rni, mivel az LCD fejjel lefel� van.
		}
	}
}
