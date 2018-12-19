#ifndef CONFIG_H
#define CONFIG_H

// Dimensions of table
#define ROW 3
#define COL 3

// Characters
#define SUBMARINE 'S'
#define WATER '~'

// Communication
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015
#define SERVER_IP "127.0.0.1"

#define NUM_OF_PLAYERS 2 // Number of expected clients

// Messages
#define READY "R"
#define WON "W"
#define LOST "L"
#define PASSIVE "P"
#define ACTIVE "A"
#define MISS " "
#define HIT "X"

#endif // CONFIG_H
