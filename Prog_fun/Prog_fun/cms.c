#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define DEBUG_MODE 0
#define FILE_PATH "database.txt"
#define USERNAME "CMS"
#define GROUP_NAME "P1_3"
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

// Function to update a student record by ID
void updateStudentByID(HashMap* hashmap, int id, const char* newName, const char* newProgramme, float newMark, int nameFlag, int programmeFlag, int markFlag) {
    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];

    // Traverse the linked list to find the record
    while (current != NULL) {
        if (current->id == id) {  // Found the record
            // Prompt the user for new values
            if (nameFlag && newName != NULL) {
                strncpy(current->name, newName, STUDENT_NAME_LENGTH - 1);
                current->name[STUDENT_NAME_LENGTH - 1] = '\0';
                printf("Name updated to: %s\n", current->name);
            }

            if (programmeFlag && newProgramme != NULL) {
                strncpy(current->programme, newProgramme, PROGRAMME_LENGTH - 1);
                current->programme[PROGRAMME_LENGTH - 1] = '\0';
                printf("Programme updated to: %s\n", current->programme);
            }

            if (markFlag) {
                current->mark = newMark;
                printf("Mark updated to: %.2f\n", current->mark);
            }

            printf("\nThe record with ID=%d is successfully updated.\n", id);
            saveToFile(FILE_PATH, hashmap);  // Save changes automatically
            return;
        }
        current = current->next;
    }

    // If the record was not found
    printf("The record with ID=%d does not exist.\n", id);
}

void parseAndExecuteUpdate(HashMap* hashmap, const char* input) {
    int id;
    char newName[STUDENT_NAME_LENGTH] = "";
    char newProgramme[PROGRAMME_LENGTH] = "";
    float newMark = -1;
    int nameFlag = 0, programmeFlag = 0, markFlag = 0;

    // Extract the ID first
    if (sscanf(input, "UPDATE ID=%d", &id) != 1) {
        printf("Invalid command format. Expected format: 'UPDATE ID=<id> <Field>=<Value>'\n");
        return;
    }

    // Find the start of the fields section after "UPDATE ID=<id>"
    const char* fieldsStart = strstr(input, "ID=");
    fieldsStart = strchr(fieldsStart, ' '); // Move past "ID=<id>" to fields
    if (!fieldsStart) {
        printf("No fields to update.\n");
        return;
    }
    fieldsStart++; // Move to the first character after the space

    // Buffer to hold the rest of the input fields
    char remainingFields[256];
    strncpy(remainingFields, fieldsStart, sizeof(remainingFields) - 1);
    remainingFields[sizeof(remainingFields) - 1] = '\0';

    // Parse each field
    char *valueStart;
    if ((valueStart = strstr(remainingFields, "Name="))) {
        valueStart += 5; // Move past "Name="
        char *nextField = strstr(valueStart, " Programme=");
        if (!nextField) nextField = strstr(valueStart, " Mark=");
        if (nextField) *nextField = '\0'; // Temporarily end the string here
        strncpy(newName, valueStart, STUDENT_NAME_LENGTH - 1);
        newName[STUDENT_NAME_LENGTH - 1] = '\0';
        nameFlag = 1;
        if (nextField) *nextField = ' '; // Restore the space
    }
    if ((valueStart = strstr(remainingFields, "Programme="))) {
        valueStart += 10; // Move past "Programme="
        char *nextField = strstr(valueStart, " Mark=");
        if (nextField) *nextField = '\0';
        strncpy(newProgramme, valueStart, PROGRAMME_LENGTH - 1);
        newProgramme[PROGRAMME_LENGTH - 1] = '\0';
        programmeFlag = 1;
        if (nextField) *nextField = ' ';
    }
    if ((valueStart = strstr(remainingFields, "Mark="))) {
        valueStart += 5; // Move past "Mark="
        newMark = atof(valueStart); // Convert string to float
        markFlag = 1;
    }

    // Call the update function with flags indicating which fields to update
    updateStudentByID(hashmap, id, newName, newProgramme, newMark, nameFlag, programmeFlag, markFlag);
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

void trimTrailingSpaces(char* str) {
    // Trim trailing spaces
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }

    // Trim leading spaces
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // Shift the string back to remove leading spaces
    if (start != str) {
        memmove(str, start, strlen(start) + 1);  // +1 to include null terminator
    }
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

// Comparison function for sorting by ID
int compareByID(const void* a, const void* b) {
    StudentRecords* studentA = *(StudentRecords**)a;
    StudentRecords* studentB = *(StudentRecords**)b;
    return studentA->id - studentB->id;  // Sort in ascending order of IDs
}

void ShowAll(HashMap* hashmap) {
    if (recordCount == 0) {
        printf("No records to display.\n");
        return;
    }

    // Dynamically allocate memory for an array of pointers to StudentRecords
    StudentRecords** allRecords = malloc(recordCount * sizeof(StudentRecords*));
    if (allRecords == NULL) {
        fprintf(stderr, "Memory allocation failed for allRecords.\n");
        return;
    }

    int index = 0;

    // Traverse through all buckets in the hash map and collect all records
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            allRecords[index++] = current;  // Add each student to the array
            current = current->next;
        }
    }

    // Sort the array of records by ID
    qsort(allRecords, recordCount, sizeof(StudentRecords*), compareByID);

    // Print the records in the desired format
    printf("%s: Here are all the records found in the table \"%s\".\n", USERNAME, tableName);
    printf("ID        Name                  Programme                  Mark\n");

    // Print each student in the sorted order
    for (int i = 0; i < recordCount; i++) {
        StudentRecords* student = allRecords[i];
        printf("%-8d  %-20s  %-25s  %.2f\n",
            student->id, student->name, student->programme, student->mark);
    }

    // Free the dynamically allocated memory
    free(allRecords);
}




struct StudentRecords* QueryRecord(HashMap* hashmap, int id) {
    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];
    while (current != NULL) {
        if (current->id == id) {
            printf("%s: The record with ID=%d is found in the data table.\n", USERNAME, id);
            printf("ID        Name                  Programme                  Mark\n");
            printf("%-8d  %-20s  %-25s  %.2f\n",
                current->id, current->name, current->programme, current->mark);
            return current;
        }
        current = current->next;
    }

    printf("%s: The record with ID=%d does not exist.\n",USERNAME, id);
    return NULL;
}


void DeleteRecord(HashMap* hashmap, int id) {
    int id_check_flag = 0;
    char check_delete[3];

    if (DEBUG_MODE == 1) printf("enter delete section\n");
    unsigned int index = hash(id);

    StudentRecords* current = hashmap->table[index];
    StudentRecords* prev = NULL;

    // loop through bucket if not the same ID
    while (current != NULL) {
        if (current->id == id) {
            if (DEBUG_MODE == 1) printf("ID: %d hash index:%d \n", current->id, index);
            id_check_flag = 1;
            break;
        }
        else {
            if (DEBUG_MODE == 1) printf(" not ID: %d hash index:%d \n", current->id, index);
            prev = current;
            current = current->next;
        }
    }

    if (id_check_flag == 0) {
        printf("%s: The record with ID=%d does not exist \n",USERNAME, id);
        return;
    }

    while (1) {
        printf("%s: Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel\n",USERNAME, id);

        printf("%s:", GROUP_NAME);
        check_delete[strcspn(fgets(check_delete, sizeof(check_delete), stdin), "\n")] = 0;

        if (_strnicmp(check_delete, "Y", 1) == 0) {
            if (DEBUG_MODE == 1) printf("The deletion is processing.\n");

            // if current is a head of bucket set ptr to next record
            if (prev == NULL) {
                hashmap->table[index] = current->next;
            }
            else {
                prev->next = current->next;
            }
            printf("%s: The record with ID=%d is successfully deleted.\n", USERNAME, id);
            recordCount--;
            break;
        }
        else if (_strnicmp(check_delete, "N", 1) == 0) {
            printf("%s: The deletion is cancelled.\n", USERNAME);
            break;
        }
        else {
            printf("Invalid Command\n");
        }
    }
    if (DEBUG_MODE == 1) printf("end delete section\n");
}






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
        printf("%s:", GROUP_NAME);
        char input[256];
        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;
        trimTrailingSpaces(input);

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


        //else if (_stricmp(input, "update") == 0) {
        //    if (open_flag == 0) {
        //        printf("Database file not open yet.\n");
        //        continue;
        //    }
        //    printf("UPDATE ID=");
        //    int id;
        //    scanf("%d", &id);
        //    getchar();  // Consume the newline character left by scanf
        //    updateStudentByID(hashmap, id);
        //}
        else if (_strnicmp(input, "UPDATE ID=", 10) == 0) {
            parseAndExecuteUpdate(hashmap, input);
              // Process the UPDATE command

        }
       
        else if (_strnicmp(input, "delete", 6) == 0) {
            if (open_flag == 0) {
                printf("Database file not open yet.\n");
                continue;
            }
            char* id_ptr;
            char s_id[10] = {'\0'};
            int id = 0;
            int letter_count = 0;

            // Find ID location and place ptr 
            id_ptr = strstr(input, "ID=");

            // Check command is entered correctly 
            if (id_ptr == NULL) {
                printf("Invalid Command. Usage: DELETE ID=<id>\n");
                continue;
            }
            // Extract ID from input
            for (int i = 3; id_ptr[i] != '\0' && letter_count < 10; i++) {
                s_id[i - 3] = id_ptr[i];
                letter_count++;
            }
            id = atoi(s_id);

            if (DEBUG_MODE == 1) printf("%d \n", id);

            // Check if ID entered correctly 
            if (id == 0) {
                printf("Invalid Command. Usage: DELETE ID=<id>\n");
                continue;
            }

            DeleteRecord(hashmap, id);

        }
        else if (_strnicmp(input, "query", 5) == 0) {
            char* id_ptr;
            char s_id[10]={'\0'};
            int id = 0;
            int letter_count = 0;

            // Find the ID location and place pointer
            id_ptr = strstr(input, "ID=");
            if (id_ptr == NULL) {
                printf("Invalid Command. Usage: QUERY ID=<id>\n");
                continue;
            }

            // Extract ID from input
            for (int i = 3; id_ptr[i] != '\0' && letter_count < 10; i++) {
                s_id[i - 3] = id_ptr[i];
                letter_count++;
            }
            id = atoi(s_id);

            if (id == 0) {
                printf("Invalid Command. Usage: QUERY ID=<id>\n");
                continue;
            }

            QueryRecord(hashmap, id);
        }
        else if (strncmp(input, "INSERT ID=", 10) == 0) {
            int id;
            char name[STUDENT_NAME_LENGTH];
            char programme[PROGRAMME_LENGTH];
            float mark;

            // Adjust pointer to skip "INSERT " and move to the actual parameters
            char* params = input + 7;  // Move past "INSERT "

            // Ensure all strings are cleared before parsing
            memset(name, 0, sizeof(name));
            memset(programme, 0, sizeof(programme));

            // Parse the input using `sscanf()`, ensuring to capture spaces properly
            int fields = sscanf(params, "ID=%d Name=%29[^P] Programme=%29[^M] Mark=%f", &id, name, programme, &mark);

            // Remove trailing spaces from name and programme
            trimTrailingSpaces(name);
            trimTrailingSpaces(programme);

            if (fields == 4) {
                // Check if the student with this ID already exists
                StudentRecords* existingStudent = findStudentByID(hashmap, id);
                if (existingStudent != NULL) {
                    printf("The record with ID=%d already exists.\n", id);
                }
                else {
                    // If student does not exist, insert the new student
                    insertStudent(hashmap, id, name, programme, mark);
                    printf("%s: A new record with ID=%d is successfully inserted.\n", USERNAME, id);
                }
            }
            else {
                printf("Invalid input. Please provide fields in the format: INSERT ID=ID Name=Name Programme=Programme Mark=Mark.\n");
            }
        }


        else if (_strnicmp(input, "delete", 6) == 0) {
            char* id_ptr;

            char s_id[10];
            int id = 0;
            int letter_count = 0;

            // Find ID location and place ptr 
            id_ptr = strstr(input, "ID=");

            // Check command is entered correctly 
            if (id_ptr == NULL) {
                printf("Invalid Command. Usage: DELETE ID=<id>\n");
                continue;
            }
            // Extract ID from input
            for (int i = 3; id_ptr[i] != '\0' && letter_count < 10; i++) {
                s_id[i - 3] = id_ptr[i];
                letter_count++;
            }
            id = atoi(s_id);

            if (DEBUG_MODE == 1) printf("%d \n", id);

            // Check if ID entered correctly 
            if (id == 0) {
                printf("Invalid Command. Usage: DELETE ID=<id>\n");
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

