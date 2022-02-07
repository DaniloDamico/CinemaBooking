void clientHandleCommand(char *command) {
    int readingOutcome;
    char buffer[MAXLINE + 1];

    // invia il comando al server
    if (!writeSocket(socketDescriptor, command, (int) strlen(command) + 1)) {
        perror("Errore nell'invio del comando.");
        return;
    }

    while ((readingOutcome = readSocket(socketDescriptor, buffer)) == 0);

    if (readingOutcome == -1) {
        perror("Errore nella lettura della risposta del server.");
        return;
    }

    if (printf("%s", buffer) <= 0) {
        perror("Errore nella stampa del messaggio ricevuto");
        return;
    }

    printf("\n");
}
