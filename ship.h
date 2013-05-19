class Ship{

public:
	//position
	float x, y, vx, vy;
	int maxH, H;

	//drawing parts
	XPoint bkWing[12];
	XPoint bkWingRender[12];

	XPoint rWing[5];
	XPoint rWingRender[5];

	XPoint lWing[5];
	XPoint lWingRender[5];

	XPoint head[4];
	XPoint headRender[4];

	XPoint collider[4];

	//construct
	Ship(int mhp){
		x = shipInitX;
		y = 0;
		maxH = mhp;
		H = maxH;

		bkWing[0].x = 0; bkWing[0].y = -3;
		bkWing[1].x = -4; bkWing[1].y = -12;
		bkWing[2].x = -5; bkWing[2].y = -10;
		bkWing[3].x = -3; bkWing[3].y = 0;
		bkWing[4].x = -9; bkWing[4].y = 6;
		bkWing[5].x = -9; bkWing[5].y = 9;
		bkWing[6].x = 0; bkWing[6].y = 4;
		bkWing[7].x = 9; bkWing[7].y = 9;
		bkWing[8].x = 9; bkWing[8].y = 6;
		bkWing[9].x = 3; bkWing[9].y = 0;
		bkWing[10].x = 5; bkWing[10].y = -10;
		bkWing[11].x = 4; bkWing[11].y = -12;

		rWing[0].x = 6; rWing[0].y = -4;
		rWing[1].x = 3; rWing[1].y = 2;
		rWing[2].x = 3; rWing[2].y = 4;
		rWing[3].x = 14; rWing[3].y = -3;
		rWing[4].x = 15; rWing[4].y = -6;

		lWing[0].x = -6; lWing[0].y = -4;
		lWing[1].x = -3; lWing[1].y = 2;
		lWing[2].x = -3; lWing[2].y = 4;
		lWing[3].x = -14; lWing[3].y = -3;
		lWing[4].x = -15; lWing[4].y = -6;

		head[0].x = 0; head[0].y = 2;
		head[1].x = -4; head[1].y = 5;
		head[2].x = 0; head[2].y = 10;
		head[3].x = 4; head[3].y = 5;

		collider[0].x = 14; collider[0].y = -3;
		collider[1].x = -14; collider[1].y = -3;
		collider[2].x = -9; collider[2].y = 9;
		collider[3].x = 9; collider[3].y = 9;
	}

	//pass in const gInfo where needed

	//true if the ship x,y will be under terrain
	bool hit(XPoint t[], int len, float tWidth){
		if(x > t[0].x && x < t[len-1].x){
			int seg = x/tWidth;
			float y1 = t[seg].y;
			float y2 = t[seg+1].y;
			float xd = x - t[seg].x;
			float gLevel = y1 + ((xd/tWidth)*(y2-y1));
			if(y >= gLevel){
				return true;
			}else{
				return false;
			}
		}
		return true;
	}

	//consider different reactions
	bool hitXY(XPoint t[], int len, float tWidth){
		if((x + vx) > t[0].x && (x + vx) < t[len-1].x){
			int seg = (x + vx)/tWidth;
			float y1 = t[seg].y;
			float y2 = t[seg+1].y;
			float xd = (x + vx) - t[seg].x;
			float gLevel = y1 + ((xd/tWidth)*(y2-y1));
			if((y + vy) >= gLevel){
				return true;
			}

			for(int i=0; i<4; i++){
				seg = (x + collider[i].x + vx)/tWidth;
				y1 = t[seg].y;
				y2 = t[seg+1].y;
				xd = (x + collider[i].x + vx) - t[seg].x;
				gLevel = y1 + ((xd/tWidth)*(y2-y1));
				if((y + collider[i].y + vy) >= gLevel){
					return true;
				}
			}
			return false;
		}
		return true;
	}

	//if it's gonna be in ground reverse v vector and don't move, try again in next frame
	void move(){
		x += vx;
		y += vy;
	}

	void drawPos(gInfo game, float sx, float sy){
		//draw: backwings, backplate, void orb, frontwings, head, appends
		for(int i=0; i<12; i++){
			bkWingRender[i].x = bkWing[i].x + sx;
			bkWingRender[i].y = bkWing[i].y + sy;
		}

		for(int i=0; i<5; i++){
			rWingRender[i].x = rWing[i].x + sx;
			rWingRender[i].y = rWing[i].y + sy;
			lWingRender[i].x = lWing[i].x + sx;
			lWingRender[i].y = lWing[i].y + sy;
		}

		for(int i=0; i<4; i++){
			headRender[i].x = head[i].x + sx;
			headRender[i].y = head[i].y + sy;
		}

		drawPoly(game, bkWingRender, 12, game.col.dred, game.col.gray192);
		drawCirc(game, sx, sy, 7, game.col.gray64, game.col.black);
		drawCirc(game, sx, sy, random(3,5), game.col.laser, game.col.blue);
		drawPoly(game, rWingRender, 5, game.col.gray128, game.col.gray64);
		drawPoly(game, lWingRender, 5, game.col.gray128, game.col.gray64);
		XSetForeground(game.dsp, game.gc, game.col.blue);
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		XDrawLine(game.dsp, game.win, game.gc, sx, sy, sx+random(-12,12), sy+random(-12,12));
		drawPoly(game, headRender, 4, game.col.gray128, game.col.gray64);
	}
};
