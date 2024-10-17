#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

//
entity_s bullet(position_s position);
//B
bool checkStreetCollision(entity_s cars[], entity_s player);
bool checkRiverCollision(entity_s player);
bool checkBulletsCollisionWithLogs(entity_s log, entity_s * bullets, int bulletCount);
bool checkBulletsCollisionWithCars(entity_s car, entity_s * bullets, int bulletCount);
bool checkBulletsCollisionWithFrog(entity_s player, entity_s * enemyBullets, int enemyBulletCount);
bool checkBulletsCollisionWithBullets(entity_s * bullets, int bulletCount, entity_s * enemyBullets, int enemyBulletCount);
void clearZone(int endX, int endY);
bool checkNearby(bool right, entity_s current, entity_s cars[]);
bool checkPlayerLimits(int x, int y);
//D
void drawScene(int points, int countdown, int hp, entity_s player, entity_s cars[], entity_s logs[],entity_s * bullets, int bulletCount, entity_s * enemyBullets, int enemyBulletCount, bool lairs[]);
void defineColors();
//I
void initScreen();
void initCars(entity_s * cars);
void initLogs(entity_s * logs);
entity_s initEntity(int type, char extra, int x, int y, int health);
//P
void printEnd(char *msg);
void printCars(entity_s car);
void printLog(entity_s log);
void printPlayer(entity_s player);
//R
bool range(int min, int max, int value);
int rnd(int MAX, int MIN);


#endif
