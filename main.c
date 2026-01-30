#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

typedef struct WordNode_struct {
    char* word;
    struct WordNode_struct* next;
    struct WordNode_struct* prev;
} WordNode;

typedef struct LadderNode_struct {
    struct LadderNode_struct* next;
    struct WordNode_struct* wordList;
    int* indexList;
} LadderNode;

char consonants[22] = "BCDFGHJKLMNPQRSTVWXYZ";
char vowels[6] = "AEIOU";
int MAX_LEN = 0;
int MIN_LEN = 0;

bool setSettings(int argc, char* argv[], char filename[100], bool* manual, bool* playMode, bool* solverMode, bool* gamemode, bool* seeded);
void setGamemode(bool gamemode, char** gameConsonants, char** gameVowels, char** gameLetters, bool manual);
void selectLetters(int* consonantLimit, int* vowelLimit, char** gameConsonants, char** gameVowels, char** gameLetters, bool manual);
int countMinLengthWords(char* filename, int minLength, int* maxLength);
void buildSides(char* gameLetters, char** side1, char** side2, char** side3, char** side4);
bool buildWordArray(char** words, char* filename, int minLength, int numWords);
bool isValidAdjacency(char letter1, char letter2, char* side1, char* side2, char* side3, char* side4);
int checkUsedLetter(WordNode* userWords, int currentWordNum, char letter);
bool solveBest(char* gameLetters, char** words, int numWords, char* side1, char* side2, char* side3, char* side4);
void displayBox(char* gameLetters, int* usedLetters);
void appendWord(char*** pWords, int* pNumWords, int* pMaxWords, char* newWord);
void appendWordNode(WordNode*** pWordNodes, int* pNumWordNodes, int* pMaxWordNodes, WordNode** newWordNode);
void findTwoCombos(bool usedLetters[12], char* gameLetters, char** solvedWords, int numWords, WordNode** newNode, WordNode*** pWordNodes, int* pNumWordNodes, int* pMaxWordNodes);
bool findThreeCombos(bool usedLetters[12], char* gameLetters, char** solvedWords, int idx, int numSolved, char* firstSolved, char lastLetter, WordNode** newNode, bool allLettersFound);
int getLadderLength(WordNode* ladder);
LadderNode* popLadder(LadderNode** ladderList);
void insertLadderAtBack(LadderNode** list, WordNode* newLadder);
WordNode* copyLadder(WordNode* ladder);
LadderNode* findNumberedSolution(char** solvedWords, char* gameLetters, int solvedLength, int reqNum);
WordNode* popLadderFromFront(LadderNode** list);


int main(int argc, char* argv[])
{
    //if gamemode true, it is easy; if false, it is hard
    bool gamemode = true;
    bool manual = false;
    bool solverMode = false;
    bool playMode = false;
    bool seeded = false;
    char filename[100];


    //DR R NOTE: RNG should be seeded with srand

    //if the argc is right and argv has -e, gamemode is true; if it has -h, gamemode is false
    if(!setSettings(argc, argv, filename, &manual, &playMode, &solverMode, &gamemode, &seeded)) {
        printf("Incorrect settings entered.");
        return 0;
    }

    if(playMode) {
        printf("playMode = ON\n");
    } else {
        printf("playMode = OFF\n");
    }

    if(solverMode) {
        printf("solverMode = ON\n");
    } else {
        printf("solverMode = OFF\n");
    }

    if(gamemode) {
        printf("gamemode = easy mode\n");
    } else {
        printf("gamemode = hard mode\n");
    }

    //set gamemode using setGamemode and use correct parameter (gamemode)
    //setGamemode will also call selectLetters

    char* gameConsonants;
    char* gameVowels;
    char* gameLetters;

    //gameConsonants and gameVowels are passed by reference so they can be used throughout main 
    //and other functions; they will be malloc'd in selectLetters
    setGamemode(gamemode, &gameConsonants, &gameVowels, &gameLetters, manual);
    printf("These will be your consonants: %s\n", gameConsonants);
    printf("And these will be your vowels: %s\n", gameVowels);

    //lastly we will use buildSides to separate each letter into their own respective side
    char* side1 = (char*)malloc(4);
    char* side2 = (char*)malloc(4);
    char* side3 = (char*)malloc(4);
    char* side4 = (char*)malloc(4);
    buildSides(gameLetters, &side1, &side2, &side3, &side4);

    //this is the minimum length needed for a word
    int minLength = 3;
    int maxLength = 0;

    //char* filename = "2of12.txt";

    //next we will build our dictionary of words, which will only have words that are at least minLength
    //but first we need to see how many words fit that restriction to properly build the words array
    //while also setting the max 
    int numWords = countMinLengthWords(filename, minLength, &maxLength);
    MAX_LEN = maxLength;
    char** words = (char**)malloc(numWords*sizeof(char*));

    for(size_t i = 0; i < numWords; i++) {
        words[i] = (char*)malloc(maxLength+1);
    }
    
    if(!buildWordArray(words, filename, minLength, numWords)) {
        return 0;
    }
    //now the words are all set!

    
    if(playMode) { //simulates playMode
        bool hasWon = false; //game will stop if this becomes true
        int maxWords = 0;
        printf("Time to start LetterBoxed! You're in playMode right now. :)\n");
        printf("The point of this game is to use each letter on your box at least once by connecting words, using the least amount of needed words possible.\n");
        printf("You will build words by connecting letters, however there is three major restrictions on connecting words.\n");
        printf("1. You cannot use the same letter twice consecutively.\n");
        printf("2. You cannot use letters on the same side consecutively, i.e., using two letters on the same side of the box together.\n");
        printf("3. When you successfully create a word, the next word MUST start with the last letter of the word you just made, i.e., 'connection' and 'next'.\n");

        printf("Each word must be at least %d characters long, and no more than %d characters long.\n", minLength, maxLength);
        printf("Here are your rules based on the gameMode you chose.\n");
        if(gamemode) {
            maxWords = 5;
            printf("The maximum amount of words you can make are %d. You can, of course, use less than this amount, which will be more difficult.\n", maxWords);
        } else {
            maxWords = 3;
            printf("The maximum amount of words you can make are %d. You can, of course, use less than this amount, which will be more difficult.\n", maxWords);
        }
        printf("Finally, you'll notice the nodes next to each letter on the box have a number, 0, which will change once used. The number will represent how many times you used it.\n");
        printf("\n");
        //printf("Let's get started!\n");

        //before game starts, i will initialize the wordNode struct in memory with needed amount based on maxWords
        WordNode* userWords = (WordNode*)malloc(sizeof(WordNode));
        userWords->word = (char*)malloc(maxLength+1);
        userWords->word[0] = '\0'; //we have to manually give the string a null terminated character

        userWords->prev = NULL;
        WordNode* current = userWords;
        WordNode* prev = userWords->prev;
        for(size_t i = 1; i < maxWords; i++) {
            current->next = (WordNode*)malloc(sizeof(WordNode));
            current->next->next = NULL;
            prev = current;
            current = current->next;
            current->word = NULL;
            current->prev = prev;
        }

        int currentWordNum = 0; //to keep track of which number we are on
        //usedLetters is the same length as gameLetters because each index represents each letter's
        //respective position in gameLetters; it will be updated through different actions and is 
        //necessary to change how the box is shown
        int* usedLetters = (int*)malloc(12*sizeof(int));
        for(size_t i = 0; i < 12; i++) {
            usedLetters[i] = 0;
        }

        while(!hasWon) { //game officially starts here
            //first we will display the box
            displayBox(gameLetters, usedLetters);
            printf("\n");
            printf("You have built %d word(s) and are currently building word number %d.\n", currentWordNum, currentWordNum+1);
            printf("Here are the previous word(s) you have built: --");

            //using current from earlier as it has access to the head of the list
            WordNode* curr = current;
            while(curr != NULL && curr->word != NULL) {
                printf("%s--", curr->word);
                curr = curr->next;
            }
            printf("\n\n");
            printf("Your current word so far: %s\n\n", userWords->word);
            printf("Enter a valid uppercase letter to continue building your word.\n\n");
            printf("If you want to start your next word, enter 'd' (must be lowercase).\n");
            printf("If you'd like to undo a letter, enter 'u' (must be lowercase). If you undo the first letter of a word, it will go back to the previous word.\n");
            printf("If you are finished, enter 'q' (must be lowercase) to confirm if you have won the game!\n");
            char enteredLetter;
            scanf(" %c", &enteredLetter);

            //make sure that char is alpha!
            if(!isalpha(enteredLetter)) {
                printf("Please enter a letter, not a digit or special character.\n");
                continue;
            }
            int ascii = (int)enteredLetter; //need the ascii value to determine what letter they used

            if(ascii == 113) { //user entered q

                //simply check if they have used all letters
                for(size_t i = 0; i < 12; i++) {
                    if(usedLetters[i] <= 0) {
                        hasWon = false;
                        break;
                    }
                    hasWon = true;
                }
                if(hasWon) {
                    printf("Congratulations! You beat the game using %d words! Keep in mind that the limit was %d. Whether you reached that limit or not, great job!\n", currentWordNum+1, maxWords);
                    printf("If you'd like to try another letter boxed puzzle with new letters, make sure to enter the correct arguments to play again!\n");
                    break;
                } else {
                    printf("Unfortunately, you did not use all of the letters provided in the box, so you lost. Feel free to try the game again with new letters!\n");
                    break;
                }

            } else if (ascii == 100) { //user entered d

                //first, we will make sure they made a word that is in our dictionary
                bool found = false;
                for(size_t i = 0; i < numWords; i++) {
                    if(strcmp(userWords->word, words[i]) == 0) {
                        found = true;
                        break;
                    }
                }

                if(!found) {
                    printf("The word you have created is not in the dictionary. Please make a new word.\n");
                    printf("\n");
                    continue;
                }

                //in this case, we will add the last letter of our current word to the next word as the first letter
                //this will only happen if they can still make another word, otherwise they should press Q
                //we also have to update the usedLetters array to reflect which letters are being used for this
                //specfic word, which will only be firstLetter
                //we will also update currentWordNum to reflect what word we are now on
                if(userWords->next != NULL) {
                    currentWordNum++;
                    char firstLetter = userWords->word[strlen(userWords->word)-1];
                    userWords = userWords->next;
                    userWords->word = (char*)malloc(maxLength+1);
                    userWords->word[0] = firstLetter;
                    userWords->word[1] = '\0';

                    char* ptr = strchr(gameLetters, toupper(firstLetter));
                    int used = ptr - gameLetters;
                    usedLetters[used]++;

                } else {
                    printf("You have made the max amount of words possible. You must check if you have won with 'Q' or undo with 'U' if you are sure you have not won just yet.\n");
                    continue;
                }
            } else if (ascii == 117) { //user entered u

                if(userWords->prev == NULL && strlen(userWords->word) < 1) {
                    printf("Your current word is your very first word, and does not contain any letters to undo.\n");
                    continue;
                } else if (strlen(userWords->word) == 1 && !(userWords->prev == NULL)) {

                    //in the case that the user only has 1 letter in their word, we need to go back to the previous word 
                    //and it is best practice to free the string we were on before
                    //we also will update usedLetters using checkUsedLetter
                    char firstLetter = userWords->word[0];
                    char* ptr = strchr(gameLetters, toupper(firstLetter));
                    int used = ptr - gameLetters;
                    usedLetters[used]--;
                    free(userWords->word);
                    userWords->word = NULL;
                    userWords = userWords->prev;
                    currentWordNum--;

                } else if (strlen(userWords->word) == 1 && (userWords->prev == NULL)) {
                    //printf("I am running the case where there is the first word and 1 letter\n");
                    char firstLetter = userWords->word[0];
                    char* ptr = strchr(gameLetters, toupper(firstLetter));
                    int used = ptr - gameLetters;
                    usedLetters[used]--;
                    userWords->word[0] = '\0';  
                } else {

                    //in the case that the user has more than 1 letter in their word, we just need to
                    //remove the last letter from their word and update usedLetters using checkUsedLetter
                    char lastLetter = userWords->word[strlen(userWords->word)-1];
                    char* ptr = strchr(gameLetters, toupper(lastLetter));
                    int used = ptr - gameLetters;
                    usedLetters[used]--;
                    userWords->word[strlen(userWords->word)-1] = '\0';
                } //there should never be a case in which the word has a length of 0 unless it is the first word
            } else { //user entered a letter to continue building their word

                //in this case we need to check some restrictions

                //1. the letter must be within the letters available
                //2. the letter must not exceed the word limit for this WordNode
                //3. the letter must not be on the same side, checked with isValidAdjacency

                if(strchr(gameLetters, toupper(enteredLetter)) == NULL) {
                    printf("The letter you chose must be one of the letters on the box. This letter is not.\n\n");
                    continue;
                } else if (strlen(userWords->word) + 1 > maxLength) {
                    printf("Adding another letter to this word would exceed the word length limit. Please choose another option.\n\n");
                    continue;
                } else if (!isValidAdjacency(userWords->word[strlen(userWords->word)-1], enteredLetter, side1, side2, side3, side4)) {
                    printf("The letter you chose must be from a different side than the last letter of your current word. This letter is not.\n\n");
                    continue;
                }

                //if all these are passed, then we can add it to this current WordNode's word, and 
                //we should also update usedLetters at the index of this specific letter
                int len = strlen(userWords->word);
                userWords->word[len] = enteredLetter;
                userWords->word[len+1] = '\0';

                char* ptr = strchr(gameLetters, toupper(enteredLetter));
                int used = ptr - gameLetters;
                //printf("The number for used is %d\n", used);
                usedLetters[used]++; //to indicate the user has used this letter

                /*debug
                printf("UsedLetters debug: ");
                for (int i = 0; i < 12; i++) {
                    printf("%d ", usedLetters[i]);
                }*/
                printf("\n");
            }
        }
    }

    if(solverMode) { // simulates solverMode
        //printf("Solver is not built just yet...\n");
        printf("--- SOLVER MODE ---\n");

        //we will add all valid words to a new words array called solvedWords; these words
        //follow sides restrictions and any other previous restrictions on the original words array
        //any of these words can be used to solve a part of the letterboxed puzzle
        int numSolved = 0;
        int maxSolved = 4; 
        char** solvedWords = (char**)malloc(maxSolved*sizeof(char*));
        for(size_t i = 0; i < numWords; i++) {

            bool isValid = true;
            char* word = words[i];
            for(size_t j = 0; j < strlen(word); j++) {
                if(j < strlen(word)-1) { 
                    if(strchr(gameLetters, toupper(word[j])) == NULL || !(isValidAdjacency(word[j], word[j+1], side1, side2, side3, side4))) {
                        isValid = false;
                        break;
                    }
                } else if (strchr(gameLetters, toupper(word[j])) == NULL) {
                    isValid = false;
                    break;
                }
            }
            if(isValid) {
                appendWord(&solvedWords, &numSolved, &maxSolved, word);
            }
        }
        
        //printing all solvedWords
        for(size_t i = 0; i < numSolved; i++) {
            //continue;
            printf("%s\n",solvedWords[i]);
        }
        printf("%d Words Total\n",numSolved);

        int numWordNodes = 0;
        int maxWordNodes = 4;
        WordNode* newNode = NULL;
        WordNode** nodeList = (WordNode**)malloc(maxWordNodes*sizeof(WordNode*));
        for(size_t i = 0; i < maxWordNodes; i++) {
            nodeList[i] = (WordNode*)malloc(sizeof(WordNode));
        }
        bool foundLetters[12]; //not to be confused with int* usedLetters
        for(size_t i = 0; i < 12; i++) {
            foundLetters[i] = false;
        }

        /*findTwoCombos(foundLetters, gameLetters, solvedWords, numSolved, &newNode, &nodeList, &numWordNodes, &maxWordNodes);
        printf("the amount of two word combos: %d\n", numWordNodes);
        //fflush(stdout);
        printf("Printing all two-word combos for this puzzle...\n\n");
        for(size_t i = 0; i < numWordNodes; i++) {
            WordNode* curr = nodeList[i];
            printf("|--%s", curr->word);
            curr = curr->next;
            printf("--%s--|\n\n", curr->word);
        }
        printf("-----------------------------------\n");
        */
        //printf("Printing all one-word combos for this puzzle with a queue algorithm...\n\n");
        //LadderNode* oneWordList = (LadderNode*)malloc(sizeof(LadderNode));
        //oneWordList = findNumberedSolution(solvedWords, gameLetters, numSolved, 1);
        //printf("One-word combos w/ queue algorithm complete.\n\n\n");


        printf("Printing all two-word combos for this puzzle with a queue algorithm...\n\n");
        LadderNode* twoWordList = (LadderNode*)malloc(sizeof(LadderNode));
        twoWordList = findNumberedSolution(solvedWords, gameLetters, numSolved, 3);
        printf("Two-word combos w/ queue algorithm complete.\n\n\n");

        //printf("Printing all three-word combos for this puzzle with a queue algorithm...\n\n");
        //LadderNode* threeWordList = (LadderNode*)malloc(sizeof(LadderNode));


        //threeWordList = findNumberedSolution(solvedWords, gameLetters, numSolved, 3);
        /*LadderNode* currLadder = threeWordList;
        while(currLadder != NULL) {
            WordNode* currWord = currLadder->wordList;
            while(currWord != NULL) {
                printf("%s--", currWord->word);
                currWord = currWord->next;
            }
            printf("\n");
            currLadder = currLadder->next;
        }*/

        printf("-----------------------------------\n");
        return 0;
    } 

}

bool setSettings(int argc, char* argv[], char filename[100], bool* manual, bool* playMode, bool* solverMode, bool* gamemode, bool* seeded) {
    strcpy(filename, "dictionary.txt");
    srand(time(0));

    for(size_t i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-p") == 0) {
            *playMode = true;
        } else if(strcmp(argv[i], "-s") == 0) {
            *solverMode = true;
        } else if(strcmp(argv[i], "-r") == 0) {
            ++i;
            if(i != argc) {
                for(size_t j = 0; j < strlen(argv[i]); j++) {
                    if(!isdigit(argv[i][j])) {
                        return false;
                    }
                }
                int seed = atoi(argv[i]);
                srand(seed);
            } else {
                return false;
            }
        } else if(strcmp(argv[i], "-d") == 0) {
            ++i;
            if(i != argc) {
                strcpy(filename, argv[i]);
                FILE* f = fopen(filename, "r");
                if(f == NULL) {
                    fclose(f);
                    return false;
                }
                fclose(f);
            }  else {
                return false;
            }
        } else if(strcmp(argv[i], "-e") == 0) {
            *gamemode = true;
        } else if (strcmp(argv[i], "-h") == 0) {
            *gamemode = false;
        } else if(strcmp(argv[i], "-m") == 0) {
            *manual = true;
        } else {
            return false;
        }
    }
    return true;
}

void setGamemode(bool gamemode, char** gameConsonants, char** gameVowels, char** gameLetters, bool manual) {
    int consonantLimit;
    int vowelLimit;
    if(gamemode) {
        consonantLimit = 8;
        vowelLimit = 4;
        printf("Gamemode set to easy. You will have 4 vowels and 8 consonants. Good luck!\n");
        selectLetters(&consonantLimit, &vowelLimit, gameConsonants, gameVowels, gameLetters, manual);
    } else {
        consonantLimit = 10;
        vowelLimit = 2;
        printf("Gamemode set to hard. You will have 2 vowels and 10 consonants. Good luck!\n");
        selectLetters(&consonantLimit, &vowelLimit, gameConsonants, gameVowels, gameLetters, manual);
    }

}

//randomly selects letters based on game rules; there is a limit to amount of consonants and vowels chosen
void selectLetters(int* consonantLimit, int* vowelLimit, char** gameConsonants, char** gameVowels, char** gameLetters, bool manual) {
    //variable for the letters the users will have for this game
    *gameConsonants = (char*)malloc(*consonantLimit+1);
    *gameVowels = (char*)malloc(*vowelLimit+1);
    *gameLetters = (char*)malloc(13);

    //using copies of the global variables so i can remove them one by one as they are chosen for the game
    char* consonantsCopy = malloc(strlen(consonants) + 1);
    strcpy(consonantsCopy, consonants);

    char* vowelsCopy = malloc(strlen(vowels) + 1);
    strcpy(vowelsCopy, vowels);

    //let users manually enter their consonants and vowels
    if(manual) {
        int count = 0;
        int i = 0;
        int j = 0;
        while(count < 12 && (i < *consonantLimit || j < *vowelLimit)) {
            char c;
            printf("Enter a letter (must not be one you have already used):\n");
            scanf(" %c", &c);
            if(strchr(consonants, toupper(c)) && i < *consonantLimit) {
                (*gameLetters)[count] = toupper(c);
                (*gameConsonants)[i] = toupper(c);
                i++;
            } else if (strchr(vowels, toupper(c)) && j < *vowelLimit) {
                (*gameLetters)[count] = toupper(c);
                (*gameVowels)[j] = toupper(c);
                j++;
            } else {
                printf("This is not a letter. Try again.\n");
                continue;
            }
            count++;
        }

        return;
    }
    //add in consonants randomly to consonantsCopy, only enough to reach consonantLimit

    for (size_t i = 0; i < *consonantLimit; i++) {
        size_t len = strlen(consonantsCopy);
        if (len == 0) {
            printf("No consonants left to pick from!\n");
            break;
        }

        size_t pick = rand() % len;
        (*gameConsonants)[i] = consonantsCopy[pick];

        // remove picked char from consonantsCopy
        for (size_t j = pick; j < len - 1; j++) {
            consonantsCopy[j] = consonantsCopy[j + 1];
        }
        consonantsCopy[len - 1] = '\0';
    }
    (*gameConsonants)[*consonantLimit] = '\0';
    free(consonantsCopy);

    //now same thing but for vowels, add them in randomly



    for (size_t i = 0; i < *vowelLimit; i++) {
        size_t len = strlen(vowelsCopy);
        if (len == 0) {
            printf("No vowels left to pick from!\n");
            break;
        }

        size_t pick = rand() % len;
        (*gameVowels)[i] = vowelsCopy[pick];

        // remove picked char from vowelsCopy
        for (size_t j = pick; j < len - 1; j++) {
            vowelsCopy[j] = vowelsCopy[j + 1];
        }
        vowelsCopy[len - 1] = '\0';
    }
    (*gameVowels)[*vowelLimit] = '\0';
    free(vowelsCopy);


    
    //lastly, we will add gameConsonants and gameVowels randomly into the gameLetters
    char* gcCopy = (char*)malloc(*consonantLimit+1);
    char* gvCopy = (char*)malloc(*vowelLimit+1);

    strcpy(gcCopy, *gameConsonants);
    strcpy(gvCopy, *gameVowels);

    //if firstPick is 0, we will add a consonant, otherwise, add a letter
    //in the case that one of them is empty, then we will just add the other instead (randomly)
    for(size_t i = 0; i < 12; i++) {
        size_t firstPick = rand() % 2;
        size_t len1 = strlen(gcCopy);
        size_t len2 = strlen(gvCopy);
        if(len1 < 1 && len2 < 1) {
            printf("All letters have been taken out of gvCopy and gcCopy.\n");
            break;
        }
        if(len1 < 1) {
            size_t secondPick = rand() % len2;
            (*gameLetters)[i] = gvCopy[secondPick];
            for(size_t j = secondPick; j < len2 - 1; j++) {
                gvCopy[j] = gvCopy[j+1];
            }
            gvCopy[len2-1] = '\0';
        } else if (len2 < 1) {
            size_t secondPick = rand() % len1;
            (*gameLetters)[i] = gcCopy[secondPick];
            for(size_t j = secondPick; j < len1 - 1; j++) {
                gcCopy[j] = gcCopy[j+1];
            }
            gcCopy[len1-1] = '\0';
        } else if (firstPick == 0) {
            size_t secondPick = rand() % len1;
            (*gameLetters)[i] = gcCopy[secondPick];
            for(size_t j = secondPick; j < len1 - 1; j++) {
                gcCopy[j] = gcCopy[j+1];
            }
            gcCopy[len1-1] = '\0';
        } else if (firstPick == 1) {
            size_t secondPick = rand() % len2;
            (*gameLetters)[i] = gvCopy[secondPick];
            for(size_t j = secondPick; j < len2 - 1; j++) {
                gvCopy[j] = gvCopy[j+1];
            }
            gvCopy[len2-1] = '\0';
        }

    }
    free(gcCopy);
    free(gvCopy);
    (*gameLetters)[12] = '\0';

}

//counts the amount of words in the dictionary that are at least minLength, and
//updates maxLength to the length of the longest word in the dictionary
int countMinLengthWords(char* filename, int minLength, int* maxLength) {
    FILE* f = fopen(filename, "r");
    if(f == NULL) {
        fclose(f);
        return false;
    }

    int count = 0;
    while(!feof(f)) {
        char word[30];
        fscanf(f, "%s", word);
        if(strlen(word) >= minLength) {
            count++;
        }

        if(strlen(word) > *maxLength) {
            *maxLength = strlen(word);
        }
    }
    printf("This is the maxLength after finishing the countMinLengthWords function: %d\n",  *maxLength);
    fclose(f);
    return count;
}

void buildSides(char* gameLetters, char** side1, char** side2, char** side3, char** side4) {
    for(size_t i = 0; i < 3; i++) {
        (*side1)[i] = gameLetters[i];
    }

    for(size_t i = 0; i < 3; i++) {
        (*side2)[i] = gameLetters[i+3];
    }

    for(size_t i = 0; i < 3; i++) {
        (*side3)[i] = gameLetters[i+6];
    }

    for(size_t i = 0; i < 3; i++) {
        (*side4)[i] = gameLetters[i+9];
    }
}

bool buildWordArray(char** words, char* filename, int minLength, int numWords) {
    FILE* f = fopen(filename, "r");
    if(f == NULL) {
        fclose(f);
        return false;
    }

    size_t iter = 0;

    while(!feof(f)) {
        if(iter > numWords) {
            printf("You have exceeded the amount of words with length: %d\n", minLength);
            fclose(f);
            return false;
        }
        char word[30];
        fscanf(f, "%s", word);
        if(strlen(word) >= minLength) {
            strcpy(words[iter], word);
            iter++;
        }
    }
    if(iter != numWords) {
        printf("You have less than the required amount of words with length: %d\n", minLength);
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

bool isValidAdjacency(char letter1, char letter2, char* side1, char* side2, char* side3, char* side4) {
    //first we will see which side the first letter is on
    //when we have the side for the first letter, we will determine if the second letter is on that side
    //if it is, that is invalid, but if not, then that is valid
    if(strchr(side1, toupper(letter1)) != NULL) {
        if(strchr(side1, toupper(letter2)) != NULL) {
            return false;
        } else {
            return true;
        } 
    } else if (strchr(side2, toupper(letter1)) != NULL) {
        if(strchr(side2, toupper(letter2)) != NULL) {
            return false;
        } else {
            return true;
        }
    } else if (strchr(side3, toupper(letter1)) != NULL) {
        if(strchr(side3, toupper(letter2)) != NULL) {
            return false;
        } else {
            return true;
        }
    } else if (strchr(side4, toupper(letter1)) != NULL) {
        if(strchr(side4, toupper(letter2)) != NULL) {
            return false;
        } else {
            return true;
        }
    } 


}

//this function checks if the used letter has already been used before in a previous word, and returns
//whatever WordNode it is used in
int checkUsedLetter(WordNode* userWords, int currentWordNum, char letter) {
    int count = 0;
    //check in backwards order to give most recent WordNode it was used in
    for(int i = currentWordNum; i >= 0; i--) {
        if(strchr(userWords[i].word, toupper(letter)) != NULL) {
            count++;
        }
    }
    return count;
}

//This function should print the box for the 'LetterBoxed' game
void displayBox(char* gameLetters, int* usedLetters) {
    printf("        %c     %c     %c\n", gameLetters[3], gameLetters[4], gameLetters[5]);
    printf("     __[%d]___[%d]___[%d]__\n", usedLetters[3], usedLetters[4], usedLetters[5]);
    printf("     |                  |\n");
    printf("%c [%d]                [%d] %c\n", gameLetters[0], usedLetters[0], usedLetters[6], gameLetters[6]);
    printf("     |                  |\n");
    printf("     |                  |\n");
    printf("%c [%d]                [%d] %c\n", gameLetters[1], usedLetters[1], usedLetters[7], gameLetters[7]);
    printf("     |                  |\n");
    printf("     |                  |\n");
    printf("%c [%d]                [%d] %c\n", gameLetters[2], usedLetters[2], usedLetters[8], gameLetters[8]);
    printf("     |                  |\n");
    printf("      __[%d]___[%d]___[%d]__\n", usedLetters[9], usedLetters[10], usedLetters[11]);
    printf("         %c     %c     %c\n", gameLetters[9], gameLetters[10], gameLetters[11]);
}

void appendWord(char*** pWords, int* pNumWords, int* pMaxWords, char* newWord) {
    (*pNumWords)++;
    if (*pNumWords > *pMaxWords) {
        *pMaxWords *= 2;
        char** temp = (char**)malloc(*pMaxWords*sizeof(char*));
        for (int i = 0; i < *pNumWords-1; ++i) {
            temp[i] = (*pWords)[i];
        }
        free(*pWords);
        *pWords = temp;
    }
    //(*words)[(*numWords)-1] = newWord; //invalid way to add the word... requires a new allocation
    (*pWords)[(*pNumWords)-1] = (char*)malloc(sizeof(char)*(strlen(newWord)+1));
    strcpy((*pWords)[(*pNumWords)-1],newWord);
}

void appendWordNode(WordNode*** pWordNodes, int* pNumWordNodes, int* pMaxWordNodes, WordNode** newWordNode) {
    (*pNumWordNodes)++;
    if (*pNumWordNodes > *pMaxWordNodes) {
        *pMaxWordNodes *= 2;
        WordNode** temp = (WordNode**)malloc(*pMaxWordNodes*sizeof(WordNode*));
        for (int i = 0; i < *pNumWordNodes-1; ++i) {
            temp[i] = (*pWordNodes)[i];
        }
        free(*pWordNodes);
        *pWordNodes = temp;
    }

    (*pWordNodes)[(*pNumWordNodes)-1] = (WordNode*)malloc(sizeof(WordNode));
    (*pWordNodes)[(*pNumWordNodes)-1] = *newWordNode;
}

//very inefficient way to find all two word combos, aka solutions that only use two words
void findTwoCombos(bool usedLetters[12], char* gameLetters, char** solvedWords, int numWords, WordNode** newNode, WordNode*** pWordNodes, int* pNumWordNodes, int* pMaxWordNodes) {
    //start with an outer loop that takes the first word, then go to the next word
    for(size_t i = 0; i < numWords; i++) {
        *newNode = (WordNode*)malloc(sizeof(WordNode));
        (*newNode)->next = (WordNode*)malloc(sizeof(WordNode));
        char firstWord[100];
        strcpy(firstWord, solvedWords[i]);
        bool allLettersFound = true;
        for(size_t k = 0; k < strlen(firstWord); k++) {
            int index = strchr(gameLetters, toupper(firstWord[k])) - gameLetters;
            //printf("this is index of firstWord k in gameLetters: %d\n", index);
            usedLetters[index] = true;
        }
        for(size_t j = 0; j < numWords; j++) {
            char secondWord[100];
            if(strcmp(firstWord, solvedWords[j]) == 0) { //if its the exact same word, skip it
                continue;
            }
            strcpy(secondWord, solvedWords[j]);
            bool found = true;
            bool allLettersFound = true;
            //this loop will check if our second word at least contains every letter that is missing from the first;
            //it also checks if every value in usedLetters is true; this means we found a firstWord that contains all 12 letters
            //in that case, or if our second word is missing a letter still, then we will continue our j loop and nothing is appended
            for(size_t k = 0; k < 12; k++) {
                if(!usedLetters[k]) {
                    allLettersFound = false;
                    found = strchr(secondWord, tolower(gameLetters[k]));
                    if(!found) { break; }
                }
            }
            if(allLettersFound || !found) {
                continue;
            } else {
                //we will make space for both words in this WordNode, copy them, and append the WordNode to the list
                //also free both spaces that were malloc'd
                (*newNode)->word = (char*)malloc(strlen(firstWord) * sizeof(char));
                strcpy((*newNode)->word, firstWord);
                //newNode->next = (WordNode*)malloc(sizeof(WordNode));
                (*newNode)->next->word = (char*)malloc(strlen(secondWord) * sizeof(char));
                strcpy((*newNode)->next->word, secondWord);
                //printf("These are the words that were put into newNode, %s and %s\n\n", (*newNode)->word, (*newNode)->next->word);
                appendWordNode(pWordNodes, pNumWordNodes, pMaxWordNodes, newNode);
                //printf("These are the words that were put into newNode, %s and %s\n\n", (*newNode)->word, (*newNode)->next->word);
                //free((*newNode)->word);
                //free((*newNode)->next->word);
            }
        }
        //lastly, reset usedLetters for the next firstWord
        for(size_t k = 0; k < 12; k++) {
            if(usedLetters[k]) {
                usedLetters[k] = false;
            }
        }
    }
}

int printLadder(WordNode* ladder) {
    while(ladder != NULL) {
        printf("%s -> ",ladder->word);
        ladder = ladder->next;
    }
    printf("\n");
}

int getLadderLength(WordNode* ladder) {
    int count = 0;
    if(ladder == NULL) {
        return count;
    }

    count++;

    while(ladder->next != NULL) {
        count++;
        ladder = ladder->next;
    }

    return count;
}

int getLadderListLength(LadderNode* ladderList) {
    int count = 0;
    if(ladderList == NULL) {
        return count;
    }

    count++;

    while(ladderList->next != NULL) {
        count++;
        ladderList = ladderList->next;
    }

    return count;
}


WordNode* popLadderFromFront(LadderNode** list) {
    WordNode* deletedNode = NULL;

    if(*list == NULL) {return NULL;}
    LadderNode* front = *list;
    deletedNode = front->wordList;

    *list = front->next;

    front->wordList = NULL;
    front->next = NULL;
    free(front->indexList);
    free(front);

    return deletedNode; 
}


void insertLadderAtBack(LadderNode** list, WordNode* newLadder) {
    LadderNode* newNode = malloc(sizeof(LadderNode));
    newNode->wordList = newLadder;
    newNode->next = NULL;
    if(*list == NULL) {
        *list = newNode;
        return;
    } else {
        LadderNode* temp = *list;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}


//this function copies the ladder from a previous one
WordNode* copyLadder(WordNode* ladder) {
    if(ladder == NULL) {
        return NULL;
    }

    WordNode* ladderCopy = NULL;
    WordNode* tail = NULL;
    WordNode* curr = ladder;

    while(curr != NULL) {
        //insertWordAtFront(&ladderCopy, curr->myWord);
        WordNode* newFront = (WordNode*)malloc(sizeof(WordNode));
        newFront->word = curr->word;
        newFront->next = NULL;
        newFront->prev = tail;

        if(ladderCopy == NULL) {
            ladderCopy = newFront;
        } else {
            tail->next = newFront;
        }

        tail = newFront;
        curr = curr->next;
    }
    return ladderCopy;

}

LadderNode* findNumberedSolution(char** solvedWords, char* gameLetters, int solvedLength, int reqNum) {
    LadderNode* myLadder = NULL;
    LadderNode* tail = NULL;

    //initialize the ladder with a node for every word in solvedWords
    LadderNode* currLadder = myLadder;
    for(size_t i = 0; i < solvedLength; i++) {
        //add a new word to the list
        currLadder = (LadderNode*)malloc(sizeof(LadderNode));
        currLadder->next = NULL;
        currLadder->wordList = (WordNode*)malloc(sizeof(WordNode));
        currLadder->indexList = (int*)malloc(sizeof(int)*reqNum);
        currLadder->indexList[0] = i;
        currLadder->wordList->word = solvedWords[i];
        currLadder->wordList->prev = NULL;
        currLadder->wordList->next = NULL;
        //currLadder = currLadder->next;

        if (myLadder == NULL) {
            myLadder = currLadder;
            tail = currLadder;
        } else {
            tail->next = currLadder;
            tail = currLadder;
        }
    }

    //now we will run the algorithm to enqueue/dequeue, based on a few factors
    //currLadder = myLadder;
    //int hi = 0;
    while(myLadder != NULL) {
        //hi++;
        //if(hi % 300 == 0) {printf("ran 10 times\n");}
        //we need to check if the current ladder has the req num of words AND
        //has a list of words that qualify as a solution. if not, then we can either
        //
        //we can check req num with getLadderLength, and we can check if the list of words
        //qualifies by checking if each current word also starts with the lastLetter of the one 
        //before it; we also need to keep track of the index that the word is at within solvedWords,
        //so we do not repatedly add the same words to the same ladderNode
        WordNode* currentWordNode = popLadderFromFront(&myLadder);
        if(!currentWordNode) {
            break;
        }
        char* allWords = (char*)malloc(sizeof(char) * 100);
        bool hasAllLetters = true;
        
        //first let's check if the words are valid by reqNum
        int ladderLength = getLadderLength(currentWordNode);
        //printf("This is the ladder length: %d\n", ladderLength);
        //printLadder(currentWordNode);
        

        if(ladderLength+1 == reqNum) {
            //printf("We have found a ladder that has the right length.\n");
            //we can either remove this ladder if it is only the first word left, 
            //or replace the word then move it to the back, or move on if this ladder
            //is valid
            WordNode* head = currentWordNode;
            strcpy(allWords, head->word);
            while(head->next != NULL) {
                head = head->next;
                strcat(allWords, head->word);
            }

            for(size_t i = 0; i < strlen(gameLetters); i++) {
                if(!strchr(allWords, tolower(gameLetters[i]))) {
                    //printf("This combo of words: %s does not contain this specific letter: %c\n", allWords, tolower(gameLetters[i]));
                    hasAllLetters = false;
                    break;
                }
            }

            //break;
            int length = strlen(currentWordNode->word);
            //char lastLetter = currentWordNode->word[length-1];
            //printf("This is allWords and this is gameLetters: %s, %s\n", allWords, gameLetters);
            //printf("This is allWords when we have the right length: %s\n", allWords);
            if(hasAllLetters) {
                printf("We have found a solution!!!!\n");
                //yay we found a solution! we can move on to the next ladder.
                WordNode* printNode = currentWordNode;
                printf("|");
                for(size_t i = 0; i < reqNum; i++) {
                    printf("%s--", printNode->word);
                    printNode = printNode->next;
                    
                }
                printf("|");
                printf("\n");
            }

        } else {
            //we can add another word onto this (unless it has already solved the puzzle)
            if(ladderLength+1 > reqNum) {break;}
            WordNode* curr = currentWordNode;
            strcpy(allWords, curr->word);
            while(curr->next != NULL) {
                curr = curr->next;
                strcat(allWords, curr->word);
            }
            for(size_t i = 0; i < strlen(gameLetters); i++) {
                if(!strchr(allWords, tolower(gameLetters[i]))) {
                    hasAllLetters = false;
                    break;
                }
            }
            //printf("This is allWords: %s\n", allWords);


            /*if(hasAllLetters) {
                //remove the current word and replace with a new one or pop if no new words left
                int currIndex = currLadder->indexList[ladderLength];
                //we can usually just replace like we do in the previous else if case
                if(currIndex < solvedLength-1) {
                    currIndex++;
                    if(solvedWords[currIndex][0] == lastLetter) {
                        currentWordNode->word = solvedWords[currIndex];
                        currLadder->indexList[ladderLength] = currIndex; 
                    } else {
                        //if the next word that we found using currIndex does not start with the
                        //last letter of the current, that means there are no more words that start with
                        //that letter. in this case we should pop and add a new ladder node without this word
                        //LadderNode* newLadder = (LadderNode*)malloc(sizeof(LadderNode));
                        WordNode* wordsCopy = copyLadder(currLadder->wordList);
                        LadderNode* copyCurr = currLadder;
                        currLadder = currLadder->next;
                        popLadderFromFront(&copyCurr);
                        insertLadderAtBack(&myLadder, wordsCopy);
                    }


            */}
            if(!hasAllLetters) {
                //were going to add all words we find that has its first letter as the last letter
                //of our current word. we'll also add the index of this word to the list, that way, when
                //we need to access it we can. when we access it, we can simply increment rather than
                //having to linear search the whole solvedWords to replace it since the list is in
                //alphabetical order
                WordNode* lastWord = currentWordNode;
                while(lastWord->next != NULL) {lastWord = lastWord->next;}
                int length = strlen(lastWord->word);
                char lastLetter = lastWord->word[length-1];
                for(size_t i = 0; i < solvedLength; i++) {
                    if(solvedWords[i][0] == lastLetter) {
                        //first make sure this word is not already in the list
                        WordNode* head = currentWordNode;
                        bool isInList = false;
                        while(head != NULL) {
                            if(strcmp(head->word, solvedWords[i]) == 0) {
                                isInList = true;
                                break;
                            }
                            head = head->next;
                        }

                        if(isInList) {continue;}
                        WordNode* copy = copyLadder(currentWordNode);

                        WordNode* tail = copy;
                        while(tail->next != NULL) {tail = tail->next;}
                        tail->next = (WordNode*)malloc(sizeof(WordNode));
                        tail->next->word = solvedWords[i];
                        tail->next->prev = tail;
                        tail->next->next = NULL;

                        insertLadderAtBack(&myLadder, copy);
                        //printf("LadderList length = %d\n",getLadderListLength(myLadder));
                        //printf("   %s %s %s %c %c\n",copy->word,copy->next->word,solvedWords[i], solvedWords[i][0], lastLetter);



char* allWords2 = (char*)malloc(sizeof(char) * 100);
        
 hasAllLetters = true;
//printf("We have found a ladder that has the right length.\n");
            //we can either remove this ladder if it is only the first word left, 
            //or replace the word then move it to the back, or move on if this ladder
            //is valid
            head = copy;
            strcpy(allWords2, head->word);
            while(head->next != NULL) {
                head = head->next;
                strcat(allWords2, head->word);
            }

            for(size_t i = 0; i < strlen(gameLetters); i++) {
                if(!strchr(allWords2, tolower(gameLetters[i]))) {
                    //printf("This combo of words: %s does not contain this specific letter: %c\n", allWords, tolower(gameLetters[i]));
                    hasAllLetters = false;
                    break;
                }
            }

            //break;
            int length = strlen(copy->word);
            //char lastLetter = currentWordNode->word[length-1];
            //printf("This is allWords and this is gameLetters: %s, %s\n", allWords, gameLetters);
            //printf("This is allWords when we have the right length: %s\n", allWords);
            if(hasAllLetters) {
                printf("We have found a solution!!!!\n");
                //yay we found a solution! we can move on to the next ladder.
                printLadder(copy);
                /*WordNode* printNode = copy;
                printf("|");
                for(size_t i = 0; i < reqNum; i++) {
                    printf("%s--", printNode->word);
                    printNode = printNode->next;
                    
                }
                printf("%s--", tail->next->word);
                */
                printf("|");
                printf("\n");
            }










                    }
                }
                

        }

        free(allWords);

        while(currentWordNode->next != NULL) {
            WordNode* deleted = currentWordNode;
            currentWordNode = currentWordNode->next;
            free(deleted);
        }
        free(currentWordNode);
    }
    return myLadder;
}


/*
        _☒   _☒    _☒
     __[0]___[0]___[0]__
     |                  |
☒_ [0]                [0] _☒
     |                  |
     |                  |
☒_ [0]                [0] _☒
     |                  |
     |                  |
☒_ [0]                [0] _☒
     |                  | 
      __[0]___[0]___[0]__
       ☒_    ☒_   ☒_
*/




