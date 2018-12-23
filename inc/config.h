#ifndef CONFIG_H
#define CONFIG_H

// Dimensions of table
#define ROW 3
#define COL 3

// Characters
#define SUBMARINE 'S'
#define WATER '~'
#define MISS ' '
#define HIT 'X'

// Communication
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015
#define SERVER_IP "127.0.0.1"

#define NUM_OF_PLAYERS 2 // Number of expected clients

// Messages
#define READY "R"
// Confirm messages
#define COORD_LENGTH 2
#define CONFIRM_LENGTH 2
#define LOST_FIELD "LF"
#define MISSED_FIELD "MF"
#define WON_GAME "WG"
#define LOST_GAME "LG"
#define LOST_SUBMARINE_1 "S1"
#define LOST_SUBMARINE_2 "S2"
// Role in game messages
#define PASSIVE "P"
#define ACTIVE "A"

#endif // CONFIG_H
