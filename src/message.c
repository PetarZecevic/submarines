#include <string.h>
#include <sys/socket.h>
#include "message.h"
#include <stdio.h>

/*
Wrap around recv function, for easier error messages handling.
*/
bool WrapperRecv(int socket, void* buffer, size_t bufferLength, char* errorBuffer)
{
	bool success = true;
	int flag = recv(socket, buffer, bufferLength, 0);
	if(flag == 0)
	{
		strcpy(errorBuffer, "Other side disconnected!");
		success = false;	
	}
	else if(flag == -1)
	{
		strcpy(errorBuffer, "recv failed!");
		success = false;
	}
	return success;
}

/*
Wrap around send function for easier error messages handling.
*/
bool WrapperSend(int socket, void* buffer, size_t bufferLength, char* errorBuffer)
{
	bool success = true;
	int flag = send(socket, buffer, bufferLength, 0);
	if(flag <= 0)
	{
		strcpy(errorBuffer, "send failed!");
		success = false;
	}
	return success;
}