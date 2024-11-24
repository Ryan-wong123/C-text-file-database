
#define _CRTDBG_MAP_ALLOC //Allow debugging to find where is the cause of memory leak
#define _CRT_SECURE_NO_WARNINGS //Suppress unnecassary warnings

#include <stdio.h> //Library for input and output
#include <stdlib.h> //Library for memory allocation
#include <string.h> //Library for strcpy, strlen
#include <ctype.h> //Library for islower and isspace
#include <stdbool.h> //Library for boolean
#include <crtdbg.h> //Library used to detecting memory leaks
#include <math.h>



#define USERNAME "CMS" //Username to display in the console
#define GROUP_NAME "P2_3" //Group name used to display in the console


#define TEST_MODE 1
#define DEBUG_MODE 0

#if TEST_MODE == 1
#include "test.h"
#define FILE_PATH "testdb.txt"

#else 
#define FILE_PATH "P2_3-CMS.txt" //File path for the db
#endif

#define TABLE_NAME_LENGTH 15 //Length of table name
#define HASHMAP_LENGTH 103 //Hashmap size
#define NAME_LENGTH 30 //Name size for structure
#define PROGRAMME_LENGTH 30 //Programme size for structure
#define GENERAL_LENGTH 70 //General string size for input 
#define ID_LENGTH 7 //Size of ID input

char tableName[TABLE_NAME_LENGTH] = ""; //Initialise the table name as empty string on start
int currentHashmapSize = HASHMAP_LENGTH;  //Set the current size of the hashmap size
int recordCount = 0; //Initialise the count of student records

//Structure for student data and alias to StudentRecords with self referential structure
typedef struct StudentRecords {
    int id; //ID of the student
    char name[NAME_LENGTH]; //Name of the student
    char programme[PROGRAMME_LENGTH]; //Programme that the student is in
    float mark; //Mark of the programme for the student
    struct StudentRecords* next; //pointer to next node used for LL in the event of hash collision
} StudentRecords;

//Store the pointers to the student records as an array
typedef struct HashMap {
    StudentRecords** table;
} HashMap;

//Hashing function for the generating hash index based on the id 
unsigned int hash(int id) {
    //The use of 97 which is a prime number ensures a more evenly distribution
    return (id * 97) % currentHashmapSize;
}

//Function prototype
void DisplayDeclaration();
void InsertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark);
void UpdateStudent(HashMap* hashmap, const char* input);
struct StudentRecords* QueryStudent(HashMap* hashmap, int id, bool printrecord);
void DeleteStudent(HashMap* hashmap, int id);
void OpenFile(const char* filename, HashMap* hashmap);
void ShowAll(HashMap* hashmap);
void saveToFile(const char* filename, HashMap* hashmap);
void resizeHashMap(HashMap* currentHashmap);
int isStringValid(const char* input);
void RemoveTrailingSpaces(char* str);
int SortbyID(const void* a, const void* b);
char* GetField(const char* input, const char* key, int maxLength);
bool IsFieldDuplicate(const char* input);

//Function for AI declaration
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

//Function to insert new record of student data
void InsertStudent(HashMap* hashmap, int id, const char* name, const char* programme, float mark) {
    
    //Allocate memory dynamically for the new student record
    StudentRecords* newStudent = malloc(sizeof(StudentRecords));

    //Stop the insertion operation if memory allocation failed
    if (newStudent == NULL) {
        perror("CMS: Memory allocation failed for new student\n");
        exit(EXIT_FAILURE);
    }
    
    //Assign ID for the new student
    newStudent->id = id;
    //Assign the name for the student
    strncpy(newStudent->name, name, sizeof(newStudent->name) - 1), newStudent->name[sizeof(newStudent->name) - 1] = '\0';
    //Assign the programme for the student
    strncpy(newStudent->programme, programme, sizeof(newStudent->programme) - 1), newStudent->programme[sizeof(newStudent->programme) - 1] = '\0';
    //Assign the mark for the student
    newStudent->mark = mark;
    //Set the pointer to the next node as null since its the only record in the bucket currently
    newStudent->next = NULL;

    //Generate the hash index to be used in the hashmap based on the ID
    unsigned int hashIndex = hash(id);
    //Insert the student record into the hash table by chaining 
    newStudent->next = hashmap->table[hashIndex];
    //Point the new student record as the head of the LL
    hashmap->table[hashIndex] = newStudent;
    
    //Increase the current count of student records
    recordCount++;

    /*
    Handle the resizing of hashmap incase the current student record count exceeed the half the size of the current hashmap
    This is done to reduce the number of collisions
    */
    if (recordCount > (currentHashmapSize / 2)) {
        resizeHashMap(hashmap);
    }
}

//Function to update student data
void UpdateStudent(HashMap* hashmap, const char* input) {

    char newName[NAME_LENGTH] = { 0 }; //Declare new name 
    char newProgramme[PROGRAMME_LENGTH] = { 0 }; //Declare new Programme
    float newMark = -1; //Set new mark as -1 as default to ensure marks do not get updated unneccasarily
    int updateFieldCount = 0; //Declare count for checking how many fields are being updated

    //Get the ID input
    char* idInput = GetField(input, "ID=", sizeof(input));

    //If the id is null then return error message
    if (!idInput) {
        printf("%s: Please enter a valid ID.\n", USERNAME);
        return;
    }

    //Convert ID into integer 
    int id = atoi(idInput);

    //Get the name input
    char* nameInput = GetField(input, "Name=", sizeof(newName));

    //Check if the current name is not null 
    if (nameInput) {
        //Update the updateFieldCount since this field is being updated
        updateFieldCount++;
        //Check if the input string only containts letters or spacing
        if (!isStringValid(nameInput)) {
            printf("%s: Name should only contain letters and spaces.\n", USERNAME);
            return;
        }
        //Copy the name input into the new name string
        strncpy(newName, nameInput, NAME_LENGTH - 1);
    }

    //Get the Programme input
    char* programmeInput = GetField(input, "Programme=", sizeof(newProgramme));
    //Check if the programme input is not null
    if (programmeInput) {
        //Update the updateFieldCount since this field is being updated
        updateFieldCount++;
        //Check if the input string only containts letters or spacing
        if (!isStringValid(programmeInput)) {
            printf("%s: Programme should only contain letters and spaces.\n", USERNAME);
            return;
        }
        //Copy the programme input into the new name string
        strncpy(newProgramme, programmeInput, PROGRAMME_LENGTH - 1);
    }

    //Extract and validate Mark
    char* markInput = GetField(input, "Mark=", sizeof(input));
    //Check if the mark input is not null
    if (markInput) {
        //Update the updateFieldCount since this field is being updated
        updateFieldCount++;
        //Check if there is no error in getting the marks
        if (strcmp(markInput, "ERROR") == 0) {
            return;
        }
        //Convert the mark string to a float 
        newMark = atof(markInput);
    }
    
    //Check if there is no field currently being input to update
    if (!updateFieldCount) {
        printf("%s: Please enter desired field to update\n", USERNAME);
        return;
    }

    //Generate the hash index for that specific ID and get the current node 
    unsigned int hashIndex = hash(id);
    StudentRecords* currentStudentRecord = hashmap->table[hashIndex];

    //Loop through the LL to find the specific ID of that student record
    while (currentStudentRecord != NULL) {
        if (currentStudentRecord->id == id) {

            //Update the specific fields if there is input for that field
            if (*newName) {
                strncpy(currentStudentRecord->name, newName, sizeof(currentStudentRecord->name) - 1);
                currentStudentRecord->name[sizeof(currentStudentRecord->name) - 1] = '\0';
            }
            if (*newProgramme) {
                strncpy(currentStudentRecord->programme, newProgramme, sizeof(currentStudentRecord->programme) - 1);
                currentStudentRecord->programme[sizeof(currentStudentRecord->programme) - 1] = '\0';
            }
            if (newMark != -1) {
                currentStudentRecord->mark = newMark;
            }

            //Student record is updated
            printf("%s: The record with ID=%d is successfully updated.\n",USERNAME, id);
            return;
        }
        
        //set the next node to check if current ID does not match that of the desired student record 
        currentStudentRecord = currentStudentRecord->next;
    }

    //Return error message to show that there exist no such ID
    printf("%s: The record with ID=%d does not exist.\n",USERNAME, id);
}

//Function to search to student record
struct StudentRecords* QueryStudent(HashMap* hashmap, int id, bool printrecord) {

    //Generate the hash index for that specific ID and get the current node 
    unsigned int hashIndex = hash(id);
    StudentRecords* currentStudentRecord = hashmap->table[hashIndex];

    //Loop through the LL to find the specific ID of that student record
    while (currentStudentRecord != NULL) {
        if (currentStudentRecord->id == id) {

            //Print the specific student record
            if (printrecord == true) {
                printf("%s: The record with ID=%d is found in the data table.\n", USERNAME, id);
                printf("%-8s  %-30s  %-30s  %-5s\n", "ID", "Name", "Programme", "Mark");
                printf("%-8d  %-30s  %-30s  %.2f\n",currentStudentRecord->id, currentStudentRecord->name, currentStudentRecord->programme, currentStudentRecord->mark);
            }
            
            return currentStudentRecord;
        }
        //set the next node to check if current ID does not match that of the desired student record 
        currentStudentRecord = currentStudentRecord->next;
    }
   
    //Return error message to show that there exist no such ID
    if (printrecord == true) {
        printf("%s: The record with ID=%d does not exist.\n", USERNAME, id);
    }
    return NULL;
}

//Function to delete student record
void DeleteStudent(HashMap* hashmap, int id) {

    //Generate the hash index for that specific ID and get the current node 
    unsigned int hashIndex = hash(id);
    StudentRecords* currentStudentRecord = hashmap->table[hashIndex];
    StudentRecords* previousStudentRecord = NULL;

    //Loop through the LL to find the specific ID of that student record
    while (currentStudentRecord != NULL) {
        if (currentStudentRecord->id == id) {
            while (1) {
                //Buffer to hold the delete input 
                char deleteInput[3];
                //double confirm deletion of record
                printf("%s: Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel\n", USERNAME, id);
                printf("%s:", GROUP_NAME);

                //Get the delete input value
                deleteInput[strcspn(fgets(deleteInput, sizeof(deleteInput), stdin), "\n")] = 0;

                // clear stdin extra char if any
                if (deleteInput[1] != '\0') {
                    int clear;
                    while ((clear = getchar()) != '\n' && clear != EOF);

                }


                //Check if want to delete
                if (_strnicmp(deleteInput, "Y", 1) == 0) {

                    //If record is at head set new table index to current records link
                    if (previousStudentRecord == NULL) {
                        hashmap->table[hashIndex] = currentStudentRecord->next;
                    }
                    //Else set previous record link to the current record link to ensure no link is lost
                    else {
                        previousStudentRecord->next = currentStudentRecord->next;
                    }

                    //show that the student record has been deleted and decrease the current student record count
                    printf("%s: The record with ID=%d is successfully deleted.\n", USERNAME, id);
                    recordCount--;

                    //Free the memory of the deleted student record to prevent memory leak
                    free(currentStudentRecord);
                    return;
                }
                
                //Check if dont want to delete
                else if (_strnicmp(deleteInput, "N", 1) == 0) {
                    printf("%s: The deletion is cancelled.\n", USERNAME);
                    return;
                }
                else {
                    printf("%s: Invalid Command\n", USERNAME);
                }
            }
        }
        //set the next node to check if current ID does not match that of the desired student record 
        previousStudentRecord = currentStudentRecord;
        currentStudentRecord = currentStudentRecord->next;
    }
    //Return error message to show that there exist no such ID
    printf("%s: The record with ID=%d does not exist.\n", USERNAME, id);
}

//Function to open file
void OpenFile(const char* filename, HashMap* hashmap) {

    //open the file and return error message if file not detected
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    //Buffer for output text
    char outputText[256];

    while (fgets(outputText, sizeof(outputText), file) != NULL) {
        //Skip header lines
        if (strncmp(outputText, "Database Name:", 14) == 0 || strncmp(outputText, "Authors:", 8) == 0 || strncmp(outputText, "ID", 2) == 0) {
            continue;
        }

        //Extract the table name
        if (strncmp(outputText, "Table Name:", 11) == 0) {
            sscanf(outputText + 12, "%[^\n]", tableName);
            continue;
        }

        if (isdigit(outputText[0])) {

            int id; //Initilise id field
            char name[NAME_LENGTH]; //Initialise name field
            char programme[PROGRAMME_LENGTH]; //Initialise programme field
            float mark; //Initialise Mark field

            //Get field values
            int fields = sscanf(outputText, "%d\t%29[^\t]\t%29[^\t]\t%f", &id, name, programme, &mark);
            RemoveTrailingSpaces(name);
            RemoveTrailingSpaces(programme);
            
            //Check if all the fields are available then populate the hashmap
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
                fprintf(stderr, "%s: Error parsing outputText: %s\n",USERNAME, outputText);
            }
        }
    }

    fclose(file);
}

//Function to show all student records
void ShowAll(HashMap* hashmap) {
    //Check if there is no student records
    if (recordCount == 0) {
        printf("%s: No student records to display.\n", USERNAME);
        return;
    }

    //Allocate memory for the student records
    StudentRecords** allStudentRecords = malloc(recordCount * sizeof(StudentRecords*));

    //Return error message if memory allocation failed
    if (allStudentRecords == NULL) {
        fprintf(stderr, "%s: Memory allocation failed for allStudentRecords.\n", USERNAME);
        return;
    }

    //Student record index
    int recordsIndex = 0;

    //get all student records
    for (int i = 0; i < currentHashmapSize; i++) {
        StudentRecords* current = hashmap->table[i];
        
        while (current != NULL) {
            allStudentRecords[recordsIndex++] = current;
            current = current->next;
        }
    }

    //Sort students by ID in ascending order
    qsort(allStudentRecords, recordCount, sizeof(StudentRecords*), SortbyID);
    
    //Print all the studetn records
    printf("%s: Here are all the records found in the table \"%s\".\n", USERNAME, tableName);
    printf("%-8s  %-30s  %-30s  %-5s\n","ID", "Name", "Programme", "Mark");

    for (int i = 0; i < recordCount; i++) {
        StudentRecords* student = allStudentRecords[i];
        printf("%-8d  %-30s  %-30s  %.2f\n",
            student->id, student->name, student->programme, student->mark);
    }

    //Free the memory of all the student records
    free(allStudentRecords);
}

//Function to save updated student records to file
void saveToFile(const char* filename, HashMap* hashmap) {
    //Open the file in write mode
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    //Write all header lines
    fprintf(file, "Database Name: %s\n", USERNAME);
    fprintf(file, "Authors: Ryan Wong, Zheng Yang, Sabihah, Devin, Timothy, Naveen\n\n");
    fprintf(file, "Table Name: %s\n", tableName);
    fprintf(file, "%-7s\t%-30s\t%-30s\t%-5s\n","ID", "Name", "Programme", "Mark");

    //Write all student records
    for (int i = 0; i < currentHashmapSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            fprintf(file, "%-7d\t%-30s\t%-30s\t%-3.2f\n", current->id, current->name, current->programme, current->mark);
            current = current->next;
        }
    }

    printf("%s: The database file \"%s\" has been successfully updated.\n", USERNAME, FILE_PATH);
    fclose(file);
}

//Function to resize the hashmap to avoid hash collision
void resizeHashMap(HashMap* currentHashmap) {

    //Get the next prime number greater than double the current size
    unsigned int newHashmapSize = currentHashmapSize * 2 + 1;
    while (1) {
        int isPrime = 1;
        //check using odd numbers
        for (int j = 3; j * j <= newHashmapSize; j += 2) {
            if (newHashmapSize % j == 0) {
                isPrime = 0;
                break;
            }
        }
        //Skip numbers lesser than 2
        if (isPrime && newHashmapSize > 2) {
            break;
        }
        newHashmapSize += 2;
    }

    //Allocate memory for the new hash map
    HashMap* newHashMap = malloc(sizeof(HashMap));
    if (!newHashMap || !(newHashMap->table = calloc(newHashmapSize, sizeof(StudentRecords*)))) {
        fprintf(stderr, "%s: Memory allocation failed during resizing\n", USERNAME);
        exit(EXIT_FAILURE);
    }

    //Generate new hash index for each student record and then insert them back into the hashmap
    for (int i = 0; i < currentHashmapSize; i++) {
        StudentRecords* current = currentHashmap->table[i];
        while (current) {
            StudentRecords* nextRecord = current->next;
            unsigned int newIndex = current->id % newHashmapSize;

            current->next = newHashMap->table[newIndex];
            newHashMap->table[newIndex] = current;

            current = nextRecord;
        }
    }

    //Free memory of hashmaps
    free(currentHashmap->table);
    *currentHashmap = *newHashMap;
    free(newHashMap);

    //Update new current size of hashmap
    currentHashmapSize = newHashmapSize;
}

//Function to check if the string only contains letters
int isStringValid(const char* input) {

    //Loop through each letter in the string 
    for (int i = 0; input[i] != '\0'; i++) {
        // Check if it is neither a letter, space, tab
        if (!isalpha((unsigned char)input[i]) && !isspace((unsigned char)input[i]) && input[i] != '\t') {
            // Return 0 to show that this string is not valid input
            return 0;
        }
        // Explicitly disallow tabs
        if (input[i] == '\t') {
            return 0;
        }
    }
    //Return 1 to show this string is valid
    return 1;
}

//Function to remove the Trailing spaces on each end 
void RemoveTrailingSpaces(char* str) {
    //Remove trailing spaces
    char* endOfString = str + strlen(str) - 1;
    while (endOfString >= str && isspace((unsigned char)*endOfString)) {
        endOfString--;
    }
    *(endOfString + 1) = '\0';

    //Remove Leading spaces
    char* startOfString = str;
    while (*startOfString && isspace((unsigned char)*startOfString)) {
        startOfString++;
    }
    if (startOfString != str) {
        memmove(str, startOfString, strlen(startOfString) + 1);
    }
}

//Function to sort the students based on their ID
int SortbyID(const void* studenta, const void* studentb) {
    //Dereference the student and return the difference in ID
    StudentRecords* studentA = *(StudentRecords**)studenta;
    StudentRecords* studentB = *(StudentRecords**)studentb;
    return studentA->id - studentB->id;  
}

//Function to extract each value from individual field
char* GetField(const char* input, const char* key, int maxLength) {
    //Buffer to store the extracted field value
    static char desiredFieldOutput[GENERAL_LENGTH];

    //Find for the first occurence of the key
    const char* start = strstr(input, key);
    if (!start) {
        return NULL;
    }

    //Point to the value of the key
    start += strlen(key);

    //Find for the other fields
    const char* endOfID = strstr(start, "ID=");
    const char* endOfName = strstr(start, "Name=");
    const char* endOfProgramme = strstr(start, "Programme=");
    const char* endOfMark = strstr(start, "Mark=");
    
    //Get the end of that specific field
    const char* endOfString = NULL;
    if (endOfID && (!endOfString || endOfID < endOfString)) {
        endOfString = endOfID;
    }
    if (endOfName && (!endOfString || endOfName < endOfString)) {
        endOfString = endOfName;
    }
    if (endOfProgramme && (!endOfString || endOfProgramme < endOfString)) {
        endOfString = endOfProgramme;
    }
    if (endOfMark && (!endOfString || endOfMark < endOfString)) {
        endOfString = endOfMark;
    }

    //Go to end of string if no fields are found
    if (!endOfString) {
        endOfString = input + strlen(input);
    }

    //Extract the value from the field
    int length = endOfString - start;
    if (length >= maxLength) {
        length = maxLength - 1; 
    }
    strncpy(desiredFieldOutput, start, length);
    
    desiredFieldOutput[length] = '\0';

    RemoveTrailingSpaces(desiredFieldOutput);

    if (strcmp(key, "Name=") == 0) {
        //If the extracted field value is empty
        if (strlen(desiredFieldOutput) == 0) {
            printf("%s: Name field cannot be empty.\n", USERNAME);
            return NULL;
        }
    }
    if (strcmp(key, "Programme=") == 0) {
        //If the extracted field value is empty
        if (strlen(desiredFieldOutput) == 0) {
            printf("%s: Programme field cannot be empty.\n", USERNAME);
            return NULL;
        }
    }

    //Check for ID number validation
    if (strcmp(key, "ID=") == 0) {
        //Allow ID to contain only numbers
        for (int i = 0; desiredFieldOutput[i] != '\0'; i++) {
            if (!isdigit((unsigned char)desiredFieldOutput[i])) {
                printf("%s: ID must contain only numbers.\n", USERNAME);
                return NULL;
            }
        }

        int idValue = atoi(desiredFieldOutput);
        if (idValue < 0) {
            printf("%s: ID cannot be negative.\n", USERNAME);
            return NULL;
        }
        else if (idValue == 0) {
            printf("%s: ID cannot be 0.\n", USERNAME);
            return NULL;
        }
        else if ((int)log10(abs(idValue)) + 1 != ID_LENGTH) {
            printf("%s: ID length not 7 digits.\n", USERNAME);
            return NULL;
        }

    }

    //Check for mark number validation
    if (strcmp(key, "Mark=") == 0) {
        //If the extracted field value is empty
        if (strlen(desiredFieldOutput) == 0) {
            printf("%s: Mark field cannot be empty.\n", USERNAME);
            return "ERROR";
        }

        int numOfFullStop = 0;
        char* tempMark = desiredFieldOutput;
      
        if (tempMark[0] == '\0') {
            printf("%s: Please key in marks.\n", USERNAME);
            return "ERROR";
        }
        //Ensure there are no double ".." in the marks and if its a valid float
        for (size_t i = 0; i < strlen(tempMark); i++) {
            if (tempMark[i] == '.') {
                numOfFullStop++;
                if (numOfFullStop > 1 || i == 0 || i == strlen(tempMark) - 1) {
                    printf("%s: Invalid Mark format. Ensure it is a valid float value.\n", USERNAME);
                    return "ERROR";
                }
            }
            else if (!isdigit((unsigned char)tempMark[i])) {
                printf("%s: Mark must contain only numeric characters or a single decimal point.\n", USERNAME);
                return "ERROR";
            }
        }

        //Check for the mark range
        float markValue = atof(tempMark);
        if (markValue < 0 || markValue > 100) {
            printf("%s: Mark must be between 0 and 100.\n", USERNAME);
            return "ERROR";
        }
    }
    return desiredFieldOutput;
}

//Function to check if there is any duplicate fields in the input
bool IsFieldDuplicate(const char* input) {
    
    //Set all field count to 0
    int idCount = 0;
    int nameCount = 0;
    int programmeCount = 0;
    int markCount = 0;

    //Tokenize the field
    char* tempField = strdup(input);
    char* token = strtok(tempField, " ");

    //Update the count of the Field
    while (token) {
        if (strstr(token, "ID=")) {
            idCount++;
        }
        else if (strstr(token, "Name=")) {
            nameCount++;
        }
        else if (strstr(token, "Programme=")) {
            programmeCount++;
        }
        else if (strstr(token, "Mark=")) {
            markCount++;
        }
        token = strtok(NULL, " ");
    }

    //Free the memory 
    free(tempField);

    //Return true if any of the fields have more than 1 count
    return idCount > 1 || nameCount > 1 || programmeCount > 1 || markCount > 1;
}

int main() {

#if TEST_MODE == 1
    printf("TESTING MODE ON\n");

    //add test inputs into stdin
    FILE* input_fp = freopen("testinput.txt", "r", stdin);
    FILE* output_fp = freopen("output.txt", "w", stdout);

#endif


    //check for memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    //Set this to file not opened yet
    int isFileOpened = 0;

    //Allocate memory for the hashmap
    HashMap* hashmap = malloc(sizeof(HashMap));

    if (hashmap == NULL) {
        fprintf(stderr, "%s: Memory allocation failed\n", USERNAME);
        exit(EXIT_FAILURE);
    }

    hashmap->table = calloc(currentHashmapSize, sizeof(StudentRecords*));
    if (!hashmap->table) {
        fprintf(stderr, "%s: Memory allocation failed for table\n", USERNAME);
        exit(EXIT_FAILURE);
    }

    //Display the AI declaration
    //DisplayDeclaration();

    while (1) {
        printf("%s:", GROUP_NAME);

        //Get the input from the user and remove any trailing spaces
        char input[256];
        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;
        RemoveTrailingSpaces(input);

        //Return error message if there is duplicate fields
        if (IsFieldDuplicate(input)) {
            printf("%s: Duplicate parameter detected in the input.\n", USERNAME);
            continue;
        }

        //Check if any of the other commands are entered before opening file then show error message
        if ((_strnicmp(input, "show all", 8) == 0 || _strnicmp(input, "update", 6) == 0 || _strnicmp(input, "delete", 6) == 0 || _strnicmp(input, "query", 5) == 0 || _strnicmp(input, "insert", 6) == 0 || _strnicmp(input, "save", 4) == 0) && isFileOpened == 0) {
            printf("%s: Database file not open yet.\n", USERNAME);
            continue;
        }

        //Open the file
        if (_stricmp(input, "open") == 0) {
            
            //Return error message saying file has already been opened
            if (isFileOpened == 1) {
                printf("%s: Database file is open already.\n", USERNAME);
                continue;
            }

            //Else open the file and set the flag to file has been opened
            OpenFile(FILE_PATH, hashmap);
            isFileOpened = 1;
            printf("%s: The database file \"%s\" is successfully opened.\n", USERNAME, FILE_PATH);
        }
        //Show all student records
        else if (_stricmp(input, "show all") == 0) {
            ShowAll(hashmap);
        }
        //Update Student records
        else if (_strnicmp(input, "update", 6) == 0) {
            char* value = GetField(input, "ID=", sizeof(input));
            if (value == NULL) {
                printf("%s: Invalid Command. Usage: UPDATE ID=<id>\n", USERNAME);
                continue;
            }

            UpdateStudent(hashmap, input);
        }
        //Delete student records
        else if (_strnicmp(input, "delete", 6) == 0) {
            //use helper function to extract ID 
            char* value = GetField(input, "ID=",sizeof(input));
            if (value == NULL) {
                printf("%s: Invalid Command. Usage: DELETE ID=<id>\n", USERNAME);
                continue;
            }
            //Convert ID into integer
            int id = atoi(value);

            DeleteStudent(hashmap, id);
        }
        //Query the student records
        else if (_strnicmp(input, "query", 5) == 0) {
            
            char* value = GetField(input, "ID=", sizeof(input));
            if (value == NULL) {
                printf("%s: Invalid Command. Usage: QUERY ID=<id>\n", USERNAME);
                continue;
            }
            //Convert ID into integer
            int id = atoi(value);
         
            QueryStudent(hashmap, id, true);
        }
        //Insert new student record
        else if (_strnicmp(input, "insert", 6) == 0) {
           
            char* params = input + 7;
            int id = 0;
            char name[NAME_LENGTH] = { 0 };
            char programme[PROGRAMME_LENGTH] = { 0 };
            float mark = 0;
            int fieldCount = 0;

            //Extract and validate ID parameter
            char* idParam = GetField(input, "ID=", sizeof(input));
            if (!idParam || (id = atoi(idParam)) <= 0) {
                printf("%s: Invalid Command. Usage: INSERT ID=<id>\n", USERNAME);
                continue;
            }
            fieldCount++;
            //Extract and validate Name parameter
            char* nameParam = GetField(input, "Name=", sizeof(name));
            if (nameParam) {
                if (!isStringValid(nameParam)) {
                    printf("%s: Name should only contain letters and spaces.\n", USERNAME);
                    continue;
                }
            }
            if (!nameParam) {
                printf("%s: Name field is required.\n", USERNAME);
                continue;
            }
            fieldCount++;
            strncpy(name, nameParam, NAME_LENGTH - 1);

            //Extract and validate Programme parameter
            char* programmeParam = GetField(input, "Programme=", sizeof(programme));
            if (programmeParam) {
                if (!isStringValid(programmeParam)) {
                    printf("%s: Programme should only contain letters and spaces.\n", USERNAME);
                    continue;
                }
            }
           
            if (!programmeParam) {
                printf("%s: Programme field is required.\n", USERNAME);
                continue;
            }
            fieldCount++;
            strncpy(programme, programmeParam, PROGRAMME_LENGTH - 1);
            //Extract and validate Mark parameter
            char* markParam = GetField(input, "Mark=", sizeof(input));
            if (markParam) {
                if (strcmp(markParam, "ERROR") == 0) {
                    continue;
                }
                else {
                    fieldCount++;
                    //Convert the Mark parameter from string to float and call the function to validate it
                    float markValue = atof(markParam);
                    mark = markValue;
                }
            }

            //Ensure all fields are available for input
            if (fieldCount != 4) {
                printf("%s: Please enter ID, Name, Programme and Mark fields.\n", USERNAME);
                continue;
            }

            //Check if student record already exists in database using ID parameter
            if (QueryStudent(hashmap, id, false)) {
                printf("%s: The record with ID=%d already exists.\n",USERNAME, id);
            }
            //If ID does not exist, new student record will be inserted
            else {
                InsertStudent(hashmap, id, name, programme, mark);
                printf("%s: A new record with ID=%d is successfully inserted.\n", USERNAME, id);
            }
        }

        //Exit the program
        else if (_stricmp(input, "exit") == 0) {
            break;
        }
        //Save update student records
        else if (_stricmp(input, "SAVE") == 0) {
        
            #if TEST_MODE == 1
            saveToFile("testdb2.txt", hashmap);
            printf("Data has been successfully saved to testdb2.txt.\n");


            #else
            saveToFile(FILE_PATH, hashmap);

            #endif

        }
        else {
            printf("%s: Invalid Command. \n", USERNAME);

        }
    }


    //Free all memory
    for (int i = 0; i < currentHashmapSize; i++) {
        StudentRecords* current = hashmap->table[i];
        while (current != NULL) {
            StudentRecords* temp = current;
            current = current->next;
            free(temp);
        }

        free(hashmap->table);
        free(hashmap);

#if TEST_MODE == 1

        fclose(input_fp);
        fclose(output_fp);
        runTest();

#endif
        return 0;
    }
}
      
/*

OPEN TEST CASES

test 1
open invalid command 
"openc"

expected 
"invalid command"

Test 2
testing of other commands not to operate before open
insert
show all
query
update
delete
save

expected 
"db not open"

Test 3
testing of open

open

expected
"The database file testdb.txt is successfully opened.

Test 4
testing different uppercase and random command
 Insert
sHow all
Query
Update
del ete

INSERT
SHOW ALL
QUERY
UPDATE
DELETE


test 5
test if data match db file

show all 

test 6
test insert with record alr in
insert id=2301234
INSERT ID=2301234 Name=sam  Programme=cybersecurity  Mark=90


test 7
test insert with no record in 

INSERT ID=2304444 Name=sam  Programme=cybersecurity  Mark=90
show all


test 8
test insert with different data type
INSERT ID=2304450 Name=ssgfe3!  Programme=cybersecurity  Mark=85
INSERT ID=2304461 Name=tester  Programme=cybersecurity aeoifeoifeofofe  Mark=90
INSERT ID=2304478 Name=tester  Programme=cybersecurity Mark=rgdf
INSERT ID=230448 Name=tester koh  Programme=cybersecurity Mark=90
INSERT ID=230448a Name=tester koh  Programme=cybersecurity Mark=90
INSERT ID=000000 Name=tester koh  Programme=cybersecurity Mark=90
INSERT ID=010000 Name=tester koh  Programme=cybersecurity Mark=90


test 9 
test function with negative ID (all having issue)

//INSERT ID=-230448 Name=tester koh  Programme=cybersecurity Mark=90
//UPDATE ID=-2395313 Name=tester ID neg
//DELETE ID=-2335554
//QUERY ID=-1242343


test 10
test update function 
UPDATE ID=2201234 Name=tester update
UPDATE ID=2201234 Programme=AI
UPDATE ID=2201234 Mark= 98


test 11
test update fucntion with different data type
UPDATE ID=2201234 Mark=sef
UPDATE ID=2304567 Mark=10000000111111111



*/


