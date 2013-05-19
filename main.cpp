/*
 * Taking 370 and I have come to realize the obscene amounts of floating point
 * blasphemies that have been committed inside this assignment, however the author has
 * yet to correct these issues as the number of ffff...ornifications he gives is 0
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
//#include <X11/xpm.h>
#include <X11/Xresource.h>
#include <X11/keysymdef.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <sys/time.h>

using namespace std;

//color vars
#define g_yellow 16
#define g_pulse 16
#define g_laser 16

#include "definitions.h"
#include "ship.h"
#include "handles.h"

unsigned long cc(const char* s){
	XColor c;
	XParseColor(game.dsp, game.cmap, s, &c);
	XAllocColor(game.dsp, game.cmap, &c);
	return c.pixel;
}

string c2hex(float r, float g, float b){
	int red = (int)r;
	int green = (int)g;
	int blue = (int)b;
	int decInt = (red*65536)+(green*256)+blue;
	//cout << decInt << endl;
	stringstream ss;
	ss << "#" << setfill('0') << setw(6) << hex << decInt;
	return ss.str();
}

void initColor(){
	//basic stuff
	game.col.black = BlackPixel(game.dsp, game.scrN);
	game.col.white = WhitePixel(game.dsp, game.scrN);
	game.cmap = DefaultColormap(game.dsp, game.scrN);

	//custom stuff
	game.col.gray192 = cc("#c0c0c0");
	game.col.gray128 = cc("#808080");
	game.col.gray64 = cc("#404040");
	game.col.gray32 = cc("#202020");
	game.col.gray24 = cc("#181818");
	game.col.gray16 = cc("#101010");

	game.col.star = cc("#ffff80");
	game.col.red = cc("#ff0000");
	game.col.dred = cc("#800000");
	game.col.laser = cc("#c0ffcc");
	game.col.blue = cc("#60ffff");

	for(int i=0; i<g_yellow; i++){
		float ratio = (i+1);
		ratio /= g_yellow;
		char *cg = (char*)c2hex(255*ratio, 200*ratio, 100*ratio).c_str();
		game.col.gradYellow[i] = cc(cg);
	}

	for(int i=0; i<g_pulse; i++){
		float ratio = (i+1);
		ratio /= g_pulse;
		char *cg = (char*)c2hex(96*ratio, 255*ratio, 255*ratio).c_str();
		game.col.gradPulse[i] = cc(cg);
	}

	for(int i=0; i<g_laser; i++){
		float ratio = (i+1);
		ratio /= g_laser;
		char *cg = (char*)c2hex(192*ratio, 255*ratio, 204*ratio).c_str();
		game.col.gradLaser[i] = cc(cg);
	}

	for(int i=0; i<g_laser; i++){
		float ratio = (i+1);
		ratio /= g_laser;
		char *cg = (char*)c2hex(255*ratio, 64*ratio, 64*ratio).c_str();
		game.col.gradLaserRed[i] = cc(cg);
	}
}

void initMessage(msg &m, char* text, float xr, float yr, ulong color){
	m.text = text;
	m.xr = xr;
	m.yr = yr;
	m.color = color;
}

void initMessages(){
	initMessage(gms.boundary, "CAUTION: Nearing Game Boundary!", 0.5, 0.2, game.col.red);
	initMessage(gms.small, "Screen size below 800x600!", 0.5, 0.8, game.col.red);
	initMessage(gms.pause, "Game Paused...", 0.5, 0.5, game.col.white);
}

void init(){
	XAutoRepeatOff(game.dsp);

	game.scrN = DefaultScreen(game.dsp);
	game.scrH = defH;
	game.scrW = defW;
	game.ctrX = defH/2;
	game.ctrY = defW/2;

	//gotta get some vibrance
	initColor();

	//make a simple window
	game.win = XCreateSimpleWindow(game.dsp, DefaultRootWindow(game.dsp), 0, 0, game.scrW, game.scrH, 0, game.col.black, game.col.black);

	//set the events we care about
	XSelectInput(game.dsp, game.win, ButtonPressMask|ButtonReleaseMask|PointerMotionMask|StructureNotifyMask|KeyPressMask|KeyReleaseMask|KeymapStateMask);

	//map to screen
	XMapWindow(game.dsp, game.win);

	//create graphics context
	game.gc = XCreateGC(game.dsp, game.win, 0, 0);

	//yea we really only want solid fills
	XSetFillStyle(game.dsp, game.gc, FillSolid);

	//done
	XFlush(game.dsp);

	//no cheats activated at start
	game.infFuel = false;
	game.hpRegen = false;
	game.enemyDiff = 5;
	game.shipMod = 0;

	//start at main menu
	//game.screen = 0;

	//test auto game start
	game.screen = 2;
	ship = new Ship(maxHP);
	initTerrain();
	initStars();

	initMessages();

	XUndefineCursor(game.dsp, game.win);
}

//test
#define verts 3
XPoint tri[verts];

//Font font = XLoadFont(game.dsp, "6x9");
//XSetFont(game.dsp, game.gc, font);

char *msg = "YOLO SWAG TROLLOLOL!!";

void update(){

	//test
	/*for(int u=0; u<verts; u++){
		tri[u].x = random(0,400);
		tri[u].y = random(0,400);
	}
	drawRect(game, 0, 0, 400, 400, game.col.black, game.col.white);
	drawPoly(game, tri, verts, game.col.gray64, game.col.white);
	drawLine(game, random(0,100), random(0,100), random(300,400), random(300,400), game.col.gray128);
	drawText(game, msg, random(0,400), random(0,400), game.col.red);*/

	if(game.screen == 2){
		handleGameRender();
	}

	XFlush(game.dsp);
}

int main(){
	srand (time(NULL));

	//connect to Xserver
	game.dsp = XOpenDisplay(0);
	if(!game.dsp){
		fprintf(stderr, "can't connect to display");
		return 1;
	}

	init();

	//test
	/*Ship *s1 = new Ship(6669);
	//s1->getM();
	free(s1);*/

	ulong prevTime = 0;
	ulong curTime;

	while(loop){
		//gonna process whatever's pending
		if(XPending(game.dsp)){
			XNextEvent(game.dsp, &ev);
			handleEv(ev);
		}

		//is it time to update?
		curTime = now();
		if((curTime - prevTime) > (uSec/fps)){
			update();
			prevTime = now();
		}

		//don't rest if there is still stuff to do
		if(!XPending(game.dsp)){
			usleep((uSec/fps)-(curTime-prevTime));
		}
	}

	XCloseDisplay(game.dsp);
	return 0;
}
