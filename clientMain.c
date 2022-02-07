#include "utilities/commonUtilities.h"
#include "utilities/clientUtilities.h"

#include "clientCommands/clientHelp.h"
#include "clientCommands/clientHandleCommand.h"

int main(int argc, char **argv) {
    int command, port;
    char buffer[MAXLINE] = {0};
    char commandChar[2];

    parseClientArguments(argc, argv, &port);
    clientSetSignals();
    printf("Client avviato.\n");

    while (1) {
        printf("Inserisci un comando o digita '0' per la lista dei comandi disponibili.\n");

        fflush(stdout);
        while (fgets(buffer, MAXLINE, stdin) == NULL);

        if (buffer[strlen(buffer) - 1] != '\n') {
            printf("Comando eccessivamente lungo.\n");
        } else {
            //sostituisco il carattere '\n' con quello di terminazione stringa
            buffer[strlen(buffer) - 1] = '\0';

            // comando da eseguire
            strncpy(commandChar, buffer, 1);
            commandChar[1] = '\0';
            errno = 0; // per catturare EINVAL
            command = (int) strtol(commandChar, NULL, 10);

            if ((errno == EINVAL) || (errno == ERANGE)) {
                printf("La conversione del comando è fallita.\n");
                continue;
            }

            switch (command) {
                case 0:
                    clientHelp();
                    break;
                case 1:
                case 2:
                case 3:
                    clientInitializeSocket(port, argv[2]);
                    clientHandleCommand(buffer);
                    closeSocket(socketDescriptor);
                    socketDescriptor = -1;
                    break;
                default:
                    printf("Il comando inserito non è stato riconosciuto.\n");
            }
        }
        bzero((void *) buffer, MAXLINE);
    }
}