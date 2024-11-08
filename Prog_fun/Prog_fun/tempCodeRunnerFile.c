// Process each field=value pair
    char* fieldsCopy = strdup(fieldsStart);  // Duplicate the fields section for safe tokenization
    char* token = fieldsCopy;

    while (token != NULL) {
        // Find the position of '=' in the current token
        char* equalSign = strchr(token, '=');
        if (equalSign == NULL) {
            printf("Invalid format: Missing '=' in field assignment.\n");
            free(fieldsCopy);
            return;
        }

        // Separate field and value by setting '=' to '\0'
        *equalSign = '\0';
        const char* field = token;
        const char* value = equalSign + 1;

        // Advance token to the next space-separated part, if any
        char* nextToken = strchr(value, ' ');
        if (nextToken != NULL) {
            *nextToken = '\0';  // End current value at the next space
            token = nextToken + 1;  // Move to the next field
        } else {
            token = NULL;  // No more fields
        }

        // Match the field and set the appropriate flag and value
        if (strcmp(field, "Name") == 0) {
            strncpy(newName, value, STUDENT_NAME_LENGTH - 1);
            newName[STUDENT_NAME_LENGTH - 1] = '\0';
            nameFlag = 1;
        } else if (strcmp(field, "Programme") == 0) {
            strncpy(newProgramme, value, PROGRAMME_LENGTH - 1);
            newProgramme[PROGRAMME_LENGTH - 1] = '\0';
            programmeFlag = 1;
        } else if (strcmp(field, "Mark") == 0) {
            newMark = atof(value);  // Convert string to float
            markFlag = 1;
        } else {
            printf("Unknown field: %s\n", field);
            free(fieldsCopy);
            return;
        }
    }

    // Call the update function with flags indicating which fields to update
    updateStudentByID(hashmap, id, newName, newProgramme, newMark, nameFlag, programmeFlag, markFlag);
    free(fieldsCopy);
}