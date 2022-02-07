int socketDescriptor = -1;

void parseClientArguments(int argc, char **argv, int *port) {

    if (argc != 3) {
        printf("Arguments: port address\n");
        exit(EXIT_FAILURE);
    } else {
        *port = convertStringToNaturalInt(argv[1]);

        if (*port <= 0) {
            printf("Valore della porta non valido. Il client verrà terminato");
            exit(EXIT_FAILURE);
        }

    }
}

void clientInitializeSocket(int port, char *addressString) {
    struct sockaddr_in serverAddress;

    // crea il socket
    if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Creazione del socket fallita.");
        exit(EXIT_FAILURE);
    }

    // inizializza serverAddress
    bzero((void *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // assegna il tipo di indirizzo
    serverAddress.sin_port = htons(port); // assegna la porta del server

    // imposta indirizzo IP del server
    if (inet_pton(AF_INET, addressString, &serverAddress.sin_addr) <= 0) {
        perror("Conversione dell'indirizzo del server fallita.");
        exit(EXIT_FAILURE);
    }

    // connessione al server
    retry_connect:
    if (connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        if (errno == EINTR)
            goto retry_connect;
        else {
            perror("connect fallita.");
            exit(EXIT_FAILURE);
        }
    }
}

void clientCloseGracefully(int signo) {
    if (socketDescriptor != -1)
        close(socketDescriptor);
    exit(EXIT_SUCCESS);
}

void clientSetSignals() {
    struct sigaction act;
    act.sa_handler = clientCloseGracefully;

    if (sigfillset(&act.sa_mask) == -1) {
        perror("errore nel riempimento di .sa_mask.");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGINT, &act, 0) == -1) {
        perror("errore nella gestione di SIGINT");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTERM, &act, 0) == -1) {
        perror("errore nella gestione di SIGTERM");
        exit(EXIT_FAILURE);
    }
}