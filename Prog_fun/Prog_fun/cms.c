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


StudentRecords* findStudentByID(HashMap* hashmap, int id) {
    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];
    while (current != NULL) {
        if (current->id == id) {
            return current;  // Record found
        }
        current = current->next;
    }
    return NULL;  // Record not found
}


void saveToFile(const char* filename, HashMap* hashmap);

// Save changes to file
void saveToFile(const char* filename, HashMap* hashmap) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    // Write header information
    fprintf(file, "Database Name: %s\n", USERNAME);
    fprintf(file, "Authors: Ryan Wong, Zheng Yang, Sabihah, Devin, Timothy, Naveen\n\n");
    fprintf(file, "Table Name: %s\n", tableName);
    fprintf(file, "ID\tName\tProgramme\tMark\n");

    // Iterate through each bucket and write records
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            fprintf(file, "%d\t%s\t%s\t%.2f\n", current->id, current->name, current->programme, current->mark);
            current = current->next;
        }
    }

    // printf("%s: The database file \"%s\" has been successfully updated.\n", USERNAME, FILE_PATH);
    fclose(file);
}
// Function to update a student record by ID
void updateStudentByID(HashMap* hashmap, int id) {
    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];

    // Traverse the linked list to find the record
    while (current != NULL) {
        if (current->id == id) {  // Found the record
            // Prompt the user for new values
            printf("Enter new name (current: %s): ", current->name);
            char newName[STUDENT_NAME_LENGTH];
            fgets(newName, sizeof(newName), stdin);
            newName[strcspn(newName, "\n")] = 0;  // Remove newline character
            if (strlen(newName) > 0) {  // Only update if input is not empty
                strncpy(current->name, newName, STUDENT_NAME_LENGTH - 1);
                current->name[STUDENT_NAME_LENGTH - 1] = '\0';
            }

            printf("Enter new programme (current: %s): ", current->programme);
            char newProgramme[PROGRAMME_LENGTH];
            fgets(newProgramme, sizeof(newProgramme), stdin);
            newProgramme[strcspn(newProgramme, "\n")] = 0;  // Remove newline character
            if (strlen(newProgramme) > 0) {  // Only update if input is not empty
                strncpy(current->programme, newProgramme, PROGRAMME_LENGTH - 1);
                current->programme[PROGRAMME_LENGTH - 1] = '\0';
            }

            printf("Enter new mark (current: %.2f): ", current->mark);
            char newMarkStr[10];
            fgets(newMarkStr, sizeof(newMarkStr), stdin);
            newMarkStr[strcspn(newMarkStr, "\n")] = 0;  // Remove newline character
            if (strlen(newMarkStr) > 0) {  // Only update if input is not empty
                float newMark = atof(newMarkStr);  // Convert string to float
                current->mark = newMark;
            }

            printf("\nThe record with ID=%d is successfully updated.\n", id);
            saveToFile(FILE_PATH, hashmap);
            return;
        }
        current = current->next;
    }

    // If the record was not found
    printf("The record with ID=%d does not exist.\n", id);
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

void clearHashMap(HashMap* hashmap) {
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            StudentRecords* temp = current;
            current = current->next;
            free(temp);
        }
        hashmap->table[i] = NULL;
    }
    recordCount = 0;
}

void OpenFile(const char* filename, HashMap* hashmap) {
   //d clearHashMap(hashmap); 

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
            char name[STUDENT_NAME_LENGTH], programme[PROGRAMME_LENGTH];
            float mark;
            int fields = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", &id, name, programme, &mark);
            if (fields == 4) {// Check if the student with the given ID already exists
                StudentRecords* existingStudent = findStudentByID(hashmap, id);
                if (existingStudent != NULL) {
                    // Update the existing record
                    strncpy(existingStudent->name, name, STUDENT_NAME_LENGTH - 1);
                    existingStudent->name[STUDENT_NAME_LENGTH - 1] = '\0';
                    strncpy(existingStudent->programme, programme, PROGRAMME_LENGTH - 1);
                    existingStudent->programme[PROGRAMME_LENGTH - 1] = '\0';
                    existingStudent->mark = mark;
                } 
                else {
                    // Insert a new record if no duplicate exists
                    insertStudent(hashmap, id, name, programme, mark);
                }

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
    char check_delete[3];


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
       
        check_delete[strcspn(fgets(check_delete, sizeof(check_delete), stdin), "\n")] = 0;

        if (_strnicmp(check_delete, "Y", 1) == 0) {
            

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
        else if (_strnicmp(check_delete, "Y", 1) == 0) {
            printf("The deletion is cancelled.\n");
            break;

        }
        else {
            printf("Invalid Command\n");
        }

    }





    if (DEBUG_MODE == 1)printf("end delete section\n");

}



void DisplayDeclaration() {
    
char declare[1400] = {
"\t\t\t\t\tDeclaration \n \
SIT's policy on copying does not allow the students to copy source code as well as assessment solutions\n \
from another person or other places.It is the students' responsibility to guarantee that their assessment\n \
solutions are their own work.Meanwhile, the students must also ensure that their work is not accessible\n \
by others.Where such plagiarism is detected, both of the assessments involved will receive ZERO mark.\n\n \
We hereby declare that:\n \
We fully understand and agree to the abovementioned plagiarism policy.\n \
We did not copy any code from others or from other places.\n \
We did not share our codes with others or upload to any other places for public access and will\n \
not do that in the future.\n \
We agree that our project will receive Zero mark if there is any plagiarism detected.\n \
We agree that we will not disclose any information or material of the group project to others or\n \
upload to any other places for public access.\n\n \
Declared by: Group 2-3\n \
Team members:\n \
1. NAVEEN GOPALKRISHNAN \t(2402612)\n \
2. LEE ZHI HONG TIMOTHY \t(2400592)\n \
3. DEVIN TAN ZHEN WEI \t\t(2400649)\n \
4. TNG ZHENG YANG \t\t(2401113)\n \
5. SABIHAH AMIRUDEEN \t\t(2401670)\n \
6. WONG HOI YOUNG, RYAN \t(2401725)\n \
Date: (please insert the date when you submit your group project)\n" };

// print declaration    
puts(declare);



}




int open_flag = 0; // to check for db open status



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
    
    // print declaration uncomment when sending
    DisplayDeclaration();
    while (1) {
        printf("Enter an operation\n");
        char input[256];
        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;
       
        if (_stricmp(input, "open") == 0) {
            OpenFile(FILE_PATH, hashmap);
            open_flag = 1;
        }
        else if (_stricmp(input, "show all") == 0) {
            if (open_flag == 0) {
                printf("Database file not open yet.\n");
                continue;
            }

            ShowAll(hashmap);
        }

        else if (_stricmp(input, "update") == 0) {

            if (open_flag == 0) {
                printf("Database file not open yet.\n");
                    continue;
            }

            printf("UPDATE ID=");
            int id;
            scanf("%d", &id);
            getchar();  // Consume the newline character left by scanf
            updateStudentByID(hashmap, id);
        }
        // str n i cmp to check the front command
        else if (_strnicmp(input, "delete" , 6) == 0) {

            if (open_flag == 0) {
                printf("Database file not open yet.\n");
                    continue;
            }

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
            printf("Invalid Command. \n");
            if (DEBUG_MODE == 1)printf("%s", input);
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
