#pragma once
#include "SDL.h"
#include "SDL2_gfx\SDL2_gfxPrimitives.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>
#include <list>
#include "exceptions.hpp"
#include "utils.hpp"

class Engine {
public:
	Engine();
	~Engine();

	void start(const char* title, int x_pos, int y_pos, int width, int height, bool is_fullscreen);	
private:
	void handleEvents(float f_elapsed_time);
	void onStart();
	void onUpdate(float f_elapsed_time = 0);
	void render();
	void updateTitle();
	void clean();

	void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, SDL_Color color);
	void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, SDL_Color color);

	bool _is_running = false;
	SDL_Window *_window = nullptr;
	SDL_Renderer *_renderer = nullptr;
	vec3 _v_camera;     // Location of camera in world space
	vec3 _v_look_dir;	// Direction vector along the direction camera points
	float _f_yaw;		// FPS Camera rotation in XZ plane
	float _f_theta;     // Spins World transform
	int _screen_height = 0;
	int _screen_width = 0;

	mesh _mesh_ñube;
	mat4x4 _mat_proj;   // Matrix that converts from view space to screen space

	Uint32 _start_clock = 0;
	Uint32 _delta_clock = 0;
	Uint32 _current_FPS = 0;
};