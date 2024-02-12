//player.h All the user interface functions

#ifndef _PLAYER_H
#define _PLAYER_H

#include <FL/Fl_Gl_Window.H>

class Player_Gl_Window : public Fl_Gl_Window {
public:
  inline Player_Gl_Window(int x, int y, int w, int h, const char *l=0) : 
  Fl_Gl_Window(x, y, w, h, l) {};

  //called to refresh the window 
  void draw(); 

  //called when an event happens inside the space		
  int handle(int event); 
};

typedef struct _MouseT {
	int button;
	int x;
	int y;
} MouseT; 

typedef struct _CameraT {
	double zoom;
	double tw;
	double el;
	double az;
	double tx;
	double ty;
	double tz;
	double atx;
	double aty;
	double atz;
} CameraT;

void GraphicsInit();
void display();

#endif