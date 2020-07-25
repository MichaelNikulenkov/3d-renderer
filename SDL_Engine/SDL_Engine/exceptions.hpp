#pragma once
#include <exception>

class exc_subsystems_init : public std::exception {
public:
	const char* what() const throw();
};

class exc_window_creation : public std::exception {
public:
	const char* what() const throw();
};

class exc_renderer_creation : public std::exception {
public:
	const char* what() const throw();
};