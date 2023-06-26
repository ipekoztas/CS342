#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>

#define MAX_FILES 100
#define MAX_WORD_LEN 64

// Ipek Oztas - Emre Karatas
// a data structure to keep words and counts together. This is a linked list application.
struct dataItem
{
    char* word;
    struct dataItem* next;
    struct dataItem* previous;
    int wordCount;
};

struct thread_arg 
{
    int threadCounter;
    char* filename;
};


struct dataItem* tail = NULL;
struct dataItem* pushData(struct dataItem* head, char* word)
{
    // Check if the word already exists in the linked list
    struct dataItem* current = head;
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->wordCount++;
            //printf("current is increased. current : %s, count is: %d \n", current->word,current->wordCount);
            return head;
        }
        current = current->next;
    }
    
    // If the word does not exist, create a new node for it
    struct dataItem* newItem = (struct dataItem*)malloc(sizeof(struct dataItem));
    if (newItem == NULL) {
        // Handle memory allocation error
        printf("Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    newItem->word = strdup(word);
    if (newItem->word == NULL) {
        // Handle memory allocation error
        printf("Memory allocation error\n");
        free(newItem);
        exit(EXIT_FAILURE);
    }
    
    newItem->wordCount = 1; 
    newItem->next = NULL;
    newItem->previous = tail;
    
    if (head == NULL) {
        head = newItem;
    }
    
    if (tail != NULL) {
        tail->next = newItem;
    }
    
    tail = newItem;
    //printf("tail is : %s, count is: %d \n", tail->word,tail->wordCount);
    return head;
}
struct dataItem* finaltail = NULL;
struct dataItem* finalPushData(struct dataItem* head, char* word, int count)
{
    // Check if the word already exists in the linked list
    struct dataItem* current = head;
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) 
        {
            current->wordCount = count;
            //printf("copied %s ",current->word);
            return head;
        }
        current = current->next;
    }
    
    // If the word does not exist, create a new node for it
    struct dataItem* newItem = (struct dataItem*)malloc(sizeof(struct dataItem));
    if (newItem == NULL) {
        // Handle memory allocation error
        printf("Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    newItem->word = strdup(word);
    if (newItem->word == NULL) {
        // Handle memory allocation error
        //printf("Memory allocation error\n");
        free(newItem);
        exit(EXIT_FAILURE);
    }
    
    newItem->wordCount = count; 
    newItem->next = NULL;
    newItem->previous = finaltail;
    
    if (head == NULL) {
        head = newItem;
    }
    
    if (finaltail != NULL) {
        finaltail->next = newItem;
    }
    
    finaltail = newItem;
    //printf("tail is : %s, count is: %d \n", tail->word,tail->wordCount);
    return head;
}


void swap(struct dataItem* a, struct dataItem* b) {
    char* tempWord = a->word;
    int tempCount = a->wordCount;
    a->word = b->word;
    a->wordCount = b->wordCount;
    b->word = tempWord;
    b->wordCount = tempCount;
}


void selectionSort(struct dataItem* start) 
{
    if (start == NULL) 
    {
        return;
    }
    
    struct dataItem* ptr1, * ptr2, * min;
    
    for (ptr1 = start; ptr1->next != NULL; ptr1 = ptr1->next) 
    {
        min = ptr1;
        
        for (ptr2 = ptr1->next; ptr2 != NULL; ptr2 = ptr2->next) 
        {
            if (ptr2->wordCount > min->wordCount) 
            {
                min = ptr2;
            }
        }
        
        if (min != ptr1) 
        {
            swap(ptr1, min);
        }
    }
}


/* Function to write output file */
void printOutputFile(char* outputFileName, struct dataItem* finalList, int topKWords) 
{
    FILE* outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) 
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    int count = topKWords;
    while (finalList != NULL && count > 0) 
    {
        fprintf(outputFile, "%s %d\n", finalList->word, finalList->wordCount);
        finalList = finalList->next;
        count--;
    }
    if (fclose(outputFile) != 0) 
    {
        perror("Error closing output file");
        exit(EXIT_FAILURE);
    }
}

struct dataItem** headPointer = NULL;
struct dataItem* parent = NULL;
void* thread_func(void* arguments);

void checkThreads(struct dataItem** parent, struct dataItem* root)
{
	if(root == NULL)
	{
	    return;
	}
	else
	{
		*parent = finalPushData(*parent,root->word,root->wordCount);
    		checkThreads(parent, root->next);
		
		
	}
}


int main(int argc, char* argv[])
{
    if (argc < 4) 
    {
        printf("Usage: threadtopk <K> <outfile> <N> <infile1> .... <infileN>\n");
        exit(EXIT_FAILURE);
    }

    int K = atoi(argv[1]);
    char* outputFileName = argv[2];
    int N = atoi(argv[3]);
    char* inputFiles[N];
    headPointer = malloc(sizeof(struct dataItem*) * N);
    struct thread_arg threadArguments[N];
    pthread_t threads[N];

    for (int i = 0; i < N; i++) 
    {
        inputFiles[i] = argv[i + 4];
    }
   for (int i = 0; i < N; i++) 
    {
        threadArguments[i].threadCounter = i;   
        threadArguments[i].filename = inputFiles[i];
        pthread_create(&threads[i], NULL, &thread_func, (void*) &threadArguments[i]);
        //printf("thread %i with tid %u created\n", i,
		       //(unsigned int) threads[i]);
    }
    for ( int i = 0; i < N; i++ ) 
    {
        pthread_join(threads[i], NULL);
        //printf("thread %i with tid %u joined\n", i,
		       //(unsigned int) threads[i]);
    }
    for ( int i = 0; i < N; i++ ) 
    {
        checkThreads(&parent, headPointer[i]);
        //printf("thread %i with tid %u checked\n", i,
		       //(unsigned int) threads[i]);
    }
    selectionSort(parent);
    printOutputFile(outputFileName,parent,K);
    return 0;

}
void* thread_func(void* arguments)
{
    struct thread_arg* args = (struct thread_arg*) arguments;
    char* fileName = (*args).filename;
    int threadCounter = (*args).threadCounter;
    FILE* file;
    char word[64];
    file = fopen(fileName, "r");
    headPointer[threadCounter] = NULL;

    while ( fscanf(file, "%s", word) == 1 ) 
    {

        char* current = strdup(word);
        
        for ( int i = 0; current[i] != '\0'; i++ ) 
        {
        
            if ( current[i] >= 'a' && current[i] <= 'z' ) 
            {
                current[i] = current[i] - 32;
            }
        }
        
        headPointer[threadCounter] = pushData(headPointer[threadCounter], current);
    }

    fclose(file);
    pthread_exit(0);
}



