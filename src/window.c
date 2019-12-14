/*
	Copyright (c) 2017-2018 ByteBit

	This file is part of BetterSpades.

	BetterSpades is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	BetterSpades is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with BetterSpades.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

void window_textinput(int allow) {}

void window_setmouseloc(double x, double y) {}

static void window_impl_mouseclick(GLFWwindow* window, int button, int action, int mods) {
	int b = 0;
	switch(button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			b = WINDOW_MOUSE_LMB;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			b = WINDOW_MOUSE_RMB;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			b = WINDOW_MOUSE_MMB;
			break;
	}
	int a = -1;
	switch(action) {
		case GLFW_RELEASE:
			a = WINDOW_RELEASE;
			break;
		case GLFW_PRESS:
			a = WINDOW_PRESS;
			break;
	}
	mouse_click(hud_window,b,a,mods>0);
}
static void window_impl_mouse(GLFWwindow* window, double x, double y) {
	mouse(hud_window,x,y);
}
static void window_impl_mousescroll(GLFWwindow* window, double xoffset, double yoffset) {
	mouse_scroll(hud_window,xoffset,yoffset);
}
static void window_impl_error(int i, const char* s) {
	on_error(i,s);
}
static void window_impl_reshape(GLFWwindow* window, int width, int height) {
	reshape(hud_window,width,height);
}
static void window_impl_textinput(GLFWwindow* window, unsigned int codepoint) {
	text_input(hud_window,codepoint);
}
static void window_impl_keys(GLFWwindow* window, int key, int scancode, int action, int mods) {
	int a = -1;
	switch(action) {
		case GLFW_RELEASE:
			a = WINDOW_RELEASE;
			break;
		case GLFW_PRESS:
			a = WINDOW_PRESS;
			break;
	}
	int tr = window_key_translate(key,0);
	if(tr>=0)
		keys(hud_window,tr,scancode,a,mods>0);
	else
		tr = WINDOW_KEY_UNKNOWN;
	if(hud_active->input_keyboard)
		hud_active->input_keyboard(tr,action,mods,key);
}

char* window_keyname(int keycode) {
	return glfwGetKeyName(keycode,0)!=NULL?(char*)glfwGetKeyName(keycode,0):"?";
}

float window_time() {
	return glfwGetTime();
}

int window_pressed_keys[64] = {0};

const char* window_clipboard() {
	return glfwGetClipboardString(hud_window->impl);
}

int window_key_translate(int key, int dir) {
	return config_key_translate(key,dir);
}

int window_key_down(int key) {
	return window_pressed_keys[key];
}

void window_mousemode(int mode) {
	int s = glfwGetInputMode(hud_window->impl,GLFW_CURSOR);
	if((s==GLFW_CURSOR_DISABLED && mode==WINDOW_CURSOR_ENABLED)
	|| (s==GLFW_CURSOR_NORMAL && mode==WINDOW_CURSOR_DISABLED))
		glfwSetInputMode(hud_window->impl,GLFW_CURSOR,mode==WINDOW_CURSOR_ENABLED?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);
}

void window_mouseloc(double* x, double* y) {
	glfwGetCursorPos(hud_window->impl,x,y);
}

void window_swapping(int value) {
	glfwSwapInterval(value);
}

void window_title(char* suffix) {
	if(suffix) {
		char title[128];
		snprintf(title,sizeof(title)-1,"FourSpades %s - %s",FOURSPADES_VERSION,suffix);
		glfwSetWindowTitle(hud_window->impl,title);
	} else {
		glfwSetWindowTitle(hud_window->impl,"FourSpades "FOURSPADES_VERSION);
	}
}

void window_init() {
	static struct window_instance i;
	hud_window = &i;

	glfwWindowHint(GLFW_VISIBLE,0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);

	glfwSetErrorCallback(window_impl_error);

	if(!glfwInit()) {
		log_fatal("GLFW3 init failed");
    	exit(1);
	}

	if(settings.multisamples>0) {
		glfwWindowHint(GLFW_SAMPLES,settings.multisamples);
	}

	hud_window->impl = glfwCreateWindow(settings.window_width,settings.window_height,"FourSpades "FOURSPADES_VERSION,settings.fullscreen?glfwGetPrimaryMonitor():NULL,NULL);
	if(!hud_window->impl) {
		log_fatal("Could not open window");
		glfwTerminate();
		exit(1);
	}

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(hud_window->impl,(mode->width-settings.window_width)/2.0F,(mode->height-settings.window_height)/2.0F);
	glfwShowWindow(hud_window->impl);

	glfwMakeContextCurrent(hud_window->impl);

	glfwSetFramebufferSizeCallback(hud_window->impl,window_impl_reshape);
	glfwSetCursorPosCallback(hud_window->impl,window_impl_mouse);
	glfwSetKeyCallback(hud_window->impl,window_impl_keys);
	glfwSetMouseButtonCallback(hud_window->impl,window_impl_mouseclick);
	glfwSetScrollCallback(hud_window->impl,window_impl_mousescroll);
	glfwSetCharCallback(hud_window->impl,window_impl_textinput);

//	if(glfwRawMouseMotionSupported())
//		glfwSetInputMode(hud_window->impl,GLFW_RAW_MOUSE_MOTION,GLFW_TRUE);
}

void window_fromsettings() {
	glfwWindowHint(GLFW_SAMPLES,settings.multisamples);
	glfwSetWindowSize(hud_window->impl,settings.window_width,settings.window_height);

	if(settings.vsync<2)
		window_swapping(settings.vsync);
	if(settings.vsync>1)
		window_swapping(0);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if(settings.fullscreen)
		glfwSetWindowMonitor(hud_window->impl,glfwGetPrimaryMonitor(),0,0,settings.window_width,settings.window_height,mode->refreshRate);
	else
		glfwSetWindowMonitor(hud_window->impl,NULL,(mode->width-settings.window_width)/2,(mode->height-settings.window_height)/2,settings.window_width,settings.window_height,0);
}

void window_deinit() {
	glfwTerminate();
}

void window_update() {
	glfwSwapBuffers(hud_window->impl);
	glfwPollEvents();
}

int window_closed() {
	return glfwWindowShouldClose(hud_window->impl);
}

int window_cpucores() {
	#ifdef OS_LINUX
		return get_nprocs();
	#endif
	#ifdef OS_WINDOWS
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return info.dwNumberOfProcessors;
	#endif
	return 1;
}
