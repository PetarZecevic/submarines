/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
    
    Ime fajla:      player.c
    Opis:           TCP/IP klijent
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include<stdio.h>      //printf
#include<string.h>     //strlen
#include<sys/socket.h> //socket
#include<arpa/inet.h>  //inet_addr
#include <fcntl.h>     //for open
#include <unistd.h>    //for close

#include "game.h"

int main(int argc , char *argv[])
{
    int playerSocket;
    struct sockaddr_in server;

    //Create socket
    playerSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (playerSocket == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    //Connect to remote server
    if (connect(playerSocket , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
	        return 1;
    }
    puts("Connected\n");	
	
	// After connection is successfull start game loop.
	GameLoop(playerSocket);

    close(playerSocket);

    return 0;
}
