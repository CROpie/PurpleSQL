#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "appmain.h"
#include "unity.h"

void setUp(void) {
  //
}

void tearDown(void) {
}

void test_main_output(void) {
    // Redirect stdout to a temp file
    FILE* tempOut = fopen("temp_output.txt", "w+");
    FILE* oldStdout = stdout;
    stdout = tempOut;

    // Redirect stdin with fmemopen or a file
    char input[] = 
        "CREATE TABLE myTable ( id INT, isDeleted BOOL, message VARCHAR(255) );\n"
        "INSERT INTO myTable ( id, isDeleted, message ) VALUES (1, false, 'first message');\n"
        "SELECT * FROM myTable;\n"
        "exit;\n"
    ;

    char* expectedOutput[] = {
        "Table creation successful.",
        "Record insertion successful.",
        "{ 1, false, 'first message', }",
        "goodbyte"
    };

    FILE* mockIn = fmemopen(input, strlen(input), "r");
    FILE* oldStdin = stdin;
    stdin = mockIn;

    // Run your main function (or any function that prints)
    appMain();

    // Restore stdin
    stdin = oldStdin;
    fclose(mockIn);

    // Restore and read stdout
    fflush(tempOut);
    fseek(tempOut, 0, SEEK_SET);
    char buffer[1024];
    fread(buffer, 1, sizeof(buffer) - 1, tempOut);
    buffer[1023] = '\0';  // Null-terminate just in case

    stdout = oldStdout;
    fclose(tempOut);

    int expectedLineCount = sizeof(expectedOutput) / sizeof(expectedOutput[0]);

    // Now assert output
    char* line = strtok(buffer, "\n");
    int lineIndex = 0;

    while (line != NULL && lineIndex < expectedLineCount) {
        printf("Line: %s\n", line);  // for debugging
        
        TEST_ASSERT_EQUAL_STRING(expectedOutput[lineIndex++], line);
        line = strtok(NULL, "\n");
    }
}

int main(void) {
    UNITY_BEGIN();

   RUN_TEST(test_main_output);

    return UNITY_END();
}
