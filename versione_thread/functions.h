#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

void defineColors();						// Inizializza i colori per ncurses

void clearScreen();							// Clear della mappa di gioco

bool range(int min, int max, int value);	// COntrolla se value è compreso tra min e max

#endif
