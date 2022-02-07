void serverSendError() {
    char error[] = "Si è verificato un errore. Il server non ha potuto risolvere la richiesta.\n";
    writeSocket(connectionSocket, error, (int) strlen(error) + 1);
}
