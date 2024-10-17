#include "libs.h"

bool range(int min, int max, int value) {
    return (min <= value && value <= max);
}

void defineColors() {
    //definisce le coppie di colori
    if (has_colors()) {
        init_pair(0, COLOR_WHITE, COLOR_BLUE);		//info, strada
        init_pair(1, COLOR_WHITE, COLOR_RED);		//car
    	init_pair(2, COLOR_RED, COLOR_WHITE);		//car
        init_pair(3, COLOR_WHITE, COLOR_BLUE);	 	//fiume
        init_pair(4, COLOR_CYAN, COLOR_MAGENTA);	//marciapiede
        init_pair(5, COLOR_MAGENTA, COLOR_GREEN);	//tane
        init_pair(6, COLOR_YELLOW, COLOR_GREEN);	//player  
    	init_pair(7, COLOR_MAGENTA, COLOR_YELLOW);	//trochi	
    }
}

void clearScreen() {
	for(int i = 0; i <= MAXX + 10; i++){
		for(int j = 0; j <= MAXY + 10; j++){
			mvaddch(j, i, ' ');
		}
	}
}
