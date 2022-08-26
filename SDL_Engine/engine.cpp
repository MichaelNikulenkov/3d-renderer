#include "engine.hpp"

Engine::Engine() {}
Engine::~Engine() {}

void Engine::start(const char* title, int x_pos, int y_pos, int width, int height, bool is_fullscreen) {
	int fulscreen_flag = 0;
	if (is_fullscreen) {
		fulscreen_flag = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		std::cout << "Subsystems initialized..." << std::endl;

		_window = SDL_CreateWindow(title, x_pos, y_pos, width, height, fulscreen_flag);
		if (_window) {
			std::cout << "Window created..." << std::endl;
		}
		else throw exc_window_creation();

		_renderer = SDL_CreateRenderer(_window, -1, 0);
		if (_renderer) {
			SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 1);
			std::cout << "Renderer created..." << std::endl;
		}
		else throw exc_renderer_creation();

		_screen_height = height;
		_screen_width = width;

		_is_running = true;

		onStart();

		auto tp1 = std::chrono::system_clock::now();
		auto tp2 = std::chrono::system_clock::now();

		while (_is_running) {
			_start_clock = SDL_GetTicks();

			// Handle Timing
			tp2 = std::chrono::system_clock::now();
			std::chrono::duration<float> elapsed_time = tp2 - tp1;
			tp1 = tp2;
			float f_elapsed_time = elapsed_time.count();
			
			handleEvents(f_elapsed_time);
			onUpdate(f_elapsed_time);
			updateTitle();
		    
			_delta_clock = SDL_GetTicks() - _start_clock;
			_start_clock = SDL_GetTicks();

			if (_delta_clock != 0)
				_current_FPS = 1000 / _delta_clock;
		}

		clean();
	}
	else {
		_is_running = false;
		throw exc_subsystems_init();
	}		
}

void Engine::updateTitle() {

	std::string s = std::to_string(_current_FPS);
	char const *pchar = s.c_str();  
	SDL_SetWindowTitle(_window, pchar);
}

void Engine::handleEvents(float f_elapsed_time) {
	SDL_Event engine_event;
	SDL_PollEvent(&engine_event);
	switch (engine_event.type)
	{
	case SDL_QUIT:
		_is_running = false;
		break;
		/* Look for a keypress */
	case SDL_KEYDOWN:
		/* Check the SDLKey values and move change the coords */
		vec3 v_forward = vectorMul(_v_look_dir, 18.0f * f_elapsed_time);
		switch (engine_event.key.keysym.sym) {
		case SDLK_UP:
			_v_camera.y += 28.0f * f_elapsed_time;	// Travel Upwards
			break;

		case SDLK_DOWN:
			_v_camera.y -= 28.0f * f_elapsed_time;	// Travel Downwards
			break;

		case SDLK_LEFT:
			_v_camera.x -= 28.0f * f_elapsed_time;	// Travel Along X-Axis
			break;

		case SDLK_RIGHT:
			_v_camera.x += 28.0f * f_elapsed_time;	// Travel Along X-Axis
			break;
		case SDLK_a:
			_f_yaw -= -18.0f * f_elapsed_time;

			break;
		case SDLK_d:
			_f_yaw += -18.0f * f_elapsed_time;
			break;

		case SDLK_w:			
			_v_camera = vectorAdd(_v_camera, v_forward);

			break;
		case SDLK_s:
			_v_camera = vectorSub(_v_camera, v_forward);
			break;
		}
	}
}

void Engine::onStart(){
	// Load object file
	_mesh_ñube.LoadFromObjectFile("Models\\teapot.obj");

	// Projection Matrix
	_mat_proj = matrixMakeProjection(90.0f, (float)_screen_height / (float)_screen_width, 0.1f, 1000.0f);
}

void Engine::onUpdate(float f_elapsed_time) {
	// Clear Screen
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 1);
	SDL_RenderClear(_renderer);

	// Set up rotation matrices
	mat4x4 mat_rot_z, mat_rot_x;

	//_f_theta += 1.0f * f_elapsed_time;
	mat_rot_z = matrixMakeRotationZ(M_PI/*_f_theta * 0.5f*/);
	mat_rot_x = matrixMakeRotationX(0/*_f_theta*/);

	mat4x4 mat_trans;
	mat_trans = matrixMakeTranslation(0.0f, 0.0f, 5.0f);

	mat4x4 mat_world;
	mat_world = matrixMakeIdentity();	// Form World Matrix
	mat_world = matrixMultiplyMatrix(mat_rot_z, mat_rot_x); // Transform by rotation
	mat_world = matrixMultiplyMatrix(mat_world, mat_trans); // Transform by translation

	// Create "Point At" Matrix for camera
	vec3 v_up = { 0,1,0 };
	vec3 v_target = { 0, 0, 1 };//vectorAdd(_v_camera, _v_look_dir);
	mat4x4 mat_camera_rot = matrixMakeRotationY(_f_yaw);
	_v_look_dir = matrixMultiplyVector(mat_camera_rot, v_target);
	v_target = vectorAdd(_v_camera, _v_look_dir);

	mat4x4 mat_camera = Matrix_PointAt(_v_camera, v_target, v_up);

	// Make view matrix from camera
	mat4x4 mat_view = matrixQuickInverse(mat_camera);

	// Store triagles for rastering later
	std::vector<triangle> vec_triangles_to_raster;

	// Draw Triangles
	for (auto tri : _mesh_ñube.tris)
	{
		triangle tri_projected, tri_transformed, tri_viewed;

		// World Matrix Transform
		tri_transformed.p[0] = matrixMultiplyVector(mat_world, tri.p[0]);
		tri_transformed.p[1] = matrixMultiplyVector(mat_world, tri.p[1]);
		tri_transformed.p[2] = matrixMultiplyVector(mat_world, tri.p[2]);

		// Calculate triangle Normal
		vec3 normal, line1, line2;

		// Get lines either side of triangle
		line1 = vectorSub(tri_transformed.p[1], tri_transformed.p[0]);
		line2 = vectorSub(tri_transformed.p[2], tri_transformed.p[0]);

		// Take cross product of lines to get normal to triangle surface
		normal = vectorCrossProduct(line1, line2);

		// You normally need to normalise a normal!
		normal = vectorNormalise(normal);

		// Get Ray from triangle to camera
		vec3 v_camera_ray = vectorSub(tri_transformed.p[0], _v_camera);

		// If ray is aligned with normal, then triangle is visible
		if (vectorDotProduct(normal, v_camera_ray) < 0.0f)
		{
			// Illumination
			vec3 light_direction = { 0.0f, 1.0f, -1.0f };
			light_direction = vectorNormalise(light_direction);

			// How "aligned" are light direction and triangle surface normal?
			float dp = std::max(0.1f, vectorDotProduct(light_direction, normal));

			// Choose console colours as required (much easier with RGB)
			tri_viewed.color = getColour(dp);

			// Convert World Space --> View Space
			tri_viewed.p[0] = matrixMultiplyVector(mat_view, tri_transformed.p[0]);
			tri_viewed.p[1] = matrixMultiplyVector(mat_view, tri_transformed.p[1]);
			tri_viewed.p[2] = matrixMultiplyVector(mat_view, tri_transformed.p[2]);

			// Clip Viewed Triangle against near plane, this could form two additional
				// additional triangles. 
			int n_clipped_triangles = 0;
			triangle clipped[2];
			n_clipped_triangles = triangleClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, tri_viewed, clipped[0], clipped[1]);

			for (int n = 0; n < n_clipped_triangles; n++) {
				// Project triangles from 3D --> 2D
				tri_projected.p[0] = matrixMultiplyVector(_mat_proj, clipped[n].p[0]);
				tri_projected.p[1] = matrixMultiplyVector(_mat_proj, clipped[n].p[1]);
				tri_projected.p[2] = matrixMultiplyVector(_mat_proj, clipped[n].p[2]);
				tri_projected.color = clipped[n].color;

				// Scaling into view, we move the normalising into cartesian space
				tri_projected.p[0] = vectorDiv(tri_projected.p[0], tri_projected.p[0].w);
				tri_projected.p[1] = vectorDiv(tri_projected.p[1], tri_projected.p[1].w);
				tri_projected.p[2] = vectorDiv(tri_projected.p[2], tri_projected.p[2].w);

				// Offset verts into visible normalised space
				vec3 v_offset_view = { 1,1,0 };
				tri_projected.p[0] = vectorAdd(tri_projected.p[0], v_offset_view);
				tri_projected.p[1] = vectorAdd(tri_projected.p[1], v_offset_view);
				tri_projected.p[2] = vectorAdd(tri_projected.p[2], v_offset_view);

				tri_projected.p[0].x *= 0.5f * (float)_screen_width;
				tri_projected.p[0].y *= 0.5f * (float)_screen_height;
				tri_projected.p[1].x *= 0.5f * (float)_screen_width;
				tri_projected.p[1].y *= 0.5f * (float)_screen_height;
				tri_projected.p[2].x *= 0.5f * (float)_screen_width;
				tri_projected.p[2].y *= 0.5f * (float)_screen_height;

				// Store triangle for sorting
				vec_triangles_to_raster.push_back(tri_projected);
			}		
		}		
	}

	// Sort triangles from back to front
	std::sort(vec_triangles_to_raster.begin(), vec_triangles_to_raster.end(), [](triangle &t1, triangle &t2)
	{
		float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
		float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
		return z1 > z2;
	});

	for (auto &tri_to_raster : vec_triangles_to_raster)
	{
		

		// Clip triangles against all four screen edges, this could yield
		// a bunch of triangles, so create a queue that we traverse to 
		//  ensure we only test new triangles generated against planes			 
		triangle clipped[2];
		std::list<triangle> list_triangles;

		// Add initial triangle
		list_triangles.push_back(tri_to_raster);
		int n_new_triangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (n_new_triangles > 0)
			{
				// Take triangle from front of queue
				triangle test = list_triangles.front();
				list_triangles.pop_front();
				n_new_triangles--;

				// Clip it against a plane. We only need to test each 
				// subsequent plane, against subsequent new triangles
				// as all triangles after a plane clip are guaranteed
				// to lie on the inside of the plane. I like how this
				// comment is almost completely and utterly justified
				switch (p)
				{
				case 0:	nTrisToAdd = triangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 1:	nTrisToAdd = triangleClipAgainstPlane({ 0.0f, (float)_screen_height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 2:	nTrisToAdd = triangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 3:	nTrisToAdd = triangleClipAgainstPlane({ (float)_screen_width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				// Clipping may yield a variable number of triangles, so
				// add these new ones to the back of the queue for subsequent
				// clipping against next planes
				for (int w = 0; w < nTrisToAdd; w++)
					list_triangles.push_back(clipped[w]);
			}
			n_new_triangles = list_triangles.size();
		}


		// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
		for (auto &t : list_triangles)
		{

			//FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);
			//DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_BLACK);
		   // Rasterize triangle
		   SDL_Color color_black = { 0, 0, 0, 1 };
		   SDL_Color color_white = { 255, 255, 255, 1 };
		   fillTriangle(t.p[0].x, t.p[0].y,
			   t.p[1].x, t.p[1].y,
			   t.p[2].x, t.p[2].y, t.color);
           /*drawTriangle(t.p[0].x, t.p[0].y,
					 t.p[1].x, t.p[1].y,
					 t.p[2].x, t.p[2].y, color_white);*/

		}

	}

	SDL_RenderPresent(_renderer);
}

void Engine::render() {
	SDL_RenderClear(_renderer);
	//render some stuff
	SDL_RenderPresent(_renderer);
}

void Engine::clean() {
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
	SDL_Quit();
	std::cout << "Subsystems cleaned" << std::endl;
}

void Engine::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, SDL_Color color) {
	SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(_renderer, x1, y1, x2, y2);
	SDL_RenderDrawLine(_renderer, x2, y2, x3, y3);
	SDL_RenderDrawLine(_renderer, x3, y3, x1, y1);
}

void Engine::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, SDL_Color color) {

	filledTrigonColor(_renderer, x1, y1, x2, y2, x3, y3, SDL_FOURCC(color.r, color.g, color.b, -1));
	/*SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
	SDL_RenderDrawLine(_renderer, x1, y1, x2, y2);
	SDL_RenderDrawLine(_renderer, x2, y2, x3, y3);
	SDL_RenderDrawLine(_renderer, x3, y3, x1, y1);*/
}