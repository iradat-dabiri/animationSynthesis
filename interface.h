#ifndef interface_h
#define interface_h

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Value_Slider.H>
#include "mocapPlayer.h"

extern Fl_Window* main_window;

extern Fl_Button* play_button;
extern Fl_Button* pause_button;
extern Fl_Button* rewind_button;
extern Fl_Button* repeat_button;
extern Fl_Button* plusOne_button;
extern Fl_Button* minusOne_button;
extern void play_callback(Fl_Button*, void*);

extern Fl_Button* loadSkeleton_button;
extern Fl_Button* loadMotion_button;
extern void load_callback(Fl_Button*, void*);

extern Fl_Button* resetScene_button;
extern void resetScene_callback(Fl_Button*, void*);

extern Fl_Value_Slider* frame_slider;
extern void fslider_callback(Fl_Value_Slider*, long);

extern Fl_Light_Button* worldAxes_button;
extern void renderWorldAxes_callback(Fl_Light_Button*, long);

extern Player_Gl_Window* glwindow;

Fl_Window* make_window();

#endif