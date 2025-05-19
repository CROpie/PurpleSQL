#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> // isspace

#include "database.h"
#include "repl.h"

/* HELPER FUNCTIONS */
/* FREE MEMORY */
void freeCommand(Command* command) {
  if (!command) return;

  // generic
  if (command->tableName) free(command->tableName);

  // create
  if (command->c_colPairs) {
    for (int i = 0; i < command->c_numColPairs; i++) {
      free(command->c_colPairs[i].colName);
      free(command->c_colPairs[i].colDef);
    }
    free(command->c_colPairs);
  }

  // insert
  if (command->i_colNames) {
    for (int i = 0; i < command->i_numColNames; i++) {
      free(command->i_colNames[i]);
    }
    free(command->i_colNames);
  }


  if (command->i_colValueRows) {
    for (int i = 0; i < command->i_numValueRows; i++) {

      for (int j = 0; j < command->i_numColNames; j++) {
        free(command->i_colValueRows[i][j]);
      }

      free(command->i_colValueRows[i]);
    }
    free(command->i_colValueRows);
  }

  // final
  free(command);
}

/* ERRORS */
char* writeError(char* str) {
	char* errMsg = malloc(strlen(str) + 1);
	strcpy(errMsg, str);
	return errMsg;
} 

/* */
char* replaceEscapedQuote(char* str) {
	size_t len = strlen(str);

	char* result = calloc(len + 1, 1);
	char* dst = result;

	for (char* src = str; *src; src++) {
		if (*src == '\'' && *(src + 1) == '\'') {
			*dst = '\'';
			src++;
		} else {
			*dst = *src;
		}
		dst++;
	}
	*dst = '\0';
	return result;
}

void trimWhitespace(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) { // All spaces?
        *str = '\0';
        return;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = '\0';

    // Optional: move trimmed string to the start of the buffer
    memmove((void*)(str - (str - str)), str, strlen(str) + 1);
}

/* VALIDATION FUNCTIONS */
// Check whether the number of ( matches ) and ' matches ' (not including inside quotes)
bool checkBalancedParensAndQuotes(char* input) {
	int openParens= 0;
	bool inQuote = false;

	char* p = input;

	while (*p) {
		// quote detected
		if (*p == '\'') {
			// skip '' which is escaped quote in SQL (PostgreSQL-style)
			if (p[1] == '\'') p++;
			// handle MySQL-style escaped quote \'
			else if (p > input && *(p-1) == '\\') {}
			else inQuote = !inQuote;

		} else if (!inQuote) {
			if (*p == '(') openParens++;
			else if (*p == ')') openParens--;

			if (openParens < 0) return false;
		}
	p++;
	}
	return openParens == 0 && !inQuote;
}

// Check whether the number of column names matches column definitions
bool checkBalancedColNameToTypeInsideParens(char* input) {
	int openParens= 0;
	int numNames = 0;
	int numTypes = 0;

	char* p = input;

	while (*p != '(') p++;
	if (!p) return false;

	p++; // skip '('
	openParens++;

	while (openParens > 0) {
		// move to colName
		while (isspace(*p)) p++;

		if (*p == ')') {
			openParens--;
			break;
		}

		// start of column definition
		char* tokenStart = p;
		bool inQuote = 0;

		// move until comma or closing paren at top level
		while (*p && (inQuote || (*p != ',' && *p != ')'))) {
			if (*p == '\'') inQuote = !inQuote;
			else if (*p == '(' && !inQuote) openParens++;
			else if (*p == '(' && !inQuote) openParens--;
			p++;
		}

		size_t len = p - tokenStart;
		char* colDef = calloc(len + 1, 1);
		strncpy(colDef, tokenStart, len);

		// count number of space-separated tokens
		int wordCount = 0;
		char* tok = strtok(colDef, " \t\r\n");
		while (tok) {
			wordCount++;
			tok = strtok(NULL, " \t\r\n");
		}

		free(colDef);

		if (wordCount < 2) {
			printf("Invalid column definition (needs name and type)\n");
			return false;
		}

		if (*p == ',') p++; // skip comma
	}

	return true;
}

/* PARSING FUNCTIONS */
ColPair* createTable_extractColumns(char** pPtr, int* colCount) {
	// copy address in original p to local variable p
	char* p = *pPtr;

	// Find the start of columns
	while (*p != '(') p++;
	if (!p) NULL;

	p++; // skip '('

	int openParens = 1;
	int capacity = 4;
	int count = 0;

	ColPair* columns = malloc(capacity * sizeof(ColPair));

	while (openParens > 0) {

		// skip leading whitespace
		while (isspace(*p)) p++;

		// if closing paren, finish
		if (*p == ')') {
			openParens--;
			break;
		}

		// start of column definition
		char* tokenStart = p;
		bool inQuote = 0;

		// move until comma or closing paren at top level
		while (*p && (inQuote || (*p != ',' && openParens != 0))) {

			if (*p == '\'') inQuote = !inQuote;
			else if (*p == '(' && !inQuote) openParens++;
			else if (*p == ')' && !inQuote) openParens--;

			// p++ -> fix a bug where no space left before final parens eg ( id INT)
			// if p++ on final ) it will be included in the column definition erroneously
			if (openParens > 0) p++;
		}

		size_t len = p - tokenStart;
		// colDef is eg "id INT NOT NULL PRIMARY KEY"
		char* colDef = calloc(len + 1, 1);
		strncpy(colDef, tokenStart, len);

		// "id"
		char* name = strtok(colDef, " \t\r\n");

		// collect all remaining column definitions ie INT NOT NULL AUTO_INCREMENT PRIMARY KEY
		// this process will replace any whitespace with spaces
		size_t defLen = 0;
		char* defParts[20];  // Arbitrary max token count
		int i = 0;

		while (1) {
			char* token = strtok(NULL, " \t\r\n");
			if (!token) break;
				defParts[i++] = token;
				defLen += strlen(token) + 1;  // +1 for space
		}

		// Join the remaining tokens into a single definition string
		char* definition = malloc(defLen);
		definition[0] = '\0';

		for (int j = 0; j < i; j++) {
				strcat(definition, defParts[j]);
				// add space between all except the end
				if (j < i - 1) strcat(definition, " ");
		}

		if (count >= capacity) {
			capacity *= 2;
			columns = realloc(columns, capacity * sizeof(ColPair));
		}

		columns[count].colName = strdup(name);
		columns[count].colDef = strdup(definition);
		count++;

		free(colDef);

		if (*p == ',') p++; // skip comma
	}
	// move off block parens
	p++;

	// remove whitespace between end of parens and whatever comes next
	// eg  ")  ," (multiple rows of columns) or ")   ;" (the end)
	while (isspace(*p)) p++;

	// return current position
	*pPtr = p;
	*colCount = count;
	return columns;
}

char** insertInto_extractColumns(char** pPtr, int* colCount) {

	// copy address in original p to local variable p
	char* p = *pPtr;

	// Find the start of columns
	while (*p != '(') p++;
	if (!p) NULL;

	p++; // skip '('

	int openParens = 1;
	int capacity = 4;
	int count = 0;

	char** columns = malloc(capacity * sizeof(char*));

	while (openParens > 0) {

		// skip leading whitespace
		while (isspace(*p)) p++;

		// if closing paren, finish
		if (*p == ')') {
			openParens--;
			break;
		}

		// start of column definition
		char* tokenStart = p;
		bool inQuote = 0;

		// move until comma or closing paren at top level
		while (*p && (inQuote || (*p != ',' && openParens != 0))) {

			if (*p == '\'') inQuote = !inQuote;
			else if (*p == '(' && !inQuote) openParens++;
			else if (*p == ')' && !inQuote) openParens--;

			// p++ -> fix a bug where no space left before final parens eg ( id INT)
			// if p++ on final ) it will be included in the column definition erroneously
			if (openParens > 0) p++;
		}

		size_t len = p - tokenStart;
		char* col = calloc(len + 1, 1);
		strncpy(col, tokenStart, len);
		trimWhitespace(col);

		if (count >= capacity) {
			capacity *= 2;
			columns = realloc(columns, capacity * sizeof(char*));
		}

		// check for string, transform inner '' to '
		if (*col == '\'' && strstr(col, "''")) {
			char* result = replaceEscapedQuote(col);
			strcpy(col, result);
			free(result);
		}

		// update length as it may have shrunk
		len = strlen(col);

		// check for string, remove outer ' ... '
		if (len >= 2 && col[0] == '\'' && col[len - 1] == '\'') {
			memmove(col, col + 1, len - 2);
			col[len - 2] = '\0';
		}

		columns[count] = col;

		count++;

		if (*p == ',') p++; // skip comma
	}
	// move off block parens
	p++;

	// remove whitespace between end of parens and whatever comes next
	// eg  ")  ," (multiple rows of columns) or ")   ;" (the end)
	while (isspace(*p)) p++;

	// return current position
	*pPtr = p;

	*colCount = count;
	return columns;
}

Where_Clause* where_extractColumns(char** pPtr, int* colCount) {
	// copy address in original p to local variable p
	char* p = *pPtr;

	int count = 0;

	Where_Clause* columns = malloc(3 * sizeof(char*));

	/* COLUMN NAME */
	while (isspace(*p)) p++;

	// start of column definition
	char* colNameStart = p;

	if (*p == ';') {
		// printf("no colName in where clause\n");
		return NULL;
	}

	while (!isspace(*p) && !strchr("=<>!;", *p)) {
		p++;
	}

	size_t colNameLen = p - colNameStart;
	char* colName = calloc(colNameLen + 1, 1);
	strncpy(colName, colNameStart, colNameLen);

	columns->w_column = colName;

	count++;

	/* OPERATOR */
	while (isspace(*p)) p++;
	char* opStart = p;

	if (*p == ';') {
		// printf("no operator in where clause\n");
		return NULL;
	}

	while (strchr("=<>!", *p)) p++;

	size_t opLen = p - opStart;
	char* operator = calloc(opLen + 1, 1);
	strncpy(operator, opStart, opLen);

	columns->w_operator = operator;

	count++;

	/* VALUE */
	while (isspace(*p)) p++;

	if (*p == ';') {
		// printf("no colValue in where clause\n");
		return NULL;
	}

	char* valStart = p;
	while (*p != ';') p++;

	size_t valLen = p - valStart;
	char* val = calloc(valLen + 1, 1);
	strncpy(val, valStart, valLen);

	columns->w_value = val;

	count++;

	// return current position
	*pPtr = p;

	*colCount = count;
	return columns;
}

char** select_extractColumns(char** pPtr, int* colCount) {

	// copy address in original p to local variable p
	char* p = *pPtr;

	int capacity = 4;
	int count = 0;

	char** columns = malloc(capacity * sizeof(char*));

	while (1) {

		// skip leading whitespace
		while (isspace(*p)) p++;

		// start of column definition
		char* tokenStart = p;

		while (!isspace(*p) && *p != ',' && *p != ';') p++;

		size_t len = p - tokenStart;
		char* col = calloc(len + 1, 1);
		strncpy(col, tokenStart, len);

		if (strncmp(col, "FROM", 4) == 0) {
			break;
		}

		if (*p == ';') {
			printf("Where clause has finished\n");
			break;
		}

		if (count >= capacity) {
			capacity *= 2;
			columns = realloc(columns, capacity * sizeof(char*));
		}

		columns[count] = col;

		count++;

		if (*p == ',') p++; // skip comma

		if (!*p) break;
	}

	while (isspace(*p)) p++;

	// return current position
	*pPtr = p;

	*colCount = count;
	return columns;
}

// Extracts the first string after a command
char* extractTableName(char** pPtr) {
	// copy address in original p to local variable p
	char* p = *pPtr;

	while (isspace(*p)) p++;
	char* start = p;
	while (!isspace(*p) && *p != '(' && *p != ';') p++;
	char* end = p;

	size_t len = end - start;
	char* tableName = malloc(len + 1);
	memcpy(tableName, start, len);
	tableName[len] = '\0';

  // move pointer to first non-whitespace
	while (isspace(*p)) p++;
	
	// update original pointer in the caller
	*pPtr = p;
	return tableName;
}

void parseCreate(char* input, Command* command) { 

	if (!checkBalancedColNameToTypeInsideParens(input)) {
		command->type = CMD_ERROR;
		command->e_message = writeError("Syntax error: Mismatch in column names and types.");
		return;	
	}

  char* p = input + strlen("CREATE TABLE");

	command->tableName = extractTableName(&p);

	int c_numColPairs;
	command->c_colPairs = createTable_extractColumns(&p, &c_numColPairs);
	command->c_numColPairs = c_numColPairs;
}

void parseInsert(char* input, Command* command) {

	char* p = input + strlen("INSERT INTO");

	command->tableName = extractTableName(&p);

	if (*p == 'V') {
		command->type = CMD_ERROR;
		command->e_message = writeError("Syntax error: columns can't be skipped yet.");
		return;
	}

	/* get the column names first */
	int i_numColNames = 0;
	char** i_colNames = insertInto_extractColumns(&p, &i_numColNames);
	command->i_numColNames = i_numColNames;
	command->i_colNames = i_colNames;

	/* loop until ; to get all values*/
	int colValueRowsCapacity = 4;
	command->i_colValueRows = calloc(sizeof(char*), colValueRowsCapacity);

	int i_numValueRows = 0;
	while (*p && *p != ';') {
		int valCount = 0;
		char** colValueRow = insertInto_extractColumns(&p, &valCount);

		if (i_numValueRows >= colValueRowsCapacity) {
			colValueRowsCapacity *= 2;
			command->i_colValueRows = realloc(command->i_colValueRows, colValueRowsCapacity * sizeof(char*));
		}

		command->i_colValueRows[i_numValueRows] = colValueRow;

		i_numValueRows++;
		p++;
	}
	command->i_numValueRows = i_numValueRows;
}

void parseSelect(char* input, Command* command) {

	char* p = input + strlen("SELECT");

	/* get the column names first */
	int s_colNameCount = 0;
	char** s_colNames = select_extractColumns(&p, &s_colNameCount);
	command->s_colNameCount = s_colNameCount;
	command->s_colNames = s_colNames;

	if (s_colNameCount == 1 && (strcmp(s_colNames[0], "*") == 0)) {
		command->s_all = true;
	}

	command->tableName = extractTableName(&p);

	if (*p == ';') return;

	// skip WHERE
	char* WHERE = extractTableName(&p);
	if (strcmp(WHERE, "WHERE") != 0) {
		command->type = CMD_ERROR;
		command->e_message = writeError("Syntax error: where was WHERE?.");
		free(WHERE);
		return;
	}
	free(WHERE);

	int w_clauseCount = 0;
	command->s_whereClause = where_extractColumns(&p, &w_clauseCount);

	if (!command->s_whereClause) {
		command->type = CMD_ERROR;
		command->e_message = writeError("Syntax error: error in where clause.");
		return;
	}
}

char* getInput() {
  char buffer[128];
  char* input = calloc(INPUT_LENGTH, 1);
  if (!input) return NULL;

  while (fgets(buffer, sizeof(buffer), stdin)) {
    strcat(input, buffer);
    if (strchr(buffer, ';')) {
      break;
    }
    printf("  -> ");
  }

  // remove all newlines
  for (int i = 0; input[i]; i++) {
    if (input[i] == '\n') input[i] = ' ';
  }

  // validation to ensure use input was not too long
  return input;
}

Command* parseInput(char* input) {
  Command* command = calloc(sizeof(Command), 1);
  command->type = CMD_UNDEFINED;

  // Remove whitespace
  trimWhitespace(input);

	// check that equal numbers of [()'] (unless inside quotes)
	if (!checkBalancedParensAndQuotes(input)) {
		command->type = CMD_ERROR;
		command->e_message = writeError("Syntax error: parens or quotes are not balanced.");
		return command;
	}

	// Check that string ends in ;
	if (input[strlen(input) - 1] != ';') {
		command->type = CMD_ERROR;
		command->e_message = writeError("Syntax error: input does not end in ;");
		return command;
	}

  if (strncmp(input, "CREATE TABLE ", 13) == 0) {
    command->type = CMD_CREATE;
    parseCreate(input, command);
  }

  if (strncmp(input, "INSERT INTO ", 12) == 0) {
    command->type = CMD_INSERT;
    parseInsert(input, command);
  }

    if (strncmp(input, "SELECT ", 7) == 0) {
    command->type = CMD_SELECT;
    parseSelect(input, command);
  }

  if (strncmp(input, "exit", 4) == 0) {
    command->type = CMD_EXIT;
  }

//   free(input);

  return command;
}
