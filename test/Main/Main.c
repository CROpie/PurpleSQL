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

void test_two_tables(void) {
    // Redirect stdout to a temp file
    FILE* tempOut = fopen("temp_output.txt", "w+");
    FILE* oldStdout = stdout;
    stdout = tempOut;

    char input[] = 
        "CREATE TABLE myTable ( id INT, isDeleted BOOL, message VARCHAR(255) );\n"
        "INSERT INTO myTable ( id, isDeleted, message ) VALUES (1, false, 'first message');\n"
        "CREATE TABLE anotherTable ( userId INT, userMessage VARCHAR(255) );\n"
        "INSERT INTO anotherTable ( userId, userMessage ) VALUES (47, 'second table!');\n"
        "SELECT * FROM myTable;\n"
        "SELECT * FROM anotherTable;\n"
        "exit;\n"
    ;

    char* expectedOutput[] = {
        "Table creation successful.",
        "Record insertion successful.",
        "Table creation successful.",
        "Record insertion successful.",
        "{ 1, false, 'first message', }",
        "{ 47, 'second table!', }",
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
    char buffer[1024] = {0};
    fread(buffer, 1, sizeof(buffer) - 1, tempOut);
    buffer[1023] = '\0';  // Null-terminate just in case

    stdout = oldStdout;
    fclose(tempOut);
    // remove("temp_output.txt");

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

void test_main_output(void) {
    // Redirect stdout to a temp file
    FILE* tempOut = fopen("temp_output.txt", "w+");
    FILE* oldStdout = stdout;
    stdout = tempOut;

    char input[] = 
        "CREATE TABLE testTable ( id INT, isDeleted BOOL, message VARCHAR(255) );\n"
        "INSERT INTO testTable ( id, isDeleted, message ) VALUES (1, false, 'first message');\n"
        "SELECT * FROM testTable;\n"
        "DROP TABLE testTable;\n"
        "exit;\n"
    ;

    char* expectedOutput[] = {
        "Table creation successful.",
        "Record insertion successful.",
        "{ 1, false, 'first message', }",
        "Table deleted.",
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
    char buffer[1024] = {0};
    fread(buffer, 1, sizeof(buffer) - 1, tempOut);
    buffer[1023] = '\0';  // Null-terminate just in case

    stdout = oldStdout;
    fclose(tempOut);
    remove("temp_output.txt");

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
//  RUN_TEST(test_two_tables);

    return UNITY_END();
}
