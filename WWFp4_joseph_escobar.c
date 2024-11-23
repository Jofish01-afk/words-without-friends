#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>


#define PORT "8080" //default port
#define BACKLOG 10 //defines how long the "queue" of requests should be, used for listening
/*implement multthreading per request pthread_create, then pass function for thread to execute.
*/
//struct for holding the arguments of file reader since fileReader uses two arguments
struct thread_fileReader{
    int client_fd;
};

//create a global root and tail node variable for random word
struct wordListNode* wordRoot = NULL;
struct wordListNode* wordTail = NULL;

//Global root and tail node for findWords
struct gameListNode* gameRoot = NULL;
struct gameListNode* gameTail = NULL;

int wordCount = 0; //for debugging
int foundWords = 0; //used to store the number of words found
int totalGameWords = 0; //used to increment the number of nodes

//contain a 30 character string and pointer to next node
//linked list for holding the dictionary words
typedef struct wordListNode{
    char string[30];
    struct wordListNode *next;
}wordListNode;

wordListNode *newWordNode(char *data){
    wordListNode *result = malloc(sizeof(wordListNode)); //allocate space for a new node
    strcpy(result->string, data); //copies the string and assigns it to the node
    result->next = NULL; //the next node result points to is empty
    return result;
}

//contain 30 character string, boolean(has it been found?), and pointer to next node
//linked list for holding the words that can be made with the master word
typedef struct gameListNode{
    char string[30];
    bool found;
    struct gameListNode *next;
}gameListNode;

//creates a new node
gameListNode *newGameNode(char *data){
    gameListNode *result = malloc(sizeof(gameListNode)); //allocate space for a new node
    strcpy(result->string, data); //copies the string and assigns it to the node
    result->next = NULL; //the next node result points to is empty
    result->found = false; //determines if the word has been found
    return result;
}

//cleans up the word list (dictionary list)
void cleanupWordList(wordListNode *root){
    wordListNode *temp;
    //free each node by assigning each node to the head.
    while (root !=NULL){
        temp = root;
        root = root->next;
        free(temp);
    }
    printf("Word list terminated. \n");
}

//cleans up the list of found words
void cleanupGameList(gameListNode *root){
    gameListNode *temp;
    //free each node by assigning each node to the head.
    while (root !=NULL){
        temp = root;
        root = root->next;
        printf("Possible Word: %s\n", root->string);
        free(temp);
    }
    printf("Game list terminated. \n");
}


/* For now will initialize the random number generator.
It will return a number which is the count of words in the dictionary.*/
int initialization() {
    srand(time(NULL));

    FILE *fp;
    char reader[30];

    fp = fopen("2of12.txt", "r"); //open dictionary file
    if (fp == NULL) {
        printf("Failed to open \n");
    }

    while (fscanf(fp, "%s", reader) !=EOF){
        for (int i = 0; reader[i] != '\0'; i++){
            reader[i] = toupper(reader[i]);
        }
        wordListNode *result = newWordNode(reader); //asigns the string to the new node
        
        

        if (wordRoot == NULL){
            wordRoot = result;
           //printf("Added word: %s\n", reader);
        }
        else {
            wordTail->next = result;
            //printf("Added word: %s\n", reader);

        }
        wordTail = result;


        wordCount++; // Increment the word count        
    }

    fclose(fp);
    //printf("Word count: %d\n", wordCount); //debug
    return wordCount;
}


//prints the puzzle and flavor text for the game
int displayWorld(wordListNode *masterWordNode, int client_fd) {
    gameListNode *temp = gameRoot;
    //printf("Root of game list: %s\n", temp->string);
    //the contents of the gamelist keeps "starting" +1 word after each reset
    char buffer[100000]; //the buffer to be sent of the network

    //use snprintf to fill the buffer
    snprintf(buffer, sizeof(buffer),
    "<html>"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>Words Without Friends</title>"
    "<style>"
    "h1 {"
    "   font-family: 'Courier New', Courier, monospace;"
    "   font-size: 3em;"
    "   text-align: center;"
    "   color: rgb(51, 51, 51);"
    "   padding: 20px;"
    "   background: linear-gradient(45deg, rgb(0, 0, 0), rgb(255, 255, 255));"
    "   color: transparent;"
    "   -webkit-background-clip: text;"
    "   text-shadow: 3px 3px 5px rgba(0, 0, 0, 0.3);"
    "}"
    "p {"
    "   text-align: center;"
    "   font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;"
    "}"
    "a {"
    "   text-align: center;"
    "   display: block;"
    "   margin-top: 20px;"
    "   font-size: 1.2em;"
    "   color: #38a1db;"
    "   text-decoration: none;"
    "}"
    "a:hover {"
    "   text-decoration: underline;"
    "}"
    "body {"
    "   background-color: rgb(220, 234, 255);"
    "   font-family: Arial, sans-serif;"
    "}"
    ".word-item {"
    "   border: 2px solid rgb(75, 70, 74);"
    "   padding: 10px;"
    "   margin: 5px 0;"
    "   box-shadow: 10px 5px 5px black;"
    "   border-radius: 0.5em;"
    "   font-size: 1.2em;"
    "   height: 40px;"
    "   display: flex;"
    "   align-items: center;"
    "   justify-content: center;"
    "   width: auto;"
    "   max-width: 300px;"
    "   margin-left: auto;"
    "   margin-right: auto;"
    "}"
    ".not-found {"
    "   background-color: azure;"
    "}"
    ".found {"
    "   background-color: rgb(187, 250, 92);"
    "   font-weight: bold;"
    "}"
    " .word-container {"
    "   display: flex;"
    "   flex-wrap: wrap;"
    "   gap: 10px;"
    "   justify-content: center;"
    "   margin: 20px 0;"
    "}"
    "form {"
    "   text-align: center;"
    "}"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Words Without Friends</h1>\n"
    "<p>Your master word is: <strong>%s</strong></p>",masterWordNode->string
);



    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<a href=\"words\">New Game!</a>"); //figure out a way to reset the totalGamewords
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<div class=\"word-container\">");
    while(temp != NULL){
        //option++;
        //printf("Option %d: %s\n", option, temp->string);
    
        //iterate through the linked list, printing _ _ _ 

        //snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "content")
        if (temp->found == false) {
            
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<div class=\"word-item not-found\">");
            for(int i = 0; i < strlen(temp->string); i++){
                snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "_ ");
                //printf("_ "); //debug
            }
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "</div>"); //closing </div> is used once its finished with the word to keep it structured
            //printf("\n"); //dubug
        }
        else {//**Found Word!: WORD
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<div class=\"word-item found\">%s</div>\n", temp->string);
            
        }

        temp = temp->next;
    }
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "</div>");
    //snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "\n");
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<p>Your master word is: <strong>%s</strong></p>", masterWordNode->string);
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<p>Find all words to win!</p>");
    
    //snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "\n");
    
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "<form submit=\"words\"><input type=\"type\" name=move autofocus></input></form></body></html>");

    send(client_fd, buffer, strlen(buffer), 0); //send the buffer over the network
    return 0;

}
/*Accepts input from the user, removes carriage return and line feed, and
capitlize each letter in the char array*/
//will also now compare each entered line and mark true if the word is found
//if word is found, print the word.
//take url as a parameter
int acceptInput(char guess[], char *url) {
    gameListNode *temp = gameRoot;

    char *input = strstr(url, "move="); //taking the url and only caring about move=
    if (input) {
        //start after move=
        input += 5; 

        //copy the input into guess, which will then continue the normal logic
        strcpy(guess, input);

        //convert guess to all capital
        for (int i = 0; i < strlen(guess); i++) {
            guess[i] = toupper(guess[i]);
        }
    }

    printf("URL ENTERED: %s\n", url); //debug
    printf("MOVE ENTERED: %s\n", input); //debug
    //printf("Debug : %s\n", guess); //debug

    while(temp != NULL){    
        //iterate through the linked list, looking for a word that matches
        if (temp->found == true){
            if(strcmp(guess, temp->string) == 0){
                printf("Word was already found.\n");                
            }
           
        }
        else {
            if(strcmp(guess, temp->string) == 0){
                temp->found = true;
                printf("**Found word!**\n");
                foundWords++;
            }
        }

        temp = temp->next;
    }
}
/*Gets the letter distribution from the user input*/
int getLetterDistribution(char word[], int wordAlphabet[]) {
    //printf("ABCDEFGHIJKLMNOPQRSTUVWXYZ: INPUT \n"); //debug vizualization
    
    //sets array input to 0
    memset(wordAlphabet, 0, 26*sizeof(int));
    /*As long as an element in the array isn't empty, it will iterate through
    the loop and assign an ascii value to each part of the guessAlphabet. 'A' has an ascii value of 65,
    so it is used as a reference point to asign values, and keep track of how many times
    each letter appears in the user input. */
    for (int i = 0; word[i] != '\0'; i++){
        int index = toupper(word[i]) - 'A';
        wordAlphabet[index]++;
    }

    //printf("\n");
    return 0;
}


/*Compare the choice distribution and our guess distribution
return true if the word can be made from the choice distribution
*/
bool compareCounts(int choiceDistro[], int guessDistro[]) {
    for (int i = 0; i < 26; i++){
        if (choiceDistro[i] < guessDistro[i]) {
            //printf("Not a match \n");
            return false;
           
        }
    }
    return true;
    
}

//Determines if the game should end if the player has found 5 words
bool isDone() {
    
    if (foundWords >= totalGameWords){
        return true;
    }
    return false;

}


/*pick a random number from 0 to the count (~39000). Traverse to that index. Search linearly for a word that is long enough
(>6 letters) and return the wordListNode. Handle the case in which you read the end of the dictionary
without finding a long enough word*/
wordListNode* getRandomWord(){
    int randomIndex = rand() % (wordCount); //pick a random number, which will be the start of our search
    int currentIndex = 0;
    struct wordListNode *currentNode = wordRoot; //starts our current node at the root
    
    //traverses to the randomly selected node
    while (currentIndex < randomIndex){
        currentNode= currentNode->next;
        currentIndex++;
    }

    while(currentNode != NULL){
        if(strlen(currentNode->string) > 6){ 
            return currentNode; //returns the node containing the string
        }
        currentNode = currentNode->next;
    }

    
    //returns in the case that a long enough word wasnt found
    printf("Could not find a long enough word.\n");
    return NULL;
}


/*Go through the dictionary to check if the masterword can be made with any
of the words.*/
void findWords(wordListNode *masterWordNode) {
    int masterWordAlphabet[26] = {0}; //initialize to 0
    
    totalGameWords = 0; //initialize to 0

    int count = 0;

    //points to the beginning of the word linked list (dictionary) to read through the list
    wordListNode *gameNode = wordRoot;


    //printf("Master word: %s\n", masterWordNode->string);
    getLetterDistribution(masterWordNode->string, masterWordAlphabet);    
    

    while(gameNode != NULL){ //goes through entire wordList
        int dictionaryAlphabet[26] = {0}; //initialize it to zero every time

        getLetterDistribution(gameNode->string, dictionaryAlphabet); //get letter distribution for each word in the dictionary
        
        if(compareCounts(masterWordAlphabet, dictionaryAlphabet)){ //compare master word distro with dictionary word distro, if return true, continue

            gameListNode* newNode = newGameNode(gameNode->string); //allocates space for the new node

            if (gameRoot == NULL){ //add to head if empty
                //count++;
                //printf("Head is empty, added #%d: %s\n", count, gameNode->string);
                gameRoot = newNode;
            }
            else { //add to tail if empty
                //count++;
                //printf("Added to tail #%d: %s\n", count, gameNode->string);
                gameTail->next= newNode;
            }
            gameTail = newNode; 
            totalGameWords++; //for debugging
        }


        gameNode = gameNode->next;
    }
    printf("Total number of game words is: %d\n", totalGameWords); //debugg
    
}

//debug vizualiztion for seeing the contents of the list 
void printGameList() {
    gameListNode *temp = gameRoot; 
    int index = 0; 
    
    while (temp != NULL) {
        printf("Node #%d: %s\n", index, temp->string); 
        temp = temp->next; 
        index++; 
    }
}


//prints to inform user the game is over and frees linked lists
int teardown() {
    cleanupWordList(wordRoot);
    cleanupGameList(gameRoot);
    printf("All done!\n");
} 

// int gameLoop() { //dont really need the loop but still need the logic in the loop
//     char guess[15]; //the char array for the user input
//     int guessAlphabet[26] = {0}; //letter distribution for the user input
//     //printf("Word found by loop: %s\n", masterWordNode->string);
//     wordListNode *masterWordNode = getRandomWord();
//     if (masterWordNode != NULL){
//         findWords(masterWordNode); 
//         do {
//             displayWorld(masterWordNode);  
//             memset(guessAlphabet, 0, sizeof(guessAlphabet)); //reset the guess distribution of the array to 0 everytime it is called
//             acceptInput(guess);  
            
//         } while (!isDone());
//     } 
//     else{ //in the case that a long enough word couldn't be found, it will exit the gameLoop()
//         printf("Insufficient word found: \n");
//     }

// }

/*open file, read the file, send contents to client after accepting*/
//byte array maybe
//HTTP/1.0 200 OK
//HTTP/1.0 404 Not Found
/*display word and accept input logic gets placed here*/
void *fileReader(void* arg){ //dont need file reading anymore so get rid of directory stuff
    struct thread_fileReader* reader = (struct thread_fileReader*)arg;

    //assigns the arguments of the structure to the variables
    int client_fd =reader->client_fd; 
    //printf("Thread received: %d, %s\n", client_fd, directoryPath); //debugging

    char buffer[50000]; //buffer for sending data
    char request[2000]; //http requst from client
    int bytes_received = recv(client_fd, request, sizeof(request), 0);
    
    request[bytes_received] = '\0'; 

    /*want to generate a new game if there is no input in the box
    if there is an input, then continue the game.*/
    
    char url[256]; //holds the url

    printGameList(); 
    //RESET TOTAL GAME WORDS AND TOTAL WORDS FOUND, DOESNT RESET WHEN CLICKING NEW GAME

    static wordListNode *masterWordNode = NULL;
    sscanf(request, "GET %s HTTP/1.1", url); //gets the url

    if (strncmp(url, "/words", 6) == 0) { //checks to see if url has "/words"
        if (strstr(url, "?move=")) { //now we check if it also contains "?move=" which is out input, which will be taken
            //process a move
            char guess[15];
            acceptInput(guess, url); //accept input from the url

            if (!isDone()) {
                //main "gameloop"
                memset(buffer, 0, sizeof(buffer)); //reset the buffer before sending it out again, otherwise it will continue to append
                snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n");
                send(client_fd, buffer, strlen(buffer), 0);
                displayWorld(masterWordNode, client_fd);//print the board
                printf("%s\n", guess); //debug

            } else {//end of game once all words are found
                memset(buffer, 0, sizeof(buffer));
                snprintf(buffer, sizeof(buffer), 
                "HTTP/1.1 200 OK\r\n"
                "content-type: text/html; charset=UTF-8\r\n\r\n"
                "<html>"
                "<head>"
                "<style>"
                "h1 {"
                "    font-family: 'Courier New', Courier, monospace;"
                "    font-size: 3em;"
                "    text-align: center;"
                "    color: rgb(51, 51, 51);"
                "    padding: 20px;"
                "    background: linear-gradient(45deg, rgb(38, 165, 27), rgb(0, 0, 0));"
                "    color: transparent;"
                "    -webkit-background-clip: text;"
                "    text-shadow: 3px 3px 5px rgba(0, 0, 0, 0.3);"
                "}"
                "a {"
                "    text-align: center;"
                "    display: block;"
                "    margin-top: 20px;"
                "    font-size: 1.2em;"
                "    color: #38a1db;"
                "    text-decoration: none;"
                "}"
                "a:hover {"
                "    text-decoration: underline;"
                "}"
                "body {"
                "   background-color: rgb(233, 124, 180);"
                "</style>"
                "</head>"
                "<body>"
                "<h1>Congratulations! You solved it!</h1>"
                "<a href=\"words\">Click here to play again!</a>"
                "</body>"
                "</html>");
                send(client_fd, buffer, strlen(buffer), 0);

                //reset game
                cleanupGameList(gameRoot);
                gameRoot =NULL;
                totalGameWords = 0; //reinitialize them to zero
                foundWords = 0;
                masterWordNode = getRandomWord();
                findWords(masterWordNode);
                
            }
        } else { //the case in which no input is provided. should handle a new game case
            //resets by clearing old list, and sets up a new board and list
            cleanupGameList(gameRoot); 
            gameRoot =NULL;
            // printf("New Game Started...\n");
            // printf("Before resetting: %d\n", totalGameWords);
            // printf("Before setting found words: %d\n", foundWords);
            totalGameWords = 0; //reinitialize them to zero
            foundWords = 0;
            // printf("Cleared game list: %d\n", totalGameWords);
            // printf("Cleared found words: %d\n", foundWords);

            masterWordNode = getRandomWord();
            findWords(masterWordNode);

            //reset buffer
            memset(buffer, 0, sizeof(buffer));
            snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n");
            send(client_fd, buffer, strlen(buffer), 0);

            displayWorld(masterWordNode, client_fd); //redisplay
        }
    } else {
        //404 if url is not entered properly
        char *message = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
        send(client_fd, message, strlen(message), 0);
    }
    close(client_fd);
    free(arg); 
    pthread_exit(NULL);
    
}

int main(int argc, char *argv[]){

    printf("Running web server on port %s...\n", PORT);
    printf("Play at: localhost:%s/words\n", PORT);
    initialization();
    /*used from slide 59 of networking*/
    int sockfd, new_fd, yes=1, rv;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

    memset(&hints, 0, sizeof hints); // Setup hints
    hints.ai_family = AF_UNSPEC; //ip family so for both
    hints.ai_socktype = SOCK_STREAM; //tcp 
    hints.ai_flags = AI_PASSIVE; // use my IP, server hosting on local machine

    //populates servinfo
    rv = getaddrinfo(NULL, PORT, &hints, &servinfo); // Get my available interfaces

    for(p = servinfo; p != NULL; p = p->ai_next) { // loop through all the results and bind ASAP
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { continue; }//finds socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { close(sockfd); continue; } //tries to bind to socket
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    if (listen(sockfd, BACKLOG) == -1) { 
        printf("Failed to listen. Wait ≈30 seconds for the program to reset.\n");
        exit(1); 
    }

    printf("Waiting for response...\n");
    while(1) {  //main accept() loop, continues to wait for new connections
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); // NOTE THE NEW FD
            
        printf("Accepted Response from Client\n");

        //while constantly waiting for connections, the readFile function is threaded to handle multiple clients
        pthread_t ptid; //new thread id
        struct thread_fileReader *reader = malloc(sizeof(struct thread_fileReader)); //creates thread struct and malloc and pass the arguments
        reader->client_fd = new_fd; 

        if (pthread_create(&ptid, NULL, fileReader, reader) != 0){
            printf("Error creating thread\n");
            free(reader);
            close(new_fd);
            exit(1);
        }
        else{
            pthread_detach(ptid); //reclaim ptid when thread is done
        }
    }
    close(new_fd);   
    return 0;
    
}