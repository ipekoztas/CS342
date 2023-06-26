#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <mqueue.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>

#define MAX_FILES 100
#define MAX_FREQS 1000
#define MAX_WORD_LEN 64


// Ipek Oztas - Emre Karatas
// a data structure to keep words and counts together. This is a linked list application.

typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;

struct dataItem
{
    char* word;
    struct dataItem* next;
    struct dataItem* previous;
    int wordCount;
};

struct dataItem* head = NULL;
struct dataItem* tail = NULL;

struct dataItem* pushData(struct dataItem* head, char* word)
{
    // Check if the word already exists in the linked list
    struct dataItem* current = head;
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->wordCount++;
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

void readFiles(char* fileName)
{
	FILE* file;
	char word[64];
	file = fopen(fileName,"r");
	char* current;
	
	while ( fscanf(file, "%s", word) == 1 ) 
	{
        	current = strdup(word);
        	for ( int i = 0; current[i] != '\0'; i++ ) 
        	{
            		if ( current[i] >= 'a' && current[i] <= 'z' ) 
            		{
                		current[i] = current[i] - 32;
            		}
            	}
            	head = pushData(head, current);
		free(current); // free the allocated memory after using it
        }
    fclose(file);
}
void printData(struct dataItem* head, FILE* outputFile, int* count) {
    if (head == NULL || *count <= 0) {
        return;
    }
    fprintf(outputFile, "%s %d\n", head->word, head->wordCount);
    (*count)--;
    printData(head->next, outputFile, count);
}


/* Function to write output file */
void printOutputFile(char* outputFileName, int topKWords) {
    int count = topKWords;
    FILE* outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    selectionSort(head);
    printData(head, outputFile, &count);
    if (fclose(outputFile) != 0) {
        perror("Error closing output file");
        exit(EXIT_FAILURE);
    }
}

void printDataToSharedMem(struct dataItem* head, char *ptr, int* count) {
    if (head == NULL || *count <=0) {
        return;
    }
    //memcpy(ptr, s , sizeof(char)*strlen(s));
    sprintf(ptr, "%s %d\n", head->word, head->wordCount);
    (*count)--;
    printDataToSharedMem(head->next, ptr+256, count);
}

void printToSharedMem(char *ptr, int topKWords)
{
    int count = topKWords;
    selectionSort(head);
    printDataToSharedMem(head, ptr, &count);
}

int compare_freqs(const void *a, const void *b) {
    WordFreq *f1 = (WordFreq*) a;
    WordFreq *f2 = (WordFreq*) b;
    return f2->count - f1->count;
}


   void top_k_words(char* filename, char* outputFilename, int k) {
    // Open input file for reading
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Read file into frequency list
    WordFreq freqs[MAX_FREQS];
    int num_freqs = 0;
    char word[MAX_WORD_LEN];
    int count;
    int found;
    while (fscanf(fp, "%s %d", word, &count) == 2) {
        found = 0;
        for (int i = 0; i < num_freqs; i++) {
            if (strcmp(word, freqs[i].word) == 0) {
                freqs[i].count += count;
                found = 1;
                break;
            }
        }
        if (!found) {
            if (num_freqs == MAX_FREQS) {
                break;
            }
            strncpy(freqs[num_freqs].word, word, MAX_WORD_LEN);
            freqs[num_freqs].count = count;
            num_freqs++;
        }
    }
    fclose(fp);

    // Sort frequency list by count
    qsort(freqs, num_freqs, sizeof(WordFreq), compare_freqs);

    // Create result list of top-K words
    char **top_words = (char**) malloc(k * sizeof(char*));
    int *top_counts = (int*) malloc(k*sizeof(int));
    for (int i = 0; i < k; i++) {
        top_words[i] = (char*) malloc(MAX_WORD_LEN * sizeof(char));
        strncpy(top_words[i], freqs[i].word, MAX_WORD_LEN);
        top_counts[i] = freqs[i].count;
    }

    // Write result list to output file
    FILE *out_fp = fopen(outputFilename, "w");
    if (out_fp == NULL) {
        perror("Error opening output file");
        return;
    }
    for (int i = 0; i < k; i++) {
        fprintf(out_fp, "%s %d\n", top_words[i], top_counts[i]);
    }
    fclose(out_fp);

    // Free memory
    for (int i = 0; i < k; i++) {
        free(top_words[i]);
    }
    free(top_words);
    free(top_counts);
}


int main(int argc, char* argv[])
{
    int shmid;
    char *shmaddr;
    int i;
    const int SHM_SIZE = 4096;

    // Create shared memory
    if ((shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory
    if ((shmaddr = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    if ( argc < 5 ) {
        printf("You have entered insufficient number of arguments! Usage: proctopk <K> <outfile> <N> <infile1> .... <infileN>\n");
        return -1;
    }
    
    int topKWords = atoi(argv[1]);
    char* outfile = argv[2];
    int N= atoi(argv[3]);
    pid_t pid[N];
    char* fileNames[N];
    for ( int i = 0; i < N; i++ ) {
        fileNames[i] = argv[i + 4];
    }
    // Create child processes
    for (i = 0; i < N; i++) {
        pid[i] = fork();
        if (pid[i] == -1) {
            perror("fork");
            exit(1);
        } else if (pid[i] == 0) {
            // Child process writes to shared memory
            readFiles(fileNames[i]);
            printToSharedMem(shmaddr+(i*topKWords*256),topKWords);
            //sprintf(shmaddr+(i*256), "Child %d wrote to shared memory.", i+1);
            exit(0);
        }
    }

   
    //printOutputFile(outfile, topKWords);
    
    // Wait for child processes to complete
    for (i = 0; i < N; i++) {
        waitpid(pid[i], NULL, 0);
    }
    char* fileN;
    fileN = "tempFile.txt";
    FILE* temp = fopen(fileN, "w");
    // Print contents of shared memory
    for (int i = 0; i <topKWords*N; i++){
        //printf("%s\n", shmaddr+(i*256));
        fprintf(temp, "%s\n", shmaddr+(i*256));

    }
    fclose(temp);
    //combine information in shared memory
    top_k_words(fileN, outfile, topKWords);

    // Detach and remove shared memory
    shmdt(shmaddr);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
