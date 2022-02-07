void serverUndoSeatsBooking(char *bookingCodes, char *fileName) {
    printf("serverUndoSeatsBooking richiesto.\n");
    FILE *fileStream;
    char buffer[MAXLINE] = {0}, rowChar[MAXLINE] = {0}, seatChar[MAXLINE] = {0};
    int row, seat;
    char userMessage[MAXLINE];
    char *currentCode, *codeContext, terminator = '\0';

    if (checkArgumentsExistance(bookingCodes, connectionSocket) == -1)
        return;

    char *filePath = malloc(strlen(getenv("HOME")) + strlen(directoryName) + strlen(fileName) + 1);
    if (filePath == NULL) {
        perror("Allocazione della memoria per il path del file dei codici fallita");
        serverSendError();
        return;
    }

    strcpy(filePath, getenv("HOME"));
    strcat(filePath, directoryName);
    strcat(filePath, fileName);

    retry_file_fopen:
    if ((fileStream = fopen(filePath, "r+")) == NULL) {
        if (errno == EINTR)
            goto retry_file_fopen;
        else {
            serverSendError();
            perror("Apertura del file dei codici fallita.");
            return;
        }
    }

    free(filePath);
    currentCode = strtok_r(bookingCodes, " ", &codeContext);

    while (currentCode != NULL) {
        if (fseek(fileStream, 0, SEEK_SET) == -1) {
            perror("Errore nel riposizionamento dell'indice nel file dei codici");
            return;
        }

        while (1) { // scorro l'intero file in cerca del codice richiesto

            bzero((void *) &buffer, sizeof(buffer));
            bzero((void *) &rowChar, sizeof(rowChar));
            bzero((void *) &seatChar, sizeof(seatChar));

            if (readNextWord(fileStream, buffer))
                break;

            if (readNextWord(fileStream, rowChar))
                break;

            if (readNextWord(fileStream, seatChar)) {
                break;
            }

            if (strstr(buffer, currentCode) != NULL) {

                if ((row = convertStringToNaturalInt(rowChar)) == -1) {
                    perror("Errore nella lettura del numero di fila dal file dei codici");
                    return;
                }

                if ((seat = convertStringToNaturalInt(seatChar)) == -1) {
                    perror("Errore nella lettura del numero di posto dal file dei codici");
                    return;
                }

                movieTheatreGrid[row][seat] = '0';

                deletePreviousLine(fileStream);

                bzero((void *) &userMessage, sizeof(userMessage));
                snprintf(userMessage, MAXLINE, "La prenotazione con il codice %s è stata annullata\n", currentCode);
                if (!writeSocket(connectionSocket, userMessage,
                                 (int) strlen(userMessage))) { // non inserisco il terminatore '\0'
                    perror("Errore nell'invio della risposta ad un client.");
                    return;
                }
                break;
            }
        }

        currentCode = strtok_r(NULL, " ", &codeContext);
    }

    // invio il carattere di terminazione
    if (!writeSocket(connectionSocket, &terminator, sizeof(char))) { // non inserisco il terminatore '\0'
        perror("Errore nell'invio della risposta ad un client.");
        return;
    }

    fclose(fileStream);
}