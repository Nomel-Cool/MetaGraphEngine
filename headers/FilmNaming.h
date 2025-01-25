#pragma once
#ifndef FILM_NAMING_H
#define FILM_NAMING_H

#include <string>

class FilmNameManager
{
public:
	void SetCurrentFilmName(const std::string& film_name);
	std::string GetCurrentFilmName();
	void ResetCurrentFilmName();
private:
	std::string current_film_name = "";
};

#endif // !FILM_NAMING_H
