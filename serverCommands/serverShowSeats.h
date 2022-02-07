void serverShowSeats() {
    printf("serverShowSeats richiesto.\n");
    int charSize = sizeof(char);
    char newline = '\n';
    char terminator = '\0';

    // invia la griglia dei posti al client
    for (int row = 0; row < rowsNumber; row++) {
        for (int seat = 0; seat < seatsNumber; seat++) {

            writeSocket(connectionSocket, &movieTheatreGrid[row][seat], charSize);

            if (seat == seatsNumber - 1)
                writeSocket(connectionSocket, &newline, charSize);
        }
    }
    writeSocket(connectionSocket, &terminator, charSize);
}
