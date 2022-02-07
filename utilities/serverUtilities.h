char *directoryName = "/progettoSistemiOperativiData";
char *codesName = "/bookingCodes";

unsigned long long currentUniqueBookingCode = 0;
char **movieTheatreGrid = NULL;
int rowsNumber = 0, seatsNumber = 0;
int connectionSocket = -1, listeningSocket = -1;

void parseServerArguments(int argc, char **argv, int *port) {

    if (argc != 4) {
        printf("Utilizzo: porta rowsNumber seatsNumber\n");
        exit(EXIT_FAILURE);
    } else {

        if ((*port = convertStringToNaturalInt(argv[1])) <= 0) {
            printf("Errore acquisizione numero di porta. Il server verrà terminato.\n");
            exit(EXIT_FAILURE);
        }

        if ((rowsNumber = convertStringToNaturalInt(argv[2])) <= 0) {
            printf("Errore acquisizione numero di file. Il server verrà terminato.\n");
            exit(EXIT_FAILURE);
        }

        if ((seatsNumber = convertStringToNaturalInt(argv[3])) == -1) {
            printf("Errore acquisizione numero di posti per fila. Il server verrà terminato.\n");
            exit(EXIT_FAILURE);
        }
    }
}

char **createMovieTheatreGrid() {

    char **grid = malloc(rowsNumber * sizeof(char *));

    if (grid == NULL) {
        perror("Errore nella creazione della griglia per i posti in sala.");
        exit(EXIT_FAILURE);
    }

    for (int row = 0; row < rowsNumber; row++) {
        if ((grid[row] = malloc(seatsNumber * sizeof(char))) == NULL) {
            perror("Errore nella creazione di un posto in sala");
            exit(EXIT_FAILURE);
        }
    }

    for (int row = 0; row < rowsNumber; row++) {
        for (int seat = 0; seat < seatsNumber; seat++) {
            grid[row][seat] = '0';
        }
    }

    return grid;
}

unsigned long long generateUniqueBookingCode() {
    currentUniqueBookingCode++;
    return currentUniqueBookingCode;
}

int readNextWord(FILE *fileStream, char *wordPointer) {
    char characterRead;
    char *buffer;

    buffer = wordPointer;

    while (fread(&characterRead, 1, 1, fileStream)) {
        *buffer++ = characterRead;
        if (characterRead == ' ' || characterRead == '\n')
            break;
    }

    if (feof(fileStream)) { // raggiunto EOF
        return 1;
    }

    *buffer = '\0';
    return 0;
}

void deletePreviousLine(FILE *fileStream) {
    char characterRead;

    if (fseek(fileStream, -2, SEEK_CUR) == -1) {
        perror("Errore nel riposizionamento dell'indice del file");
        exit(EXIT_FAILURE);
    }

    while (fread(&characterRead, 1, 1, fileStream) == 0) {
        perror("Errore nella lettura da file");
    }

    while (characterRead != '\n') {
        if (fseek(fileStream, -1, SEEK_CUR) == -1) {
            perror("Errore nel riposizionamento dell'indice del file");
            exit(EXIT_FAILURE);
        }

        if (characterRead != ' ') {
            if (fputc('*', fileStream) == EOF) {
                perror("Errore nella scrittura del file");
                exit(EXIT_FAILURE);
            }
        } else {
            if (fseek(fileStream, +1, SEEK_CUR) == -1) {
                perror("Errore nel riposizionamento dell'indice del file");
                exit(EXIT_FAILURE);
            }
        }

        if ((ftell(fileStream) == 1))
            break;

        if (fseek(fileStream, -2, SEEK_CUR) == -1) {
            perror("Errore nel riposizionamento dell'indice del file");
            exit(EXIT_FAILURE);
        }

        while (fread(&characterRead, 1, 1, fileStream) == 0) {
            perror("Errore nella lettura da file");
        }
    }
}

void createProgramDirectory() {
    int creationResult;

    char directoryPath[MAXLINE] = {0};
    strcpy(directoryPath, getenv("HOME"));
    strcat(directoryPath, directoryName);
    creationResult = mkdir(directoryPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (creationResult == -1) {
        if (errno == EEXIST)
            return;
        else {
            printf("Non è stato possibile creare la cartella %s. Il server verrà terminato.\n", directoryName);
            exit(EXIT_FAILURE);
        }
    }
}

void emptyFile(char *fileName) {
    char *file;

    if ((file = malloc(strlen(getenv("HOME")) + strlen(directoryName) + strlen(fileName) + 1)) == NULL) {
        perror("Non è stato possibile allocare spezio per il nome del file bookingCodes. Il server verrà terminato");
        exit(EXIT_FAILURE);
    }

    strcpy(file, getenv("HOME"));
    strcat(file, directoryName);
    strcat(file, fileName);

    if (access(file, F_OK) == 0) {
        retry_file_delete:
        if (remove(file) == -1) {
            if (errno == EINTR)
                goto retry_file_delete;
            else {
                perror("Impossibile eliminare il file bookingCodes già esistente. Il server verrà terminato.");
                exit(EXIT_FAILURE);
            }
        }
    }

    free(file);
}

int openFile(char *fileName) {
    int creationResult;
    char *file;

    if ((file = malloc(strlen(getenv("HOME")) + strlen(directoryName) + strlen(fileName) + 1)) == NULL)
        return -1;

    strcpy(file, getenv("HOME"));
    strcat(file, directoryName);
    strcat(file, fileName);

    retry_file_open:
    if ((creationResult = open(file, O_RDWR | O_CREAT | O_APPEND, 0777)) == -1) {
        if (errno == EINTR)
            goto retry_file_open;
        else
            return -1;
    }

    free(file);
    return creationResult;
}

int checkArgumentsExistance(const char *arguments, int socketDescriptor) {
    char errorMessage[MAXLINE] = {0};

    if (arguments == NULL) {
        snprintf(errorMessage, MAXLINE, "Errore nella lettura degli argomenti");
        if (!writeSocket(socketDescriptor, errorMessage, (int) strlen(errorMessage)))
            perror("Errore nell'invio del messaggio d'errore ad un client.");
        return -1;
    }

    return 0;
}

void serverCloseGracefully(int signo) {
    char file[MAXLINE] = {0};

    closeSocket(listeningSocket);

    for (int row = 0; row < rowsNumber; row++) {
        free(movieTheatreGrid[row]);
    }

    free(movieTheatreGrid);

    strcpy(file, getenv("HOME"));
    strcat(file, directoryName);
    strcat(file, codesName);

    remove(file);
    exit(EXIT_SUCCESS);
}

void serverSetSignals() {
    struct sigaction act;
    act.sa_handler = serverCloseGracefully;

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

void serverInitializeListeningSocket(int port) {
    int on = 1;
    struct sockaddr_in serverAddress;

    // crea un socket TCP d'ascolto
    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("errore nella creazione del socket d'ascolto");
        exit(EXIT_FAILURE);
    }

    // consente il riutilizzo dell'indirizzo con cui si farà il bind
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        perror("Errore nell'impostazione delle opzioni del socket");
        exit(EXIT_FAILURE);
    }

    // inizializzo sockaddr_in
    bzero((void *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(
            INADDR_ANY); // il server accetta dati su una qualunque delle sue interfacce di rete
    serverAddress.sin_port = htons(port); // numero di porta del server per prime comunicazioni

    // assegna l'indirizzo al socket
    if (bind(listeningSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Errore in bind");
        closeSocket(listeningSocket);
        exit(EXIT_FAILURE);
    }

    // mettiamo il server in ascolto sulla porta
    if (listen(listeningSocket, BACKLOG) == -1) {
        perror("Errore in listen");
        closeSocket(listeningSocket);
        exit(EXIT_FAILURE);
    }
}