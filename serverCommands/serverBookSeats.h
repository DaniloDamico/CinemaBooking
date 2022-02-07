void serverBookSeats(char *seats, char *fileName) {
    printf("serverBookSeats richiesto.\n");
    char *currentToken, *currentRowChar, *currentSeatChar;
    char *stringContext;
    char freeSeat = '0', terminator = '\0';
    int currentRow, currentSeat;
    char uniqueCodeLog[MAXLINE] = {0};
    char userMessage[MAXLINE] = {0};
    unsigned long long uniqueCode;
    int fileDescriptor;

    if (checkArgumentsExistance(seats, connectionSocket) == -1)
        return;

    //  un posto è identificato come un numero di fila seguito da un numero di posto.
    //  consumo seats per cercare di prenotare tutti i posti disponibili.

    currentToken = strtok_r(seats, " ", &stringContext);

    if ((fileDescriptor = openFile(fileName)) == -1) {
        perror("Non è stato possibile aprire il file dei codici.");
        serverSendError();
        return;
    }

    while (currentToken != NULL) {
        char *tokenContext;

        // conversione del numero di fila
        if ((currentRowChar = strtok_r(currentToken, ".", &tokenContext)) == NULL) {
            perror("Errore nella lettura del numero di fila dal comando client");
            serverSendError();
            return;
        }

        if ((currentRow = convertStringToNaturalInt(currentRowChar)) == -1) {
            perror("Errore nella conversione del numero di fila dal comando");
            serverSendError();
            return;
        }

        // conversione del numero di posto
        if ((currentSeatChar = strtok_r(NULL, ".", &tokenContext)) == NULL) {
            perror("Errore nella lettura del numero di posto dal comando client");
            serverSendError();
            return;
        }

        if ((currentSeat = convertStringToNaturalInt(currentSeatChar)) == -1) {
            perror("Errore nella conversione del numero di posto dal comando");
            serverSendError();
            return;
        }

        // scrittura
        if (currentRow < rowsNumber && currentSeat < seatsNumber) {

            if (movieTheatreGrid[currentRow][currentSeat] == freeSeat) {

                uniqueCode = generateUniqueBookingCode();

                snprintf(uniqueCodeLog, MAXLINE, "%llu %d %d\n", uniqueCode, currentRow, currentSeat);


                if (write(fileDescriptor, uniqueCodeLog, strlen(uniqueCodeLog)) != strlen(uniqueCodeLog)) {
                    perror("Errore nella scrittura di un codice di prenotazione");
                    serverSendError();
                    exit(EXIT_FAILURE);
                }

                movieTheatreGrid[currentRow][currentSeat] = '1';

                snprintf(userMessage, MAXLINE,
                         "Il posto fila %d, poltrona %d è stato prenotato. L'identificativo della prenotazione è: %llu\n",
                         currentRow, currentSeat, uniqueCode);
                fflush(stdout);
                if (!writeSocket(connectionSocket, userMessage, (int) strlen(userMessage))) { // non inserisco '\0'
                    perror("Errore nell'invio della risposta ad un client.");
                    return;
                }
                bzero((void *) &userMessage, sizeof(userMessage));

            } else {
                snprintf(userMessage, MAXLINE,
                         "Il posto fila %d, poltrona %d è già stato prenotato. Non è stato possibile effettuare la prenotazione\n",
                         currentRow, currentSeat);
                fflush(stdout);
                if (!writeSocket(connectionSocket, userMessage, (int) strlen(userMessage))) { // non inserisco '\0'
                    perror("Errore nell'invio della risposta ad un client.");
                    return;
                }
                bzero((void *) &userMessage, sizeof(userMessage));
            }
        } else {
            snprintf(userMessage, MAXLINE, "Il posto indicato non è valido.\n");
            fflush(stdout);
            if (!writeSocket(connectionSocket, userMessage, (int) strlen(userMessage))) { // non inserisco '\0'
                perror("Errore nell'invio della risposta ad un client.");
                return;
            }
            bzero((void *) &userMessage, sizeof(userMessage));
        }
        currentToken = strtok_r(NULL, " ", &stringContext);
    }

    // scrivo il terminatore '\0' per indicare la fine del messaggio del server
    if (!writeSocket(connectionSocket, &terminator, sizeof(char))) { // non inserisco '\0'
        perror("Errore nell'invio della risposta ad un client.");
        return;
    }

    close(fileDescriptor);
}
