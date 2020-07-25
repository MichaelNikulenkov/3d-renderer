#include "exceptions.hpp"

const char* exc_subsystems_init::what() const throw() {
	return "ERROR: FAILED TO INITIALIZE SUBSYSTEMS";
}

const char* exc_window_creation::what() const throw() {
	return "ERROR: FAILED TO CREATE WINDOW";
}

const char* exc_renderer_creation::what() const throw() {
	return "ERROR: FAILED TO CREATE RENDERER";
}