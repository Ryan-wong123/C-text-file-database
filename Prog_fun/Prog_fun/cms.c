#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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
void saveToFile(const char* filename, HashMap* hashmap);
struct StudentRecords* QueryRecord(HashMap* hashmap, int id, bool printrecord);
void insertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark);
void updateStudentByID(HashMap* hashmap, int id, const char* newName, const char* newProgramme, float newMark, int nameFlag, int programmeFlag, int markFlag);
void parseAndExecuteUpdate(HashMap* hashmap, const char* input);
void TrimTrailingSpaces(char* str);
void OpenFile(const char* filename, HashMap* hashmap);
int SortbyID(const void* a, const void* b);
void ShowAll(HashMap* hashmap);
void DeleteRecord(HashMap* hashmap, int id);
void saveToFile(const char* filename, HashMap* hashmap);
char* GetField(const char* input, const char* key, int maxLength, const char* error_message);

void insertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark) {
    // Allocate memory for a new student record
    StudentRecords* newStudent = malloc(sizeof(StudentRecords));
    if (newStudent == NULL) {
        perror("Memory allocation failed for new student\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the new student record
    newStudent->id = id;
    strncpy(newStudent->name, name, sizeof(newStudent->name));
    strncpy(newStudent->programme, programme, sizeof(newStudent->programme));
    newStudent->mark = mark;
    newStudent->next = NULL;

    // Calculate the index using the hash function
    unsigned int index = hash(id);

    // Insert the new student at the head of the linked list at the index
    newStudent->next = hashmap->table[index];
    hashmap->table[index] = newStudent;

    // Increment the record count
    recordCount++;

    // Resize the hash map if the load factor exceeds 50%
    if (recordCount > (currentSize / 2)) {
        resizeHashMap(hashmap);
    }
}

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
            return;
        }
        current = current->next;
    }

    // If the record was not found
    printf("The record with ID=%d does not exist.\n", id);
}
void UpdateUser(HashMap* hashmap, const char* input) {
    int id = atoi(GetField(input, "ID=", sizeof(input), "Invalid Command. Usage: UPDATE ID=<id> Name=<name> Programme=<programme> Mark=<mark>\n"));
    if (id == 0) return;  // If ID is invalid, exit

    char newName[STUDENT_NAME_LENGTH] = "", newProgramme[PROGRAMME_LENGTH] = "";
    float newMark = -1;

    // Get fields, update flags as needed
    char* currentName = GetField(input, "Name=", sizeof(newName), "Invalid Command. Usage: UPDATE ID=<id> Name=<name> Programme=<programme> Mark=<mark>\n");
    if (currentName) strncpy(newName, currentName, STUDENT_NAME_LENGTH - 1);

    char* currentProgramme = GetField(input, "Programme=", sizeof(newProgramme), "Invalid Command. Usage: UPDATE ID=<id> Name=<name> Programme=<programme> Mark=<mark>\n");
    if (currentProgramme) strncpy(newProgramme, currentProgramme, PROGRAMME_LENGTH - 1);

    char* currentMark = GetField(input, "Mark=", sizeof(input), "Invalid Command. Usage: UPDATE ID=<id> Name=<name> Programme=<programme> Mark=<mark>\n");
    if (currentMark) newMark = atof(currentMark);

    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];

    while (current != NULL) {
        if (current->id == id) {
            // Update fields if present
            if (*newName) {
                strncpy(current->name, newName, STUDENT_NAME_LENGTH - 1);
                current->name[STUDENT_NAME_LENGTH - 1] = '\0';
                printf("Name updated to: %s\n", current->name);
            }

            if (*newProgramme) {
                strncpy(current->programme, newProgramme, PROGRAMME_LENGTH - 1);
                current->programme[PROGRAMME_LENGTH - 1] = '\0';
                printf("Programme updated to: %s\n", current->programme);
            }

            if (newMark != -1) {
                current->mark = newMark;
                printf("Mark updated to: %.2f\n", current->mark);
            }

            printf("\nThe record with ID=%d is successfully updated.\n", id);
            return;
        }
        current = current->next;
    }

    printf("The record with ID=%d does not exist.\n", id);
}


void resizeHashMap(HashMap* oldHashMap) {
    int i, j, newSize ;
    for (i = currentSize * 2 + 1;; i++) {
        for (j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                break;
            }
        }
        if (j * j > i) {
            newSize = i;  
        }
    }

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

void TrimTrailingSpaces(char* str) {
    // Trim trailing spaces
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
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
            sscanf(line + 12, "%[^\n]", tableName);
            continue;
        }

        if (isdigit(line[0])) {
            int id;
            char name[STUDENT_NAME_LENGTH], programme[PROGRAMME_LENGTH];
            float mark;
            int fields = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", &id, name, programme, &mark);
            if (fields == 4) {// Check if the student with the given ID already exists
                StudentRecords* existingStudent = QueryRecord(hashmap, id, false);
                if (existingStudent != NULL) {
                    // Update the existing record
                    snprintf(existingStudent->name, STUDENT_NAME_LENGTH, "%s", name);
                    snprintf(existingStudent->programme, PROGRAMME_LENGTH, "%s", programme);
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

    fclose(file);
}

// Comparison function for sorting by ID
int SortbyID(const void* a, const void* b) {
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
    qsort(allRecords, recordCount, sizeof(StudentRecords*), SortbyID);

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

struct StudentRecords* QueryRecord(HashMap* hashmap, int id, bool printrecord) {
    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];
    while (current != NULL) {
        if (current->id == id) {

            if (printrecord == true) {
                printf("%s: The record with ID=%d is found in the data table.\n", USERNAME, id);
                printf("ID        Name                  Programme                  Mark\n");
                printf("%-8d  %-20s  %-25s  %.2f\n",
                    current->id, current->name, current->programme, current->mark);
            }
            return current;
        }
        current = current->next;
    }
    if (printrecord == true) {
        printf("%s: The record with ID=%d does not exist.\n", USERNAME, id);
    }
   
    return NULL;
}


void DeleteRecord(HashMap* hashmap, int id) {
    int id_check_flag = 0;
    char check_delete[3];


    unsigned int index = hash(id);

    StudentRecords* current = hashmap->table[index];
    StudentRecords* prev = NULL;

    // loop through bucket if not the same ID
    while (current != NULL) {
        if (current->id == id) {
            id_check_flag = 1;
            break;
        }
        else {
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

            // if current is a head of bucket set ptr to next record
            if (prev == NULL) {
                hashmap->table[index] = current->next;
            }
            else {
                prev->next = current->next;
            }
            printf("%s: The record with ID=%d is successfully deleted.\n", USERNAME, id);
            recordCount--;
            free(current);
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

    printf("%s: The database file \"%s\" has been successfully updated.\n", USERNAME, FILE_PATH);
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
    3. DEVIN TAN ZHEN WEI \t(2400649)\n \
    4. TNG ZHENG YANG \t\t(2401113)\n \
    5. SABIHAH AMIRUDEEN \t(2401670)\n \
    6. WONG HOI YOUNG, RYAN \t(2401725)\n \
    Date: (please insert the date when you submit your group project)\n" };

    // print declaration    
    puts(declare);

}
char* GetField(const char* input, const char* key, int maxLength, const char* error_message) {
    const char* start = strstr(input, key);  // Find the key
    if (!start) {
        printf("%s", error_message);
        return NULL;
    }

    start += strlen(key);  // Move past the key (e.g., "Name=")

    // Now look for the next key (e.g., "ID=", "Name=", "Programme=", "Mark=") or the end of the string
    const char* nextKey = strpbrk(start, "I");  // Look for characters in next key 'ID=', 'Name=', etc.
    if (!nextKey) {
        nextKey = input + strlen(input);  // If no next key, go to the end of the string
    }

    // Find the boundary (either next key or end of string)
    const char* end = strstr(start, "ID=");
    if (!end) end = strstr(start, "Name=");
    if (!end) end = strstr(start, "Programme=");
    if (!end) end = strstr(start, "Mark=");
    if (!end) end = input + strlen(input);  // No next keyword, go to end

    int length = end - start;
    if (length > maxLength - 1) length = maxLength - 1;  // Ensure within bounds

    // Allocate memory for the output value and copy the substring
    char* output = malloc(length + 1);
    if (!output) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    strncpy(output, start, length);
    output[length] = '\0';  // Null-terminate the string

    return output;
}


int isFileOpened = 0; // to check for db open status

int main() {
    char idBuffer[10] = "";

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

    DisplayDeclaration();

    while (1) {
        printf("%s:", GROUP_NAME);
        char input[256];
        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;
        TrimTrailingSpaces(input);

        if ((_strnicmp(input, "show all", 8) == 0 || _strnicmp(input, "UPDATE ID=", 10) == 0 || _strnicmp(input, "delete", 6) == 0 || _strnicmp(input, "query", 5) == 0 || _strnicmp(input, "INSERT ID =",10) == 0) && isFileOpened == 0) {
            OpenFile(FILE_PATH, hashmap);
            isFileOpened = 1;
        }

        if (_stricmp(input, "open") == 0) {
            if (isFileOpened == 1) {
                printf("Database file is open already.\n");
                continue;
            }

            OpenFile(FILE_PATH, hashmap);
            isFileOpened = 1;
            printf("%s: The database file \"%s\" is successfully opened.\n", USERNAME, FILE_PATH);
        }
        else if (_stricmp(input, "show all") == 0) {
            ShowAll(hashmap);
        }
        else if (_strnicmp(input, "UPDATE ID=", 10) == 0) {
            UpdateUser(hashmap, input);
        }
        else if (_strnicmp(input, "delete", 6) == 0) {
            //int id = GetId(input, "Invalid Command. Usage: DELETE ID=<id>\n");

            char* value = GetField(input, "ID=",sizeof(input), "Invalid Command. Usage: DELETE ID=<id>\n");
            int id = atoi(value);

            if (id == 0) {
                continue;
            }

            DeleteRecord(hashmap, id);
        }
        else if (_strnicmp(input, "query", 5) == 0) {
            char* value = GetField(input, "ID=", sizeof(input), "Invalid Command. Usage: QUERY ID=<id>\n");
            int id = atoi(value);

            if (id == 0) {
                continue;
            }

            QueryRecord(hashmap, id ,true);
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
            TrimTrailingSpaces(name);
            TrimTrailingSpaces(programme);

            if (fields == 4) {
                // Check if the student with this ID already exists
                StudentRecords* existingStudent = QueryRecord(hashmap, id, false);
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
        else if (_stricmp(input, "exit") == 0) {
            break;
        }
        else if (_stricmp(input, "SAVE") == 0) {
            saveToFile(FILE_PATH, hashmap);
        }
        else {
            printf("Invalid Command. \n");
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
        free(hashmap->table); // free the pointers of head node in each LL 
        free(hashmap);        // free the memory of the overall hash map

        return 0;
    }