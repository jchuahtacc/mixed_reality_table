/*
 TUIO C++ Server Demo
 
 Copyright (c) 2005-2012 Martin Kaltenbrunner <martin@tuio.org>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDED_SimpleSimulator_H
#define INCLUDED_SimpleSimulator_H

#include <tuio/TuioServer.h>
#include <tuio/TuioCursor.h>
#include <tuio/osc/OscTypes.h>
#include <list>
#include <math.h>

#include <tuio/FlashSender.h>
#include <tuio/TcpSender.h>

#include <SDL.h>
#include <SDL_thread.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

using namespace TUIO;

class SimpleSimulator { 
	
public:
	SimpleSimulator(TuioServer *server);
	~SimpleSimulator() {};
	
	void run();
	TuioServer *tuioServer;
	std::list<TuioCursor*> stickyCursorList;
	std::list<TuioCursor*> jointCursorList;
	std::list<TuioCursor*> activeCursorList;
	
private:
	void drawFrame();
	void drawString(const char *str);
	void processEvents();
	void initWindow();

	SDL_Surface *window;
	bool verbose, fullupdate, periodic, fullscreen, running, help;
	
	int width, height;
	int screen_width, screen_height;
	int window_width, window_height;
	TuioTime frameTime;
	
	void mousePressed(float x, float y);
	void mouseReleased(float x, float y);
	void mouseDragged(float x, float y);
	//int s_id;
};

#endif /* INCLUDED_SimpleSimulator_H */
