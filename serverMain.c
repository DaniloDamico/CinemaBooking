#include "utilities/commonUtilities.h"
#include "utilities/serverUtilities.h"

#include "serverCommands/serverSendError.h"
#include "serverCommands/serverBookSeats.h"
#include "serverCommands/serverShowSeats.h"
#include "serverCommands/serverUndoSeatsBooking.h"

int main(int argc, char *argv[]) {
    int command, port, readOutcome;
    char buffer[MAXLINE] = {0}, *arguments;
    char commandChar[2];

    parseServerArguments(argc, argv, &port);
    serverSetSignals();
    createProgramDirectory();
    emptyFile(codesName);
    movieTheatreGrid = createMovieTheatreGrid();
    serverInitializeListeningSocket(port);

    printf("Server avviato.\n");

    while (1) {

        // accetto connessione in arrivo e la gestisco sul connectionSocket
        retry_accept:
        if ((connectionSocket = accept(listeningSocket, NULL, NULL)) == -1) {
            if (errno == EINTR)
                goto retry_accept;
            else {
                perror("Errore nella accept");
                continue;
            }
        }

        while ((readOutcome = readSocket(connectionSocket, buffer)) == 0);

        if (readOutcome == -1) {
            perror("Errore nella lettura di un comando. Non verrà processato.");
            closeSocket(connectionSocket);
            continue;
        }

        // comando da eseguire
        strncpy(commandChar, buffer, 1);
        commandChar[1] = '\0';
        if ((command = convertStringToNaturalInt(commandChar)) == -1) {
            printf("La conversione in intero del comando è fallita.\n");
            serverSendError(connectionSocket);
            continue;
        }

        // argomenti del comando
        if (buffer[1] != '\0') {
            arguments = buffer + 2;
        } else {
            arguments = NULL;
        }

        switch (command) {
            case 1:
                serverShowSeats();
                break;
            case 2:
                serverBookSeats(arguments, codesName);
                break;
            case 3:
                serverUndoSeatsBooking(arguments, codesName);
                break;
            default:
                printf("Errore nella ricezione dei dati.\n");
                serverSendError();
        }

        closeSocket(connectionSocket);
        bzero((void *) buffer, MAXLINE);
    }
}