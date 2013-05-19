Ship *ship;

//TODO WASD + mouse
void handleKeyPress(XEvent e){
	//cout << "press: ";
	switch(getKS(e)){
	case XK_Left:
	case XK_a:
		//cout << "left" << endl;
		k.left = true;
		break;
	case XK_Right:
	case XK_d:
		//cout << "right" << endl;
		k.right = true;
		break;
	case XK_Up:
	case XK_w:
		//cout << "up" << endl;
		k.up = true;
		break;
	case XK_Down:
	case XK_s:
		//cout << "down" << endl;
		k.down = true;
		break;
	case XK_space:
		paused = !paused;
		break;
	default:
		//cout << "?" << endl;
		break;
	}
}

void handleKeyRelease(XEvent e){
	//cout << "release: ";
	switch(getKS(e)){
	case XK_Left:
	case XK_a:
		//cout << "left" << endl;
		k.left = false;
		break;
	case XK_Right:
	case XK_d:
		//cout << "right" << endl;
		k.right = false;
		break;
	case XK_Up:
	case XK_w:
		//cout << "up" << endl;
		k.up = false;
		break;
	case XK_Down:
	case XK_s:
		//cout << "down" << endl;
		k.down = false;
		break;
	case XK_q:
		loop = false;
		break;
	default:
		//cout << "?" << endl;
		break;
	}
}

void handleModify(XEvent e){
	if((e.xconfigure.height != game.scrH)||(e.xconfigure.width != game.scrW)){
		game.scrH = e.xconfigure.height;
		game.scrW = e.xconfigure.width;
		initStars();
		//cout << "width: " << game.scrW << " height: " << game.scrH << endl;
	}
}

void handleMouseMove(XEvent e){
	mouseX = e.xmotion.x;
	mouseY = e.xmotion.y;
}

void handleMousePress(){
	mouseDown = true;
	//cout << "butt " << mouseX << "," << mouseY << endl;
}

void handleMouseRelease(){
	mouseDown = false;
	//cout << "fart " << mouseX << "," << mouseY << endl;
}

void handleEv(XEvent e){
	switch(e.type){
	case ConfigureNotify:
		handleModify(e);
		break;
	case ButtonPress:
		handleMousePress();
		break;
	case ButtonRelease:
		handleMouseRelease();
		break;
	case MotionNotify:
		handleMouseMove(e);
		break;
	case KeymapNotify:
		XRefreshKeyboardMapping(&e.xmapping);
		break;
	case KeyPress:
		handleKeyPress(e);
		break;
	case KeyRelease:
		handleKeyRelease(e);
		break;
	}
}

XColor col(const char* s){
	XColor c;
	XParseColor(game.dsp, game.cmap, s, &c);
	XAllocColor(game.dsp, game.cmap, &c);
	return c;
}

vector<particle> activeParts;
vector<particle> bufferParts;
vector<laser> activeLaser;
vector<laser> buffer;

void handleParticles(){
	if(activeParts.size() > 0){
		bufferParts.clear();
		for(int i=0; i<activeParts.size(); i++){
			activeParts[i].life--;
			if(activeParts[i].life >= 0){
				if(activeParts[i].grav){
					activeParts[i].vy += gravity;
				}

				activeParts[i].vy *= drag;
				activeParts[i].vx *= drag;

				if((activeParts[i].x + activeParts[i].vx) > play.tpt[0].x && (activeParts[i].x + activeParts[i].vx) < play.tpt[terrainLen-1].x){
					int seg = (activeParts[i].x + activeParts[i].vx)/9;//twidth
					float y1 = play.tpt[seg].y;
					float y2 = play.tpt[seg+1].y;
					float xd = (activeParts[i].x + activeParts[i].vx) - play.tpt[seg].x;
					float gLevel = y1 + ((xd/9)*(y2-y1));
					if(activeParts[i].collide && ((activeParts[i].y + activeParts[i].vy) >= gLevel)){
						activeParts[i].vx = (-activeParts[i].vx)*damp;
						activeParts[i].vy = (-activeParts[i].vy)*damp;
					}else{
						activeParts[i].x += activeParts[i].vx;
						activeParts[i].y += activeParts[i].vy;
					}
				}

				bufferParts.push_back(activeParts[i]);
			}
		}
		activeParts.swap(bufferParts);
	}
}

void handleRenderParticles(float dcx, float dcy){
	for(int i=0; i<activeParts.size(); i++){
		if(activeParts[i].life > 0){
			XSetForeground(game.dsp, game.gc, activeParts[i].color[(activeParts[i].cLen * activeParts[i].life)/activeParts[i].lifeMax]);
		}else{
			XSetForeground(game.dsp, game.gc, game.col.black);
		}
		XDrawPoint(game.dsp, game.win, game.gc, activeParts[i].x + dcx, activeParts[i].y + dcy);
	}
}

void addParticle(float x, float y, float vx, float vy, int life, bool grav, ulong color[], bool coll = true){
	particle part;
	part.x = x;
	part.y = y;
	part.vx = vx;
	part.vy = vy;
	part.lifeMax = life;
	part.life = life+1;
	part.color = color;
	part.cLen = g_yellow;
	part.grav = grav;
	part.collide = coll;
	activeParts.push_back(part);
}

void collisionParticles(float x, float y, float vx, float vy, int life, bool grav, ulong color[]){
	int gen = (int)((vx*vx) + (vy*vy))*2;
	for(int i=0; i<gen; i++){
		if((vx*vx)<9){
			if(vx > 0){
				vx = 3;
			}else{
				vx = -3;
			}
		}

		if((vy*vy)<9){
			if(vy > 0){
				vy = 3;
			}else{
				vy = -3;
			}
		}

		addParticle(x, y, randFlt(-0.5,0.3)*vx, randFlt(-0.5,0.3)*vy, life, grav, color);
	}
}

void handleLasers(){
	if(activeLaser.size() > 0){
		buffer.clear();
		for(int i=0; i<activeLaser.size(); i++){
			if((activeLaser[i].x + activeLaser[i].vx) > play.tpt[0].x && (activeLaser[i].x + activeLaser[i].vx) < play.tpt[terrainLen-1].x){
				int seg = (activeLaser[i].x + activeLaser[i].vx)/9;//twidth
				float y1 = play.tpt[seg].y;
				float y2 = play.tpt[seg+1].y;
				float xd = (activeLaser[i].x + activeLaser[i].vx) - play.tpt[seg].x;
				float gLevel = y1 + ((xd/9)*(y2-y1));

				activeLaser[i].x += activeLaser[i].vx;
				activeLaser[i].y += activeLaser[i].vy;

				//laser particles
				if(random(0,1)){
					addParticle(activeLaser[i].x, activeLaser[i].y, randFlt(0.1,0.2)*activeLaser[i].vx, randFlt(0.1,0.2)*activeLaser[i].vy, 30, true, game.col.gradLaser, false);
				}

				if((activeLaser[i].y) >= gLevel){
					activeLaser[i].active = false;

					if(play.terrainDestruct){
						play.tpt[seg].y++;
						play.tpt[seg+1].y++;
					}

					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradLaser, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradLaser, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradLaser, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradLaser, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradPulse, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradPulse, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradYellow, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradYellow, false);
					addParticle(activeLaser[i].x, activeLaser[i].y, -randFlt(0.2,0.4)*activeLaser[i].vx, -randFlt(0.2,0.4)*activeLaser[i].vy, 60, true, game.col.gradYellow, false);
				}else if(activeLaser[i].y > altLim){
					buffer.push_back(activeLaser[i]);
				}
			}
		}
		activeLaser.swap(buffer);
	}
}

void addLaser(float x, float y, float vx, float vy){
	laser las;
	las.x = x;
	las.y = y;
	las.vx = vx;
	las.vy = vy;
	activeLaser.push_back(las);
}

void drawLaser(float dcx, float dcy){
	XSetForeground(game.dsp, game.gc, game.col.laser);
	for(int i=0; i<activeLaser.size(); i++){
		XDrawLine(game.dsp, game.win, game.gc, activeLaser[i].x-activeLaser[i].vx+dcx, activeLaser[i].y-activeLaser[i].vy+dcy, activeLaser[i].x+dcx, activeLaser[i].y+dcy);
	}
}

short fireWait = fireDelay;

void handleFire(float dcx, float dcy){
	if(mouseDown && (fireWait == 0)){
		fireWait = fireDelay;
		float angle = atan2(mouseX-dcx-ship->x, mouseY-dcy-ship->y)-(PI/2);
		float vx = cos(angle)*laserSpeed*randFlt(1-laserSpray, 1+laserSpray);
		float vy = -sin(angle)*laserSpeed*randFlt(1-laserSpray, 1+laserSpray);
		addLaser(ship->x+vx+random(-laserRad, laserRad), ship->y+vy+random(-laserRad, laserRad), vx, vy);
	}else if(fireWait > 0){
		fireWait--;
	}
}

//should rememver to streamline change colors
void handleRenderStars(){
	XSetForeground(game.dsp, game.gc, game.col.star);
	for(int i=0; i<starCount; i++){
		if(play.stars[i].type == 0){
			XDrawPoint(game.dsp, game.win, game.gc, play.stars[i].x, play.stars[i].y);
		}else{
			XDrawLine(game.dsp, game.win, game.gc, play.stars[i].x-1, play.stars[i].y, play.stars[i].x+1, play.stars[i].y);
			XDrawLine(game.dsp, game.win, game.gc, play.stars[i].x, play.stars[i].y-1, play.stars[i].x, play.stars[i].y+1);
		}
	}
}

void physics(){
	ship->vy += gravity;

	if(k.up){
		ship->vy -= accel;
		addParticle(ship->x, ship->y, ship->vx + randFlt(-pulseSpread,pulseSpread), ship->vy + randFlt(0.4,0.8)*pulse, 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(-pulseSpread,pulseSpread), ship->vy + randFlt(0.4,0.8)*pulse, 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(-pulseSpread,pulseSpread), ship->vy + randFlt(0.4,0.8)*pulse, 120, true, game.col.gradPulse);

	}
	if(k.down){
		ship->vy += accel;
		addParticle(ship->x, ship->y, ship->vx + randFlt(-pulseSpread,pulseSpread), ship->vy + randFlt(0.4,0.8)*-pulse, 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(-pulseSpread,pulseSpread), ship->vy + randFlt(0.4,0.8)*-pulse, 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(-pulseSpread,pulseSpread), ship->vy + randFlt(0.4,0.8)*-pulse, 120, true, game.col.gradPulse);
	}
	if(k.left){
		ship->vx -= accel;
		addParticle(ship->x, ship->y, ship->vx + randFlt(0.4,0.8)*pulse, ship->vy + randFlt(-pulseSpread,pulseSpread), 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(0.4,0.8)*pulse, ship->vy + randFlt(-pulseSpread,pulseSpread), 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(0.4,0.8)*pulse, ship->vy + randFlt(-pulseSpread,pulseSpread), 120, true, game.col.gradPulse);
	}
	if(k.right){
		ship->vx += accel;
		addParticle(ship->x, ship->y, ship->vx + randFlt(0.4,0.8)*-pulse, ship->vy + randFlt(-pulseSpread,pulseSpread), 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(0.4,0.8)*-pulse, ship->vy + randFlt(-pulseSpread,pulseSpread), 120, true, game.col.gradPulse);
		addParticle(ship->x, ship->y, ship->vx + randFlt(0.4,0.8)*-pulse, ship->vy + randFlt(-pulseSpread,pulseSpread), 120, true, game.col.gradPulse);
	}

	ship->vx *= drag;
	ship->vy *= drag;

	if(ship->hitXY(play.tpt,terrainLen,9)){
		ship->vy = -(ship->vy)*damp;
		ship->vx = -(ship->vx)*damp;
		float dmg = sqrt((ship->vx * ship->vx)+(ship->vy * ship->vy));
		if(dmg > 1){
			ship->H -= 100*dmg;
			if(ship->H < 0){
				ship->H = 0;
			}
			//cout << ship->H << endl;

			collisionParticles(ship->x, ship->y, ship->vx, ship->vy, random(colSparkLife-colSparkLifeVar, colSparkLife+colSparkLifeVar), true, game.col.gradYellow);
		}
	}else if((ship->y+ship->vy > altLim)&&(ship->x+ship->vx > xMin)&&(ship->x+ship->vx < xMax)){
		ship->move();
	}else{
		ship->vx*=-damp;
		ship->vy*=-damp;
	}

	handleLasers();
	handleParticles();
}

//float sccX = (game.scrW/2);
//float sccY = (game.scrH/2);

float sccX = shipInitX;
float sccY = 0;

float sccVX = 0;
float sccVY = 0;

void camera(){
	//float dx = sccX - ship->x;
	//float dy = sccY - ship->y;

	//float dx = game.scrW/2 - ship->x;
	//float dy = game.scrH/2 - ship->y;

	sccVX += (ship->x - sccX)/50;
	sccVY += (ship->y - sccY)/50;

	sccVX *= cameraDamp;
	sccVY *= cameraDamp;

	sccX += sccVX;
	sccY += sccVY;
	
	//sccX -= dx;
	//sccY -= dy;

	float dcx = game.scrW/2 - sccX;
	float dcy = game.scrH/2 - sccY;

	//ship->x += dx;
	//ship->y += dy;


	float minY = -9001;
	float bkminY = -9001;

	XPoint drtr[terrainLen+2];
	XPoint bkdrtr[bkTerrainLen+2];

	for(int i=1; i<(terrainLen-1); i++){
		drtr[i].x = play.tpt[i].x + dcx;
		drtr[i].y = play.tpt[i].y + dcy;

		if(drtr[i].y > minY){
			minY = drtr[i].y;
		}
	}

	for(int i=1; i<(bkTerrainLen-1); i++){
		bkdrtr[i].x = play.bktpt[i].x + dcx/2 + (game.scrW/4);
		bkdrtr[i].y = play.bktpt[i].y + dcy/2;

		if(bkdrtr[i].y > bkminY){
			bkminY = bkdrtr[i].y;
		}
	}

	//cout << bkdrtr[3].x << "," << bkdrtr[3].y << endl;

	drtr[0].x = drtr[1].x;
	drtr[0].y = minY + (game.scrH/2);
	drtr[terrainLen-1].x = drtr[terrainLen-2].x;
	drtr[terrainLen-1].y = minY + (game.scrH/2);

	bkdrtr[0].x = bkdrtr[1].x;
	bkdrtr[0].y = bkminY + (game.scrH/2);
	bkdrtr[bkTerrainLen-1].x = bkdrtr[bkTerrainLen-2].x;
	bkdrtr[bkTerrainLen-1].y = bkminY + (game.scrH/2);

	handleFire(dcx, dcy);

	//XSetForeground(game.dsp, game.gc, game.col.white);
	//XDrawLines(game.dsp, game.win, game.gc, drtr, terrainLen, CoordModeOrigin);
	drawPoly(game, bkdrtr, bkTerrainLen, game.col.gray24, game.col.gray24);

	drawLaser(dcx,dcy);
	ship->drawPos(game, ship->x + dcx, ship->y + dcy);
	handleRenderParticles(dcx,dcy);
	drawPoly(game, drtr, terrainLen, game.col.gray16, game.col.gray16);
}

void handleGameMessages(){
	if((ship->y < altLim+boundWarn)||(ship->x < xMin+boundWarn)||(ship->x > xMax-boundWarn)){
		drawText(game, gms.boundary);
	}

	if(game.scrW < defW || game.scrH < defH){
		drawText(game, gms.small);
	}
}

void handleGameRender(){
	//regen for now
	if(paused){
		XSetForeground(game.dsp, game.gc, game.col.black);
		XFillRectangle(game.dsp, game.win, game.gc, 0, 0, game.scrW, game.scrH);
		drawText(game, gms.pause);
	}else{
		if(ship->H < ship->maxH){
			ship->H++;
		}

		physics();

		XSetForeground(game.dsp, game.gc, game.col.black);
		XFillRectangle(game.dsp, game.win, game.gc, 0, 0, game.scrW, game.scrH);

		handleRenderStars();

		camera();

		XSetForeground(game.dsp, game.gc, game.col.white);
		XDrawLine(game.dsp, game.win, game.gc, 10, 8, 210, 8);

		for(int i=10; i<(ship->H/5)+10; i+=2){
			XDrawLine(game.dsp, game.win, game.gc, i, 10, i, 15 + (500/i));
		}

		handleGameMessages();
	}
}
