//uber lazy coder
typedef unsigned long ulong;
#define uSec 1000000
#define PI 3.14159265
#define charWidth 6
#define cameraDamp 0.93

//default window dimensions
#define defH 600
#define defW 800

//frame rate
#define fps 60

//used for keyboard input processing
#define keyBuffSize 3

//physics variables
#define gravity 0.0666	//constant downward acceleration
#define damp 0.42		//the reverse multiplier for bounce after collision
#define drag 0.993		//air resistance multiplier
#define accel 0.16		//thruster acceleration strength
#define pulse 2
#define pulseSpread 0.3

//game variables
#define maxHP 1000
#define fireDelay 0
#define laserSpray 0.05
#define laserRad 2
#define laserSpeed 15
#define terrainLen 1200
#define bkTerrainLen 300
#define colSparkLife 240
#define colSparkLifeVar 120
#define starCount 200
#define starMax 5
#define shipInitX 5000
#define altLim -4000
#define boundWarn 500
#define xMin 1600
#define xMax 9200
#define padCount 7

//need to define abs bounds

struct colorPallete{
	//basic colors
	ulong black;
	ulong white;

	//custom colors
	ulong gray192;
	ulong gray128;
	ulong gray64;
	ulong gray32;
	ulong gray24;
	ulong gray16;

	ulong star;
	ulong red;
	ulong blue;
	ulong laser;
	ulong dred;

	ulong gradYellow[g_yellow];
	ulong gradPulse[g_pulse];
	ulong gradLaser[g_laser];
	ulong gradLaserRed[g_laser];
};

//container for important game information
struct gInfo{
	//X components
	Display *dsp;
	Window win;
	GC gc;
	Colormap cmap;

	//window vars
	int scrH;	//screen height
	int scrW;	//screen width
	int scrN;	//screen number
	int ctrX;	//center X
	int ctrY;	//center Y

	//cheat/mod vars
	bool infFuel;	//infinite fuel
	bool hpRegen;	//regenerating hull
	int enemyDiff;		//determines difficulty of enemies 0 ~ 9 (passive ~ death god)

	//game vars
	int shipMod;	//selected ship design, purely for looks
	int screen;		//current screen, 0 = main menu, 1 = mod/cheat menu, 2 = game, 3 = endscreen

	//color pallete
	colorPallete col;
};

//need to reset starts on resize
struct star{
	int x,y;
	short type;//0,1,2
};

struct pad{
	int l,r; //within the abs limit the index of terrain points
};

struct alien{
	float x, y, vx, vy, rad;
	int hp;
};

vector<alien> aliens;

//container for playtime information
//should be initialized anew by the same call that starts each game
struct pInfo{
	XPoint tpt[terrainLen];
	XPoint bktpt[bkTerrainLen];
	bool terrainDestruct;
	int maxX;
	star stars[starCount];
	pad pads[padCount];
};

struct msg{
	char* text;
	ulong color;
	float xr,yr;
};

struct gmsg{
	msg boundary;
	msg small;
	msg pause;
};

//tracers for control states
struct keys{
	bool up;
	bool down;
	bool left;
	bool right;
};

//simple struct for some particles
struct particle{
	bool grav, collide;
	float x,y,vx,vy;
	int life, lifeMax;
	ulong *color;
	short cLen;
};

//Imma firing ma lazer!
struct laser{
	float x,y,vx,vy;
	bool active;
};

//init game information structures
gInfo game;
gmsg gms;
pInfo play;
XEvent ev;
keys k;
bool loop = true;
int mouseX, mouseY;
bool mouseDown;
bool paused = false;

//get current microsecond time
ulong now(){
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000000 + tv.tv_usec;
}

//get key symbol
KeySym getKS(XEvent e){
	KeySym ks;
	int cc;
	char buffer[keyBuffSize];
	XComposeStatus cp;
	cc = XLookupString(&e.xkey, buffer, keyBuffSize, &ks, &cp);
	return ks;
}

int random(int min, int max){
	return (rand() % (max - min + 1)) + min;
}

float randFlt(float min, float max){
	return min + (float)rand()/((float)RAND_MAX/(max-min));
}

//some patterned random function
float fTerrain(int dist, int varRad, int amplitude){
	return (sin(dist*PI/180)*amplitude) + random(-varRad, varRad);
}

//probably follow a randomized composite sine function
void initTerrain(){
	play.terrainDestruct = false;

	int stretchFactor = 3;
	int rOff1 = random(0,360);
	int rOff2 = random(0,360);
	int rOff3 = random(0,360);
	int rOff4 = random(0,360);
	int rOffMaster = random(0,360);
	play.maxX = (terrainLen-1)*3*stretchFactor;
	for(int i=0; i<terrainLen; i++){
		play.tpt[i].x = i*3*stretchFactor;
		play.tpt[i].y = fTerrain(rOff1 + i*29, 5, 20) +
						fTerrain(rOff2 + i*7, 4, 35*stretchFactor) +
						fTerrain(rOff3 + i*3, 3, 35) +
						fTerrain(rOff4 + i*2, 3, 50) +
						fTerrain(rOffMaster + i*stretchFactor, 0, 120*stretchFactor) +
						600;
	}

	rOff2 = random(0,360);
	rOff3 = random(0,360);
	rOff4 = random(0,360);
	rOffMaster = random(0,360);
	for(int i=0; i<bkTerrainLen; i++){
		play.bktpt[i].x = (i*3*stretchFactor*(terrainLen/bkTerrainLen))/2;
		play.bktpt[i].y = 	fTerrain(rOff2 + i*14, 8, 20*stretchFactor) +
							fTerrain(rOff3 + i*6, 6, 20) +
							fTerrain(rOff4 + i*16, 6, 36) +
							fTerrain(rOffMaster + i*stretchFactor*4, 10, 80) +
							500;
	}
}

void initStars(){
	for(int i=0; i<starCount; i++){
		play.stars[i].x = random(0,game.scrW);
		play.stars[i].y = random(0,game.scrH);
		play.stars[i].type = random(0,1);
	}
}

//custom drawing functions
//consider adding internal check for drawing off screen so we avoid requesting those
void drawLine(const gInfo g, float x1, float y1, float x2, float y2, ulong c){
	XSetForeground(g.dsp, g.gc, c);
	XDrawLine(g.dsp, game.win, g.gc, x1, y1, x2, y2);
}

void drawRect(const gInfo g, float x, float y, float w, float h, ulong cFill, ulong cEdge){
	XSetForeground(g.dsp, g.gc, cFill);
	XFillRectangle(g.dsp, g.win, g.gc, x, y, w, h);
	if(cFill != cEdge){
		XSetForeground(g.dsp, g.gc, cEdge);
		XDrawLine(g.dsp, g.win, g.gc, x, y, x+w, y);
		XDrawLine(g.dsp, g.win, g.gc, x, y, x, y+h);
		XDrawLine(g.dsp, g.win, g.gc, x+w, y+h, x+w, y);
		XDrawLine(g.dsp, g.win, g.gc, x+w, y+h, x, y+h);
	}
}

void drawPoly(const gInfo g, XPoint v[], int vLen, ulong cFill, ulong cEdge){
	XSetForeground(g.dsp, g.gc, cFill);
	XFillPolygon(g.dsp, g.win, g.gc, v, vLen, Complex, CoordModeOrigin);
	if(cFill != cEdge){
		XSetForeground(g.dsp, g.gc, cEdge);
		XDrawLines(g.dsp, g.win, g.gc, v, vLen, CoordModeOrigin);
		XDrawLine(g.dsp, g.win, g.gc, v[0].x, v[0].y, v[vLen-1].x, v[vLen-1].y);
	}
}

void drawCirc(const gInfo g, float x, float y, float rad, ulong cFill, ulong cEdge){
	XSetForeground(g.dsp, g.gc, cFill);
	XFillArc(g.dsp, g.win, g.gc, x-rad, y-rad, 2*rad, 2*rad, 0, 360*64);
	if(cFill != cEdge){
		XSetForeground(g.dsp, g.gc, cEdge);
		XDrawArc(g.dsp, g.win, g.gc, x-rad, y-rad, 2*rad, 2*rad, 0, 360*64);
	}
}

void drawText(const gInfo g, msg m){
	XSetForeground(g.dsp, g.gc, m.color);
	XDrawString(g.dsp, g.win, g.gc, g.scrW*m.xr-((strlen(m.text)*charWidth)/2), g.scrH*m.yr, m.text, strlen(m.text));
}
