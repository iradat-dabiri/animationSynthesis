#include "interface.h"

Fl_Window* main_window = (Fl_Window*)0;

// button group - load ASF/AMC data
Fl_Button* loadSkeleton_button = (Fl_Button*)0;
Fl_Button* loadMotion_button = (Fl_Button*)0;
Fl_Button* resetScene_button = (Fl_Button*)0;

//button group - motion play
Fl_Button* rewind_button = (Fl_Button*)0;
Fl_Button* pause_button = (Fl_Button*)0;
Fl_Button* play_button = (Fl_Button*)0;
Fl_Button* repeat_button = (Fl_Button*)0;
Fl_Button* plusOne_button = (Fl_Button*)0;
Fl_Button* minusOne_button = (Fl_Button*)0;

Fl_Value_Slider* frame_slider = (Fl_Value_Slider*)0;

Fl_Light_Button* worldAxes_button = (Fl_Light_Button*)0;

Player_Gl_Window* glwindow = (Player_Gl_Window*)0;

Fl_Window* make_window() {
	Fl_Window* w; {
		Fl_Window* o = main_window = new Fl_Window(741, 622, "Motion Capture Player");
		w = o; {
			Fl_Group* o = new Fl_Group(10, 485, 615, 140); 
			{
				Fl_Button* o = loadSkeleton_button = new Fl_Button(10, 495, 120, 40, "Load Skeleton");
				o->callback((Fl_Callback*)load_callback);
			}
			{
				Fl_Button* o = loadMotion_button = new Fl_Button(140, 495, 120, 40, "Load Motion");
				o->callback((Fl_Callback*)load_callback);
			}
			{
				Fl_Button* o = resetScene_button = new Fl_Button(400, 495, 120, 40, "Reset Scene");
				o->callback((Fl_Callback*)resetScene_callback);
			}
			{
				Fl_Button* o = pause_button = new Fl_Button(430, 575, 35, 25, "@||");
				o->labeltype(FL_SYMBOL_LABEL);
				o->labelsize(12);
				o->callback((Fl_Callback*)play_callback);
			}
			{
				Fl_Button* o = rewind_button = new Fl_Button(465, 575, 35, 25, "@|<");
				o->labeltype(FL_SYMBOL_LABEL);
				o->labelsize(12);
				o->callback((Fl_Callback*)play_callback);
			}
			{
				Fl_Button* o = play_button = new Fl_Button(500, 575, 35, 25, "@>");
				o->labeltype(FL_SYMBOL_LABEL);
				o->labelsize(12);
				o->callback((Fl_Callback*)play_callback, (void*)(0));
			}
			{
				Fl_Button* o = minusOne_button = new Fl_Button(535, 575, 35, 25, "-1");
				o->labeltype(FL_SYMBOL_LABEL);
				o->labelsize(12);
				o->callback((Fl_Callback*)play_callback, (void*)(0));
			}
			{
				Fl_Button* o = plusOne_button = new Fl_Button(570, 575, 35, 25, "+1");
				o->labeltype(FL_SYMBOL_LABEL);
				o->labelsize(12);
				o->callback((Fl_Callback*)play_callback, (void*)(0));
			}
			{
				Fl_Button* o = repeat_button = new Fl_Button(605, 575, 35, 25, "@<->");
				o->labeltype(FL_SYMBOL_LABEL);
				o->labelsize(12);
				o->callback((Fl_Callback*)play_callback, (void*)(0));
			}
			o->end();
		} //FL_group

		{
			Player_Gl_Window* o = glwindow = new Player_Gl_Window(5, 5, 640, 480, "label");
			o->box(FL_DOWN_FRAME);
			o->labeltype(FL_NO_LABEL);
		}

		{
			Fl_Value_Slider* o = frame_slider = new Fl_Value_Slider(10, 545, 630, 20);  // slider
			o->type(5);
			o->labelsize(18);
			o->minimum(1);
			o->maximum(60000);
			o->step(1);
			o->callback((Fl_Callback*)fslider_callback, (void*)(0));
			o->align(197);
		}

		{
			Fl_Light_Button* o = worldAxes_button = new Fl_Light_Button(650, 375, 85, 25, "Axes");
			o->callback((Fl_Callback*)renderWorldAxes_callback, (void*)(0));
			o->align(FL_ALIGN_INSIDE);
		}
		o->end();
	}
	return w;
}
