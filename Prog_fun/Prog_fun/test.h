
#define _CRTDBG_MAP_ALLOC // allow debugging to find where is the cause of memory leak
#define _CRT_SECURE_NO_WARNINGS // suppress unnecassary warnings

#include <ctype.h>  
#include <stdlib.h>
#include <string.h> // library for strcpy, strlen
#include <stdio.h> // library for input and output

#define INPUT_FILE "testinput.txt"//commands to run
#define OUTPUT_FILE "output2.txt" //load the output of the test
#define BASE_FILE "testoutput.txt"//expected result
#define TEST_RESULT "testresult.txt"//where to print the test results

#define LINE_MAX 256
#define MAX_CASES 256
typedef struct errorCase {
	int commandNum;
	char commandLine[LINE_MAX];
	char errorLine[LINE_MAX];
	char baseLine[LINE_MAX];
} errorCase;

void printErrorCase(FILE* result,errorCase ec) {
	fprintf(result, "For command number:%d, \"%s\" the following error was found:\n", ec.commandNum,ec.commandLine);
	fprintf(result, "Expected:%s", ec.baseLine);
	fprintf(result, "Got:%s\n", ec.errorLine);
	return;
}

void runTest() {
    FILE* input = fopen(INPUT_FILE, "r");
	FILE* output = fopen(OUTPUT_FILE, "r");
	FILE* base = fopen(BASE_FILE, "r");
	FILE* result = fopen(TEST_RESULT,"w");
	if (input == NULL || output == NULL || base == NULL) {
		return;
	}

	errorCase* failCases = (errorCase*)malloc(MAX_CASES * sizeof(errorCase));
	int commandNum = 0,pos=-1;



	char inputLine[LINE_MAX] = {'\0'}, outputLine[LINE_MAX] = { '\0' }, baseLine[LINE_MAX] = { '\0' };
	while (fgets(outputLine, sizeof(outputLine), output)&&fgets(baseLine, sizeof(baseLine), base)) {

		if (outputLine[0] == 'P' && outputLine[1] == '2') {//if new command
			commandNum++;
		}
		if (strcmp(outputLine, baseLine)) {//if error
			//errorCase tempCase;
			//tempCase.commandNum = commandNum;
			//strcpy(tempCase.errorLine, outputLine);
			//strcpy(tempCase.baseLine, baseLine);
			//failCases[pos++] = tempCase;

			fprintf(result, "For command number:%d the following error was found:\n", commandNum);
			fprintf(result, "Expected:%s", baseLine);
			fprintf(result, "Got:%s\n", outputLine);
		}

	}
	//int pos2 = 0;//count the currentline in the input file
	//for (int i = 0; i<pos; i++) {
	//	while (fgets(inputLine, sizeof(inputLine), input)) {
	//		pos2++;
	//		if (pos2 == failCases[i].commandNum) {
	//			strtok(inputLine, "\n");
	//			strcpy(failCases[i].commandLine, inputLine);
	//			break;
	//		}
	//		
	//	}
	//	printErrorCase(result, failCases[i]);
	//}

	fclose(input), fclose(output), fclose(base),fclose(result);
	return;
}
