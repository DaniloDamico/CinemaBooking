#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zconf.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>

#define MAXLINE 1024
#define BACKLOG 1024

int convertStringToNaturalInt(char *string) {
    int integer;
    long conversionResult;

    errno = 0; // per catturare EINVAL
    conversionResult = (int) strtol(string, NULL, 10);

    if ((errno == EINVAL) || (errno == ERANGE) && (conversionResult == LONG_MIN || conversionResult == LONG_MAX)) {
        printf("La conversione richiesta è fallita.\n");
        return -1;
    }

    if (conversionResult < 0) {
        printf("Il valore convertito non è un numero naturale.\n");
        return -1;
    }

    integer = (int) conversionResult;
    return integer;
}

int readSocket(int socketDescriptor, char *buffer) {
    int numberOfBytesRead;
    char byteRead = ' ';

    for (int i = 0; i < MAXLINE - 1; i++) {
        if ((numberOfBytesRead = recv(socketDescriptor, &byteRead, 1, 0)) == 1) {
            buffer[i] = byteRead;
            if (byteRead == '\0')
                break;
        } else {
            if (numberOfBytesRead == 0 && i == 0)
                return 0;
            if (numberOfBytesRead == 0 && i != 0) {
                *buffer = '\0';
                break;
            }
            if (errno != EINTR)
                return -1;
        }
    }
    return 1;
}

void closeSocket(int socketDescriptor) {
    retry_close_socket:
    if (close(socketDescriptor) < 0) {
        if (errno == EINTR) {
            goto retry_close_socket;
        } else {
            perror("Errore nella chiusura di un socket");
            exit(EXIT_FAILURE);
        }
    }
}

int writeSocket(int socketDescriptor, char *source, int sourceSize) {
    int numberOfBytesWritten;
    char *currentOffset;

    currentOffset = source;

    while (sourceSize) {
        if ((numberOfBytesWritten = send(socketDescriptor, source, sourceSize, 0)) <= 0) {
            if (errno == EINTR)
                numberOfBytesWritten = 0;
            else
                return -1;
        }
        sourceSize -= numberOfBytesWritten;
        currentOffset += numberOfBytesWritten;
    }
    return 1;
}