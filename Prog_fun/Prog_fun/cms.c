#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_STUDENTS 100 // Define a maximum number of students
#define FILE_PATH "database.txt"
#define USERNAME "CMS"

int count = 0;
char input[256];
char tableName[30] = "";

typedef struct StudentRecords {
    int id;
    char name[30];
    char programme[30];
    float mark;
} StudentRecords;

void OpenFile(const char* filename, StudentRecords* students, int* count) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];

    *count = 0;

    while (fgets(line, sizeof(line), file)) {
        // do not read the header lines
        if (strncmp(line, "Database Name:", 14) == 0 ||
            strncmp(line, "Authors:", 8) == 0 ||
            strncmp(line, "ID", 2) == 0) {
            continue;
        }

        // Get the words after "Table Name:" and save it as the table name
        if (strncmp(line, "Table Name:", 11) == 0) {
            sscanf_s(line + 12, "%[^\n]", tableName, (unsigned)_countof(tableName));
            continue; 


        }

        // Check if the line starts with a digit (ID)
        if (isdigit(line[0])) {
            if (*count < MAX_STUDENTS) { // Check if there's space for another student
                int fields = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f",
                    &students[*count].id,
                    students[*count].name,
                    students[*count].programme,
                    &students[*count].mark);
                if (fields == 4) {
                    (*count)++;
                }
                else {
                    fprintf(stderr, "Error parsing line: %s\n", line);
                }
            }
            else {
                fprintf(stderr, "Maximum number of students reached.\n");
                break; // Exit the loop if limit is reached
            }
        }
    }
    printf("%s: The database file \"%s\" is successfully opened.\n", USERNAME, FILE_PATH);
    fclose(file);
}

void ShowAll(StudentRecords* students, int count) {
    printf("%s: Here are all the records found in the table \"%s\".\n", USERNAME, tableName);
    // Print the data to verify it was read correctly
    for (int i = 0; i < count; i++) {
        printf("ID: %d, Name: %s, Programme: %s, Mark: %.2f\n",
            students[i].id, students[i].name, students[i].programme, students[i].mark);
    }
}

int main() {
    StudentRecords* students = malloc(sizeof(StudentRecords) * MAX_STUDENTS); // Allocate memory for student records
    if (students == NULL) {
        perror("Error allocating memory");
        return 1; // Exit if memory allocation fails
    }

    while (1) {
        printf("Enter an operation\n");

        input[strcspn(fgets(input, sizeof(input), stdin), "\n")] = 0;

        if (input != NULL) {
            if (_stricmp(input, "open") == 0) {
                OpenFile(FILE_PATH, students, &count);
            }
            else if (_stricmp(input, "show all") == 0) {
                ShowAll(students, count);
            }
            else if (_stricmp(input, "exit") == 0) {
                break;
            }
        }
    }

    
    free(students); // Free allocated memory
    return 0;
}
