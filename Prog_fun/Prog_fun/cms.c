#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 100 // Define a maximum number of students
#define FILE_PATH "database.txt"
#define USERNAME "CMS"
#define TABLE_NAME_LENGTH 30
#define HASHMAP_SIZE 101 // A prime number for better distribution

char tableName[TABLE_NAME_LENGTH] = ""; // Global tableName variable

typedef struct StudentRecords {
    int id;
    char name[30];
    char programme[30];
    float mark;
    struct StudentRecords* next; // Pointer for linked list
} StudentRecords;

typedef struct HashMap {
    StudentRecords* table[HASHMAP_SIZE]; // Array of pointers to StudentRecords
} HashMap;

// Hash function to compute the index based on student ID
unsigned int hash(int id) {
    return id % HASHMAP_SIZE;
}

// Create a new student record
StudentRecords* createStudent(int id, const char* name, const char* programme, float mark) {
    StudentRecords* newStudent = malloc(sizeof(StudentRecords));
    newStudent->id = id;
    // Correct usage of strncpy_s
    strncpy_s(newStudent->name, sizeof(newStudent->name), name, _TRUNCATE);
    strncpy_s(newStudent->programme, sizeof(newStudent->programme), programme, _TRUNCATE);
    newStudent->mark = mark;
    newStudent->next = NULL;
    return newStudent;
}

// Insert a student into the hashmap
void insertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark) {
    unsigned int index = hash(id);
    StudentRecords* newStudent = createStudent(id, name, programme, mark);

    // Insert at the head of the linked list
    newStudent->next = hashmap->table[index];
    hashmap->table[index] = newStudent;
}

// Function to free all students in the hashmap
void freeHashMap(HashMap* hashmap) {
    for (int i = 0; i < HASHMAP_SIZE; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            StudentRecords* toDelete = current;
            current = current->next;
            free(toDelete);
        }
    }
}

// Modified OpenFile function to use hashmap
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

// Show all student records in the hashmap
void ShowAll(HashMap* hashmap) {
    printf("%s: Here are all the records found in the table \"%s\".\n", USERNAME, tableName);
    for (int i = 0; i < HASHMAP_SIZE; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            printf("ID: %d, Name: %s, Programme: %s, Mark: %.2f\n",
                current->id, current->name, current->programme, current->mark);
            current = current->next;
        }
    }
}

int main() {
    HashMap* hashmap = malloc(sizeof(HashMap));
    if (hashmap == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(hashmap->table, 0, sizeof(hashmap->table)); // Initialize the hashmap to NULL

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
        else if (_stricmp(input, "exit") == 0) {
            break;
        }
    }

    freeHashMap(hashmap); // Free the hashmap
    free(hashmap); // Free the allocated memory for the hashmap
    return 0;
}
