//Michael Nwachi Spring 2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define PORT "8000"
#define BACKLOG 10
#define BUFFER_SIZE 1024
#define ROOT "/Users/michaelnwachi/Desktop/ArchLinuxShare/myNewDirectory" 
#define MAX_QUOTE_LENGTH 1024

char *Globalpuzzle = NULL;

/*
This is the struct of the quote as the quote 
                             as an author and phrase
*/
typedef struct Quote{
    char *phrase;
    char *author;
    struct Quote *next;
}Quote;

Quote *head = NULL;
int count = 0;

/*
This method is used to create the quote and set the pointers to null
*/
Quote *createQuote(){

    struct Quote *newQuote = ( Quote *)malloc(sizeof( Quote));
    if(newQuote == NULL){
        printf("error new quote is null");
        exit(EXIT_FAILURE);
    }
    newQuote->phrase = NULL;
    newQuote->author = NULL;
    newQuote->next = NULL;

    return newQuote;
}

/*
This method is used to append the new quote to the end of the linkedlist
*/
void appendQ(Quote *newQuote){
    if(head == NULL){
       head = newQuote;
    }else{
       Quote *temp = head;
        while(temp ->next != NULL){
            temp = temp->next;
        }
        temp->next = newQuote;
    }
}

/*
This method is used to load the quotes from the file 
                into memory to be used 
*/
void loadPuzzles(){
    FILE *file = fopen("quotes.txt", "r");
    if(file == NULL){
        printf("error file is NULL");
        exit(EXIT_FAILURE);
    }

    Quote *current = NULL;
    char line[MAX_QUOTE_LENGTH];
    while(fgets(line,MAX_QUOTE_LENGTH,file)!=NULL){
        if(strlen(line) < 3){
            if(current != NULL){
                appendQ(current);
                count++;
                current = NULL;
            }
           Quote *newQuote = createQuote();
           if(head == NULL){
            head = current;
           }else if(strncmp(line, "--", 2) == 0){
            if(current == NULL){
                printf("error current is Null");
                exit(EXIT_FAILURE);
            }
            current ->author = strdup(line+3);
           }
        }else{
            if(current == NULL){
                current = createQuote();
            }
            if(current->phrase == NULL){
                 current->phrase = strdup(line);
            }else{
                 current->phrase = realloc(current->phrase, strlen(current->phrase) + strlen(line) + 1);
                strcat(current->phrase, line);
            }
        }
    }
    if(current != NULL){
        appendQ(current);
        count++;
    }
    fclose(file);
}

typedef struct gameState{
    char orderedKeys[26];
    char inputKeys[26];
}gameState;

/*
This method is used to implement the Fisher-Yates algorithm and randomize the encrypted string
            then implement the idea of replacing letters within the long length of the randomize letters
*/
void shuffle(char key[]){
    srand((unsigned int)time(NULL));
    for(int i = 25; i >= 0; i--){
        int j = random() % (i+1);
            char temp = key[i];
            key[i] = key[j];
            key[j] = temp;
        
    }
}

/*
This method only returns the global variable when it is called
*/
const char* getPuzzle(){
    if(count == 0){
        loadPuzzles();
    }
    int randomIndex = random() % count;
    Quote *currentQuote = head;
    for(int i = 0; i < randomIndex; i++ ){
        currentQuote = currentQuote->next;
    }
   return currentQuote->phrase;
}

/*
This method frees the quote's memory after use
*/
void freeQ(){
    Quote *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->phrase);
        free(temp->author);
        free(temp);
    }
}

/*
This method makes sure that the global variable isn't null then adds the 
            the implementation of two arrays that are that are randomized letters
                                and letters that the user inputted to replace another letter
*/
void initialization(gameState *state){
    srand((unsigned int)time(NULL));
    const char *puzzle = getPuzzle();
    if(puzzle != NULL){
       Globalpuzzle = strdup(puzzle);
       if(Globalpuzzle == NULL){
        printf("error the global puzzle is null");
        exit(EXIT_FAILURE);
       }
    }
    for(int i = 0; i <26; i++){
       state->orderedKeys[i] = 'A'+i;
    }
    shuffle(state->orderedKeys);

    for(int i = 0; i<26; i++){
        state->inputKeys[i] = '\0';
    }
}

/*
This method is used to get the users input and makes sure that the user input length is less than 3
        and probably quits if user enters quit
*/
void acceptInput(gameState *state){
    char input[100];
    printf("Enter 2 letters of choice:\n ");
    if(fgets(input,sizeof(input), stdin) !=NULL){
        input[strcspn(input, "\n")] = '\0';
        if(strcmp(input, "quit") == 0){
         printf("User has quit the program\n");
         exit(EXIT_SUCCESS);
        }else if(strlen(input) == 2 && isalpha(input[0]) && isalpha(input[1])){
            printf("%s\n ", Globalpuzzle);
             state->inputKeys[input[1]-'A'] = input[0];
        }
    }else{
        printf("Error not a letter/invalid input\n");
        exit(EXIT_FAILURE);
    }
}


bool isGameOver(const gameState* state){
    bool Decrypted = true;
    for(int i = 0; i < sizeof(state->orderedKeys); ++i){
        char encryptedChar = state->orderedKeys[i];
        if(encryptedChar >= 'A' && encryptedChar <= 'Z'){
            int index = encryptedChar - 'A';
            if(state->inputKeys[index] == '\0'){
                Decrypted = false;
                break;
            }
        }
    }
    return Decrypted;
}

/*
This method populates the user input with variables called distance and replacement 
        as replacement variable will replace distance and implements this as well and prints it out
*/
bool updateState(gameState *state, const char* input){
   if(input == NULL || strcmp(input, "quit\n") == 0){
    return true;
}else if(strlen(input) == 2){
    char distance = input[0];
    char replacement = input[1];
    printf("Decrypted State: ");
    if(isalpha(distance) && isalpha(replacement)){
    for(int i = 0; i < strlen(state->orderedKeys); ++i){
        char encryptedChar = state->orderedKeys[i];
        if(encryptedChar >= 'A' && encryptedChar <= 'Z'){
            int index = encryptedChar - 'A';
            if(state->inputKeys[index] == '\0'){
                printf("_");
            }else{
                printf("%c", state->inputKeys[index]);
            }
        }else{
            printf("%c", encryptedChar);
            }
        }
    }
    return false;
} 
else{
    printf("Error: not enough or too many characters in input");
    return false;
    }
}

/*
This method prints out the global variable, the encrypted array of randomized letters and 
        the decrypted array of letters based off user input
*/
bool displayWorld(const gameState* state){
    int client_fd;
    char start[BUFFER_SIZE];
    sprintf(start, "<html><body>Encrypted State: %s<br>", state->orderedKeys);
    char startText[] = "<form action=\"crypt\" method=\"GET\">"
                "<input type=\"text\" name=\"move\" autofocus maxlength=\"2\"></input>"
                 "</form>"   
                 "<p>Decrypted State:<br>";
    bool Decrypted = true;
    for(int i = 0; i < strlen(state->orderedKeys); ++i){
        char encryptedChar = state->orderedKeys[i];
        if(encryptedChar >= 'A' && encryptedChar <= 'Z'){
            int index = encryptedChar - 'A';
            if(state->inputKeys[index] == '\0'){
                sprintf(start,"%s",startText,"_");
                Decrypted = false;
            }else{
                sprintf(start, "%s", startText,"%c", state->inputKeys[index], '\0');
            }
        }else{
            sprintf(start,"%s",startText,"%c", encryptedChar, '\0');
        }
    }
    sprintf(start,"%s",startText,"\n");
    send(client_fd,start, strlen(start), 0);
    send(client_fd,startText,strlen(startText),0);
    return isGameOver(state);
}

/*
This method is used to receive data for the game from client, and process the data to
            where it could a response back to with an appropiate response
*/
void handleGame(int client_fd){
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;
        gameState state;

     bytesRead = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytesRead < 0) {
        perror("recv error");
        close(client_fd);
        pthread_exit(NULL);
    }

    if(strncmp(buffer, "GET /crypt", 10) == 0){
        char *move = strstr(buffer, "?move=");

        if(move == NULL){
            const char *newPuzzle = getPuzzle();
            if(newPuzzle != NULL){
                char *responsePuzzle = "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n";
                send(client_fd, responsePuzzle, strlen(responsePuzzle), 0);
                displayWorld(&state);
            }
        }else{
            char moveCh[3] = {0};
            strncpy(moveCh, move + 6, 2);
            bool over = isGameOver(&state);
            if(!over){
                displayWorld(&state);
            }else{
                char* response = "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n"
                "<html><body>Congratulations! You solved it! <a href=\"crypto\">Another?</a></body></html>";
                send(client_fd, response, strlen(response), 0);
            }
        }
    }
}

/*
This method is used to show the user that the game is over 
                                    and frees the memory of the global variable
*/
void tearDown(int client_fd){
    char endMessage[] = "ALL DONE";
    send(client_fd,endMessage, strlen(endMessage), 0);
    if(Globalpuzzle != NULL){
         free(Globalpuzzle);
         Globalpuzzle = NULL;
    }
    freeQ();
}

/*
This method handles the request, process it and sends back to appropiate response
*/
void *handleRequest(void *arg) {
    int client_fd = *((int *)arg);
    free(arg);
    int error = chdir("/Users/michaelnwachi/Desktop/ArchLinuxShare/myNewDirectory");
    if(error < 0){
        perror("error chdir");
            exit(EXIT_FAILURE);
   }
   char buffer[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];

    //Reads the request from the client
    read(client_fd, buffer, sizeof(buffer));
    char *saveptr;
    char *token = strtok_r(buffer, " ", &saveptr); 
    token = strtok_r(NULL, " ", &saveptr);
    
    /*
    Opens the file and sees if the file is there or not
                if not it returns 404, if it is it returns 200
    */
   strcpy(filePath, token);
    int file = open(filePath, O_RDONLY);
    printf("File open");
    if(file == -1){
        char response[] = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        pthread_exit(NULL);
    }
    char response[] = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, response, strlen(response), 0);

    ssize_t fileContent;

    //Sends all the file content to the client from the request
    while(fileContent = read(file, buffer, sizeof(buffer))){
            send(client_fd, buffer, fileContent, 0);
        }
        printf("file added content");
    
   close(file);
   close(client_fd);
   pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_socket;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage client_addr; 
    socklen_t socket_size;
    int getInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //Gets the information for the source of the server
    if ((getInfo = getaddrinfo("http://localhost:8000/hello.html", PORT, &hints, &servinfo)) != 0) {
        perror("get address error");
        exit(EXIT_FAILURE);
    }

    // Loops through the server and waits to bind with and listening for an accept from a client
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_socket);
            perror("bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); 

    if (listen(server_socket, BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    loadPuzzles();

    printf("connecting to the client \n");
     printf("PORT: %d\n", PORT);

    //While true handles to the client's accept and response for the server
    while (1) {
        socket_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &socket_size);
        if (client_socket == -1) {
            perror("failed to accepted after allocated memory");
            continue;
        }
        printf("accepted accept");

        pthread_t thread_id;
        int *client_fd = malloc(sizeof(int));
        *client_fd = client_socket;


        if(client_fd == NULL){
            perror("memory allocated failed");
            close(client_socket);
            continue;
        }
         //Creates the thread used to handle the client's request
        if (pthread_create(&thread_id, NULL, handleRequest, client_fd) != 0) {
            perror("error pthread");
            close(client_socket);
        }
    
    }
    close(server_socket);
    return 0;
}
