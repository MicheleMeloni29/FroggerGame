#ifndef STRUCT_H_INCLUDED
#define STRUCT_H_INCLUDED

/* Struttura per la comunicazione tra figli e padre */
typedef struct position_struct {
    int x;        //posizione x
    int y;        //posizione y
} pos;

/* Enum per definire una serie di messaggi da mostrare per le varie collisioni ogni volta che rinizia una manche */
typedef enum restart_msg { CRASH  = 0, SPLASH = 1, ENEMY = 2, CLOSED = 3, TIME = 4, WIN = 5 } restart_msg;

#endif
