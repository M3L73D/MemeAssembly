#include "translator.h"
#include "../logger/log.h"

#include <time.h>
#include <string.h>

extern char* version_string;
extern struct command commandList[];

void translateToAssembly(struct parsedCommand parsedCommand, FILE *outputFile) {
    struct command command = commandList[parsedCommand.opcode];
    char *translationPattern = command.translationPattern;

    size_t strLen = strlen(translationPattern) - command.usedParameters;
    for(int i = 0; i < command.usedParameters; i++) {
        strLen += strlen(parsedCommand.parameters[i]);
    }

    char *translatedLine = malloc(strLen + 1); //Include an extra byte for the null-Pointer
    if(translatedLine == NULL) {
        fprintf(stderr, "Critical error: Memory allocation for command translation failed!");
        exit(EXIT_FAILURE);
    }
    translatedLine[0] = '\0';

    for(int i = 0; i < strlen(translationPattern); i++) {
        char character = translationPattern[i];
        if(character >= '0' && character <= (char) command.usedParameters + 48) {
            printDebugMessage("\tAppending parameter", parsedCommand.parameters[character - 48]);
            strncat(translatedLine, parsedCommand.parameters[character - 48], strLen);
        } else {
            char appendix[2] = {character, '\0'};
            strncat(translatedLine, appendix, strLen);
        }
    }

    printDebugMessage("\tWriting to file: ", translatedLine);
    if(parsedCommand.opcode != 0) {
        fprintf(outputFile, "\t");
    }
    fprintf(outputFile, "%s\n", translatedLine);

    printDebugMessage("\tDone, freeing memory", "");
    free(translatedLine);
}

void writeToFile(struct commandsArray *commandsArray, FILE *outputFile) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(outputFile, "#\n# Generated by the MemeAssembly compiler %s on %s#\n", version_string, asctime(&tm));
    fprintf(outputFile, ".intel_syntax noprefix\n\n");
    fprintf(outputFile, ".section .text\n");

    //Traverse the commandsArray to look for any functions
    for(int i = 0; i < commandsArray -> size; i++) {
        if(commandsArray -> arrayPointer[i].opcode == 0 && commandsArray -> arrayPointer[i].translate == 1) {
            //Write the function name with the prefix ".global" to the file
            fprintf(outputFile, ".global %s\n", commandsArray -> arrayPointer[i].parameters[0]);
        }
    }
    fprintf(outputFile, "\n\n");

    for(int i = 0; i < commandsArray -> size; i++) {
        if(i == commandsArray -> randomIndex) {
            fprintf(outputFile, "\t.LConfusedStonks: ");
        }

        if(commandsArray -> arrayPointer[i].translate == 1) {
            printDebugMessageWithNumber("Translating Index:", i);
            translateToAssembly(commandsArray -> arrayPointer[i], outputFile);
        }
    }

    printDebugMessage("Done, closing output file", "");
    fclose(outputFile);
}
