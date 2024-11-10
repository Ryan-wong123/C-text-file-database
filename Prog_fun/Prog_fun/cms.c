#define _CRTDBG_MAP_ALLOC // allow debugging to find where is the cause of memory leak
#define _CRT_SECURE_NO_WARNINGS // suppress unnecassary warnings

#include <stdio.h> // library for input and output
#include <stdlib.h> // library for memory allocation
#include <string.h> // library for strcpy, strlen
#include <ctype.h> // library for islower and isspace
#include <stdbool.h> //library for boolean
#include <crtdbg.h> // library used to detecting memory leaks

#define FILE_PATH "database.txt" // path name for the txt file
#define USERNAME "CMS" // username to display in the console
#define GROUP_NAME "P1_3" // group name used to display in the console
#define TABLE_NAME_LENGTH 15
#define HASHMAP_LENGTH 1
#define NAME_LENGTH 30
#define PROGRAMME_LENGTH 30
#define GENERAL_LENGTH 70

char tableName[TABLE_NAME_LENGTH] = "";
int currentSize = HASHMAP_LENGTH;  
int recordCount = 0; 

typedef struct StudentRecords {
    int id;
    char name[NAME_LENGTH];
    char programme[PROGRAMME_LENGTH];
    float mark;
    struct StudentRecords* next;
} StudentRecords;

typedef struct HashMap {
    StudentRecords** table;
} HashMap;

unsigned int hash(int id) {
    return (id * 97) % currentSize;
}

void DisplayDeclaration();

void InsertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark);
void UpdateStudent(HashMap* hashmap, const char* input);
struct StudentRecords* QueryStudent(HashMap* hashmap, int id, bool printrecord);
void DeleteStudent(HashMap* hashmap, int id);

void OpenFile(const char* filename, HashMap* hashmap);
void ShowAll(HashMap* hashmap);
void saveToFile(const char* filename, HashMap* hashmap);

void resizeHashMap(HashMap* currentHashmap);
void TrimTrailingSpaces(char* str);
int SortbyID(const void* a, const void* b);
char* GetField(const char* input, const char* key, int maxLength);

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


    puts(declare);

}

void InsertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark) {
    
    StudentRecords* newStudent = malloc(sizeof(StudentRecords));
    if (newStudent == NULL) {
        perror("Memory allocation failed for new student\n");
        exit(EXIT_FAILURE);
    }
    
    newStudent->id = id;
    strncpy(newStudent->name, name, sizeof(newStudent->name) - 1), newStudent->name[sizeof(newStudent->name) - 1] = '\0';
    strncpy(newStudent->programme, programme, sizeof(newStudent->programme) - 1), newStudent->programme[sizeof(newStudent->programme) - 1] = '\0';
    newStudent->mark = mark;
    newStudent->next = NULL;

    unsigned int index = hash(id);

    newStudent->next = hashmap->table[index];
    hashmap->table[index] = newStudent;
    
    recordCount++;

    if (recordCount > (currentSize / 2)) {
        resizeHashMap(hashmap);
    }
}

void UpdateStudent(HashMap* hashmap, const char* input) {
    int id = atoi(GetField(input, "ID=", sizeof(input)));
    if (id == 0) {
        printf("Please enter valid ID.\n");
        return;
    }
       

    char newName[NAME_LENGTH] = "", newProgramme[PROGRAMME_LENGTH] = "";
    float newMark = -1;

   
    char* currentName = GetField(input, "Name=", sizeof(newName));
    if (currentName) {
        strncpy(newName, currentName, NAME_LENGTH - 1);
    }

    char* currentProgramme = GetField(input, "Programme=", sizeof(newProgramme));
    if (currentProgramme) {
        strncpy(newProgramme, currentProgramme, PROGRAMME_LENGTH - 1);
    }

    char* currentMark = GetField(input, "Mark=", sizeof(input));
    if (currentMark) {
        newMark = atof(currentMark);
    }

    unsigned int index = hash(id);
    StudentRecords* current = hashmap->table[index];

    while (current != NULL) {
        if (current->id == id) {
            
            if (*newName) {
                strncpy(current->name, newName, sizeof(current->name) - 1), current->name[sizeof(current->name) - 1] = '\0';
            }
            if (*newProgramme) {
                strncpy(current->programme, newProgramme, sizeof(current->programme) - 1), current->programme[sizeof(current->programme) - 1] = '\0';
            }
            if (newMark != -1) {
                current->mark = newMark;
            }

            printf("\nThe record with ID=%d is successfully updated.\n", id);
            return;
        }
        current = current->next;
    }

    printf("The record with ID=%d does not exist.\n", id);
}

struct StudentRecords* QueryStudent(HashMap* hashmap, int id, bool printrecord) {
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

void DeleteStudent(HashMap* hashmap, int id) {
    int id_check_flag = 0;
    char check_delete[3];


    unsigned int index = hash(id);

    StudentRecords* current = hashmap->table[index];
    StudentRecords* prev = NULL;


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
        printf("%s: The record with ID=%d does not exist \n", USERNAME, id);
        return;
    }

    while (1) {
        printf("%s: Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel\n", USERNAME, id);

        printf("%s:", GROUP_NAME);
        check_delete[strcspn(fgets(check_delete, sizeof(check_delete), stdin), "\n")] = 0;

        if (_strnicmp(check_delete, "Y", 1) == 0) {


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

void OpenFile(const char* filename, HashMap* hashmap) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char fileOutputLine[256];
    while (fgets(fileOutputLine, sizeof(fileOutputLine), file) != NULL) {
        if (strncmp(fileOutputLine, "Database Name:", 14) == 0 ||
            strncmp(fileOutputLine, "Authors:", 8) == 0 ||
            strncmp(fileOutputLine, "ID", 2) == 0) {
            continue;
        }

        if (strncmp(fileOutputLine, "Table Name:", 11) == 0) {
            sscanf(fileOutputLine + 12, "%[^\n]", tableName);
            continue;
        }

        if (isdigit(fileOutputLine[0])) {
            int id;
            char name[NAME_LENGTH], programme[PROGRAMME_LENGTH];
            float mark;
            int fields = sscanf(fileOutputLine, "%d\t%[^\t]\t%[^\t]\t%f", &id, name, programme, &mark);
            if (fields == 4) {
                StudentRecords* existingStudent = QueryStudent(hashmap, id, false);
                if (existingStudent != NULL) {

                    snprintf(existingStudent->name, NAME_LENGTH, "%s", name);
                    snprintf(existingStudent->programme, PROGRAMME_LENGTH, "%s", programme);
                    existingStudent->mark = mark;
                }
                else {
                    InsertStudent(hashmap, id, name, programme, mark);
                }
            }
            else {
                fprintf(stderr, "Error parsing fileOutputLine: %s\n", fileOutputLine);
            }
        }
    }

    fclose(file);
}

void ShowAll(HashMap* hashmap) {
    if (recordCount == 0) {
        printf("No records to display.\n");
        return;
    }


    StudentRecords** allRecords = malloc(recordCount * sizeof(StudentRecords*));
    if (allRecords == NULL) {
        fprintf(stderr, "Memory allocation failed for allRecords.\n");
        return;
    }

    int index = 0;


    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            allRecords[index++] = current;
            current = current->next;
        }
    }


    qsort(allRecords, recordCount, sizeof(StudentRecords*), SortbyID);

    printf("%s: Here are all the records found in the table \"%s\".\n", USERNAME, tableName);
    printf("ID        Name                  Programme                  Mark\n");

    for (int i = 0; i < recordCount; i++) {
        StudentRecords* student = allRecords[i];
        printf("%-8d  %-20s  %-25s  %.2f\n",
            student->id, student->name, student->programme, student->mark);
    }


    free(allRecords);
}

void saveToFile(const char* filename, HashMap* hashmap) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }


    fprintf(file, "Database Name: %s\n", USERNAME);
    fprintf(file, "Authors: Ryan Wong, Zheng Yang, Sabihah, Devin, Timothy, Naveen\n\n");
    fprintf(file, "Table Name: %s\n", tableName);
    fprintf(file, "ID\tName\tProgramme\tMark\n");


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

void resizeHashMap(HashMap* currentHashmap) {
    int newSize;

    // Find the next prime number greater than currentSize * 2
    for (newSize = currentSize * 2 + 1; ; newSize += 2) {  // Skip even numbers
        int isPrime = 1;
        if (newSize % 2 == 0) continue;  // Skip even numbers
        for (int j = 3; j * j <= newSize; j += 2) {  // Skip even divisors
            if (newSize % j == 0) {
                isPrime = 0;
                break;
            }
        }
        if (isPrime) break;
    }

    // Allocate memory for the new hash map
    HashMap* newHashMap = malloc(sizeof(HashMap));
    if (!newHashMap || !(newHashMap->table = calloc(newSize, sizeof(StudentRecords*)))) {
        fprintf(stderr, "Memory allocation failed during resizing\n");
        exit(EXIT_FAILURE);
    }

    // Rehash the old records into the new hash map
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = currentHashmap->table[i];
        while (current) {
            StudentRecords* nextRecord = current->next;
            unsigned int newIndex = current->id % newSize;

            current->next = newHashMap->table[newIndex];
            newHashMap->table[newIndex] = current;

            current = nextRecord;
        }
    }

    // Free the old hash map and update to the new one
    free(currentHashmap->table);
    *currentHashmap = *newHashMap;
    free(newHashMap);

    // Update the current size
    currentSize = newSize;
}

void TrimTrailingSpaces(char* str) {
    // Find the length of the string
    char* end = str + strlen(str) - 1;

    // Move the end pointer back to the last non-space character
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }

    // Set the new null terminator after the last non-space character
    *(end + 1) = '\0';

    // Find the first non-space character from the start
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // If there are leading spaces, shift the string to remove them
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int SortbyID(const void* a, const void* b) {
    StudentRecords* studentA = *(StudentRecords**)a;
    StudentRecords* studentB = *(StudentRecords**)b;
    return studentA->id - studentB->id;  
}

char* GetField(const char* input, const char* key, int maxLength) {
    static char desiredFieldOutput[GENERAL_LENGTH];  // Static buffer for reuse

    // Find the start of the key in the input string
    const char* start = strstr(input, key);
    if (!start) {
        return NULL;  // Key not found
    }

    start += strlen(key);  // Move past the key to the field value

    // Look for the next field or the end of the input
    const char* end = strstr(start, "ID=");
    if (!end) end = strstr(start, "Name=");
    if (!end) end = strstr(start, "Programme=");
    if (!end) end = strstr(start, "Mark=");
    if (!end) end = start + strlen(start);  // No other fields found, go to the end

    // Calculate the length of the field, making sure it doesn't exceed maxLength
    int length = end - start;
    if (length >= maxLength) {
        length = maxLength - 1;  // Ensure the value fits within the buffer
    }

    // Copy the field value into the static buffer
    strncpy(desiredFieldOutput, start, length)[length] = '\0';

    return desiredFieldOutput;  // Return the pointer to the static buffer
}

int main() {
    //check for memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    char idBuffer[10] = "";
    int isFileOpened = 0;

    HashMap* hashmap = malloc(sizeof(HashMap));

    if (hashmap == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    hashmap->table = calloc(currentSize, sizeof(StudentRecords*));
    if (!hashmap->table) {
        fprintf(stderr, "Memory allocation failed for table\n");
        exit(EXIT_FAILURE);
    }

    DisplayDeclaration();

    while (1) {
        printf("%s:", GROUP_NAME);
        char input[256];
        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;
        TrimTrailingSpaces(input);

        if ((_strnicmp(input, "show all", 8) == 0 || _strnicmp(input, "update", 6) == 0 || _strnicmp(input, "delete", 6) == 0 || _strnicmp(input, "query", 5) == 0 || _strnicmp(input, "INSERT ID=",10) == 0) && isFileOpened == 0) {
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
        else if (_strnicmp(input, "update", 6) == 0) {
            UpdateStudent(hashmap, input);
        }
        else if (_strnicmp(input, "delete", 6) == 0) {
            char* value = GetField(input, "ID=",sizeof(input));
            if (value == NULL) {
                printf("Invalid Command. Usage: DELETE ID=<id>\n");
                continue;
            }

            int id = atoi(value);

            if (id == 0) {
                printf("Please enter valid ID.\n");
                continue;
            }

            DeleteStudent(hashmap, id);
        }
        else if (_strnicmp(input, "query", 5) == 0) {
            char* value = GetField(input, "ID=", sizeof(input));
            if (value == NULL) {
                printf("Invalid Command. Usage: QUERY ID=<id>\n");
                continue;
            }
            int id = atoi(value);

            if (id == 0) {
                printf("Please enter valid ID.\n");
                continue;
            }

            QueryStudent(hashmap, id ,true);
        }
        else if (strncmp(input, "INSERT ID=", 10) == 0) {
            int id;
            char name[NAME_LENGTH];
            char programme[PROGRAMME_LENGTH];
            float mark;

     
            char* params = input + 7; 

         
            memset(name, 0, sizeof(name));
            memset(programme, 0, sizeof(programme));

        
            int fields = sscanf(params, "ID=%d Name=%29[^P] Programme=%29[^M] Mark=%f", &id, name, programme, &mark);

   
            TrimTrailingSpaces(name);
            TrimTrailingSpaces(programme);

            if (fields == 4) {

                StudentRecords* existingStudent = QueryStudent(hashmap, id, false);
                if (existingStudent != NULL) {
                    printf("The record with ID=%d already exists.\n", id);
                }
                else {
                    
                    InsertStudent(hashmap, id, name, programme, mark);
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

        
    for (int i = 0; i < currentSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            StudentRecords* temp = current;
            current = current->next;
            free(temp); 
        }
    }
    free(hashmap->table);
    free(hashmap);        

    return 0;
}