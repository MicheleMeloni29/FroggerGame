#ifndef STRUCT_H_INCLUDED
#define STRUCT_H_INCLUDED

typedef struct position_struct {
    int x;
    int y;
} position_s;

typedef enum entity_enum { PLAYER  = 0, CAR = 1, LOG = 2, BULLET = 3} entity_type;

typedef struct entity_struct {
    char extra;
    int type;
    int health;
    bool has_enemy;
    position_s pos;
} entity_s;

typedef struct {
    int remainingTime;  // tempo rimanente in secondi
} time_update_msg;

#endif // STRUCT_H_INCLUDED
