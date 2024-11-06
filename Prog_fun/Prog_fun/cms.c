#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define DEBUG_MODE 0
#define FILE_PATH "database.txt"
#define USERNAME "CMS"
#define TABLE_NAME_LENGTH 15
#define INITIAL_HASHMAP_SIZE 101 // Initial size, will dynamically grow
#define STUDENT_NAME_LENGTH 30
#define PROGRAMME_LENGTH 30

char tableName[TABLE_NAME_LENGTH] = "";
int currentSize = INITIAL_HASHMAP_SIZE;  // Dynamic size of the hash map
int recordCount = 0;  // Track the number of records

typedef struct StudentRecords {
    int id;
    char name[STUDENT_NAME_LENGTH];
    char programme[PROGRAMME_LENGTH];
    float mark;
    struct StudentRecords* next; // Linked list for collision handling
} StudentRecords;

typedef struct HashMap {
    StudentRecords** table; // Dynamic array of pointers to StudentRecords
} HashMap;

unsigned int hash(int id) {
    return (id * 97) % currentSize; // Use a prime number multiplier for better distribution
}

void resizeHashMap(HashMap* oldHashMap);

StudentRecords* createStudent(int id, const char* name, const char* programme, float mark) {
    StudentRecords* newStudent = malloc(sizeof(StudentRecords));
    if (newStudent == NULL) {
        fprintf(stderr, "Memory allocation failed for new student\n");
        exit(EXIT_FAILURE);
    }
    newStudent->id = id;
    strncpy_s(newStudent->name, sizeof(newStudent->name), name, _TRUNCATE);
    strncpy_s(newStudent->programme, sizeof(newStudent->programme), programme, _TRUNCATE);
    newStudent->mark = mark;
    newStudent->next = NULL;
    return newStudent;
}

void insertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark) {
    unsigned int index = hash(id);  // Hash function with current size
    StudentRecords* newStudent = createStudent(id, name, programme, mark);

    // Insert at the head of the linked list (collision handling)
    newStudent->next = hashmap->table[index];
    hashmap->table[index] = newStudent;

    recordCount++;

    // If load factor exceeds 50%, resize the hash map
    if (recordCount > (currentSize / 2)) {
        resizeHashMap(hashmap);
    }
}

// Function to find the next prime number larger than a given number
int nextPrime(int n) {
    int i, j;
    for (i = n + 1;; i++) {
        for (j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                break;
            }
        }
        if (j * j > i) {
            return i;  // Return the next prime number
        }
    }
}
void resizeHashMap(HashMap* oldHashMap) {
    int newSize = nextPrime(currentSize * 2); // Find the next prime size

    HashMap* newHashMap = malloc(sizeof(HashMap));
    if (newHashMap == NULL) {
        fprintf(stderr, "Memory allocation failed during resizing\n");
        exit(EXIT_FAILURE);
    }
    newHashMap->table = malloc(newSize * sizeof(StudentRecords*));
    if (newHashMap->table == NULL) {
        fprintf(stderr, "Memory allocation failed for new hash map table\n");
        exit(EXIT_FAILURE);
    }
    memset(newHashMap->table, 0, newSize * sizeof(StudentRecords*));

    // Rehash all records from the old hash map to the new one
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = oldHashMap->table[i];
        while (current != NULL) {
            // Manually calculate new index and reinsert records
            unsigned int newIndex = current->id % newSize;
            StudentRecords* nextRecord = current->next; // Save the next record

            // Insert into new hash map
            current->next = newHashMap->table[newIndex];
            newHashMap->table[newIndex] = current;

            current = nextRecord; // Move to the next record
        }
    }

    free(oldHashMap->table);  // Free the old table memory
    currentSize = newSize;    // Update global variable with new size

    // Copy the new hash map structure into the old hash map pointer
    *oldHashMap = *newHashMap;
    free(newHashMap);  // Free temporary new hash map structure
}

void OpenFile(const char* filename, HashMap* hashmap) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Database Name:", 14) == 0 ||
            strncmp(line, "Authors:", 8) == 0 ||
            strncmp(line, "ID", 2) == 0) {
            continue;
        }

        if (strncmp(line, "Table Name:", 11) == 0) {
            sscanf_s(line + 12, "%[^\n]", tableName, (unsigned)_countof(tableName));
            continue;
        }

        if (isdigit(line[0])) {
            int id;
            char name[30], programme[30];
            float mark;
            int fields = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", &id, name, programme, &mark);
            if (fields == 4) {
                insertStudent(hashmap, id, name, programme, mark);
            }
            else {
                fprintf(stderr, "Error parsing line: %s\n", line);
            }
        }
    }
    printf("%s: The database file \"%s\" is successfully opened.\n", USERNAME, FILE_PATH);
    fclose(file);
}

void ShowAll(HashMap* hashmap) {
    printf("%s: Here are all the records found in the table \"%s\".\n", USERNAME, tableName);
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        if (current != NULL) {
            printf("Bucket %d:\n", i); // Print bucket number
        }
        while (current != NULL) {
            printf("    ID: %d, Name: %s, Programme: %s, Mark: %.2f\n",
                current->id, current->name, current->programme, current->mark);
            current = current->next;
        }
    }
}











void DeleteRecord(HashMap* hashmap, int id) {
    int id_check_flag = 0;
    char check_delete;


    if (DEBUG_MODE == 1)printf("enter delete section\n");
    unsigned int index = hash(id);
    
    StudentRecords* current = hashmap->table[index];
    StudentRecords* prev = NULL;

    // loop through bucket if not the same ID
    while (current != NULL) {
        if (current->id == id) {
            if (DEBUG_MODE == 1)printf("ID: %d hash index:%d \n", current->id, index);
            id_check_flag = 1;
            break;
        }
        else {
            if (DEBUG_MODE == 1)printf(" not ID: %d hash index:%d \n", current->id, index);
            prev = current;
            current = current->next;
            
        }

    }
    
    if (id_check_flag == 0) {
        printf("The record with ID=%d does not exist \n", id);
        return;
    }
    
    while (1) {

        printf("Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel\n", id);
        
        scanf_s(" %c", &check_delete);
        if (check_delete == 'Y') {
            

            if(DEBUG_MODE ==1)printf("The deletion is processing.\n");
            if(DEBUG_MODE == 1)printf("%p \n", current);

            // if current is a head of bucket set ptr to next record
            if (prev == NULL) {
                hashmap->table[index] = current->next;

            }
            // if at middle skip this node by setting last node pointer to this node next pointer
            else {
                prev->next = current->next;

            }
            printf("The record with ID=%d is successfully deleted.\n", id);
            recordCount--;
            break;


        }
        else if (check_delete == 'N') {
            printf("The deletion is cancelled.\n");
            break;

        }
        else {
            printf("Invalid Command\n");
        }

    }





    if (DEBUG_MODE == 1)printf("end delete section\n");

}








int main() {
    HashMap* hashmap = malloc(sizeof(HashMap));
    if (hashmap == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    hashmap->table = malloc(currentSize * sizeof(StudentRecords*));
    if (hashmap->table == NULL) {
        fprintf(stderr, "Memory allocation failed for table\n");
        exit(EXIT_FAILURE);
    }
    memset(hashmap->table, 0, currentSize * sizeof(StudentRecords*)); // Initialize the hash map


    while (1) {
        printf("Enter an operation\n");
        char input[256];
        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;

        if (_stricmp(input, "open") == 0) {
            OpenFile(FILE_PATH, hashmap);
        }
        else if (_stricmp(input, "show all") == 0) {
            ShowAll(hashmap);
        }
        // str n i cmp to check the front command
        else if (_strnicmp(input, "delete" , 6) == 0) {
            char *id_ptr;
            char s_id[10];
            int id = 0;
            int letter_count = 0;

            // find ID location and place ptr 
            id_ptr = strstr(input, "ID=");

            //check command is enter corrently 
            if (id_ptr == NULL) {
                printf("Invalid Command. eg. DELETE ID=<id> \n");
                continue;
            }
            // for loop the ID portion into s_id
            for (int i = 3; id_ptr[i] != '\0' && letter_count < 10; i++) {
                
                s_id[i-3] = id_ptr[i];
                letter_count++;
            }
            id = atoi(s_id);
            
            if (DEBUG_MODE == 1)printf("%d \n", id);

            // check ID enter correctly 
            if (id == 0) {
                printf("Invalid Command. eg. DELETE ID=<id> \n");
                continue;
            }


            DeleteRecord(hashmap, id);
            
        }
        else if (_stricmp(input, "exit") == 0) {
            break;
        }
        else {
            printf("Invalid Command m. \n");
            printf("%s", input);
        }


    }
    // Freeing allocated memory
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            StudentRecords* temp = current;
            current = current->next;
            free(temp); // Free each student record
        }
    }
    free(hashmap->table); // Free hash map table
    free(hashmap);        // Free hash map structure

    return 0;
}
