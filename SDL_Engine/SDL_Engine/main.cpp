// Example program:
// Using SDL2 to create an application window
#include "engine.hpp"

int main(int argc, char* argv[]) {

	Engine* engine = new Engine();
	try {
		engine->start("0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, false);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	
	delete engine;
	system("pause");
	return 0;
}