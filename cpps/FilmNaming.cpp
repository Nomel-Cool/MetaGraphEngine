#include "FilmNaming.h"

void FilmNameManager::SetCurrentFilmName(const std::string& film_name)
{
    current_film_name = film_name;
}

std::string FilmNameManager::GetCurrentFilmName()
{
    return current_film_name;
}

void FilmNameManager::ResetCurrentFilmName()
{
    current_film_name = "";
}
