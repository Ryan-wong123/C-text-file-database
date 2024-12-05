
#define _CRTDBG_MAP_ALLOC // allow debugging to find where is the cause of memory leak
#define _CRT_SECURE_NO_WARNINGS // suppress unnecassary warnings

#include <ctype.h>  
#include <stdlib.h>
#include <string.h> // library for strcpy, strlen
#include <stdio.h> // library for input and output

#define INPUT_FILE "testinput.txt"//commands to run
#define OUTPUT_FILE "output.txt" //load the output of the test
#define BASE_FILE "testoutput.txt"//expected result
#define TEST_RESULT "testresult.txt"//where to print the test results

#define LINE_MAX 256
#define MAX_CASES 256


void runTest() {
	FILE* output = fopen(OUTPUT_FILE, "r");
	FILE* base = fopen(BASE_FILE, "r");
	FILE* result = fopen(TEST_RESULT,"w");
	if (output == NULL || base == NULL) {
		return;
	}
	int commandNum = 0,pos=-1;
	char inputLine[LINE_MAX] = {'\0'}, outputLine[LINE_MAX] = { '\0' }, baseLine[LINE_MAX] = { '\0' };
	while (fgets(outputLine, sizeof(outputLine), output)&&fgets(baseLine, sizeof(baseLine), base)) {//loop thru both files

		if (outputLine[0] == 'P' && outputLine[1] == '2') {//check for new command
			commandNum++;
		}
		if (baseLine[0] == '-' && baseLine[1] == '-') {//check for sep
			strtok(baseLine, "|");
			char* cat = strtok(NULL, "|");
			fprintf(result, cat);
			fprintf(result, "\n");
			continue;
		}
		if (strcmp(outputLine, baseLine)) {//check for error
			fprintf(result, "For command number:%d the following error was found:\n", commandNum);
			fprintf(result, "Expected:%s", baseLine);
			fprintf(result, "Got:%s\n", outputLine);
		}

	}

	fclose(output), fclose(base),fclose(result);
	return;
}
