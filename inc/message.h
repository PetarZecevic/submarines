#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>
#include <stdbool.h>

/*
Wrap around recv function, for easier error messages handling.
Returns true if there is no error, and false if there is error.
*/
extern bool WrapperRecv(int socket, void* buffer, size_t bufferLength, char* errorBuffer);

/*
Wrap around send function for easier error messages handling.
Returns true if there is no error, and false if there is error.
*/
extern bool WrapperSend(int socket, void* buffer, size_t bufferLength, char* errorBuffer);

#endif // MESSAGE_H