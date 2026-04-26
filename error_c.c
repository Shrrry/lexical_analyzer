#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX 100

typedef struct {
    char name[50];
    char type[20];
} Symbol;

Symbol table[MAX];
int count = 0;


// ---------------- ERROR STORAGE ----------------
char lexical[50][200];
char syntaxErr[50][200];
char semantic[50][200];

int lcount = 0;
int scount = 0;
int semcount = 0;


// ---------------- UTILITY FUNCTIONS ----------------
void trim(char *s) {
    int i = 0;
    int j = strlen(s) - 1;

    while (isspace(s[i])) i++;
    while (j >= i && isspace(s[j])) j--;

    int k = 0;
    for (int x = i; x <= j; x++) {
        s[k++] = s[x];
    }

    s[k] = '\0';
}


int isValidIdentifier(char *s) {
    if (!isalpha(s[0]) && s[0] != '_')
        return 0;

    for (int i = 1; s[i]; i++) {
        if (!isalnum(s[i]) && s[i] != '_')
            return 0;
    }

    return 1;
}


int findVar(char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(table[i].name, name) == 0)
            return i;
    }
    return -1;
}


void addVar(char *type, char *name) {
    if (findVar(name) != -1) {
        sprintf(
            semantic[semcount++],
            "%s → multiple declaration",
            name
        );
        return;
    }

    strcpy(table[count].name, name);
    strcpy(table[count].type, type);
    count++;
}


int isInteger(char *v) {
    for (int i = 0; v[i]; i++) {
        if (!isdigit(v[i]))
            return 0;
    }
    return 1;
}


int isFloat(char *v) {
    int dot = 0;

    for (int i = 0; v[i]; i++) {
        if (v[i] == '.') {
            dot++;
        }
        else if (!isdigit(v[i])) {
            return 0;
        }
    }

    return dot == 1;
}


int containsString(char *e) {
    return strchr(e, '"') != NULL;
}


// ---------------- MAIN ----------------
int main() {

    char line[200][200];
    int n = 0;

    printf("Enter C code (~ to stop):\n");

    while (1) {
        fgets(line[n], 200, stdin);

        if (line[n][0] == '~')
            break;

        n++;
    }

    int bracket = 0;
    int paren = 0;


    // ---------------- SYNTAX CHECK ----------------
    for (int i = 0; i < n; i++) {

        char t[200];
        strcpy(t, line[i]);

        trim(t);

        if (strlen(t) == 0)
            continue;

        for (int j = 0; t[j]; j++) {
            if (t[j] == '{')
                bracket++;

            if (t[j] == '}')
                bracket--;

            if (t[j] == '(')
                paren++;

            if (t[j] == ')')
                paren--;
        }


        // main function syntax error
        if (strstr(t, "main") &&
            strchr(t, '{') &&
            !strchr(t, ')')) {

            sprintf(
                syntaxErr[scount++],
                "int main( { → missing )"
            );

            sprintf(
                syntaxErr[scount++],
                "Incorrect function declaration syntax"
            );
        }


        // missing semicolon
        if (t[0] != '#' &&
            !strchr(t, '{') &&
            !strchr(t, '}') &&
            !strstr(t, "main") &&
            !strstr(t, "if") &&
            !strstr(t, "while") &&
            !strstr(t, "for") &&
            t[strlen(t)-1] != ';') {

            sprintf(
                syntaxErr[scount++],
                "%s → missing semicolon",
                t
            );
        }
    }


    // unmatched braces
    if (bracket != 0) {
        sprintf(
            syntaxErr[scount++],
            "Mismatched curly braces"
        );
    }


    // unmatched parentheses
    if (paren != 0) {
        sprintf(
            syntaxErr[scount++],
            "Mismatched parentheses"
        );
    }



    // ---------------- LEXICAL + SEMANTIC ----------------
    for (int i = 0; i < n; i++) {

        char t[200];
        strcpy(t, line[i]);

        trim(t);

        if (strlen(t) == 0)
            continue;

        if (t[0] == '#')
            continue;

        if (strstr(t, "main") ||
            strstr(t, "printf") ||
            strstr(t, "scanf") ||
            strstr(t, "return"))
            continue;


        // ---------------- DECLARATION ----------------
        if (strncmp(t, "int ", 4) == 0 ||
            strncmp(t, "float ", 6) == 0 ||
            strncmp(t, "char ", 5) == 0) {

            char type[20];
            char vars[200];

            sscanf(t, "%s %[^\n]", type, vars);

            vars[strcspn(vars, ";")] = '\0';

            char *token = strtok(vars, ",");

            while (token != NULL) {

                trim(token);

                char var[50];
                char val[100];

                val[0] = '\0';

                if (strchr(token, '=')) {
                    sscanf(
                        token,
                        "%[^=]=%s",
                        var,
                        val
                    );

                    trim(var);
                    trim(val);
                }
                else {
                    strcpy(var, token);
                }


                // lexical error
                if (!isValidIdentifier(var)) {
                    sprintf(
                        lexical[lcount++],
                        "%s → invalid identifier",
                        var
                    );
                }
                else {
                    addVar(type, var);
                }


                // type mismatch check
                if (strlen(val) > 0) {

                    if (strcmp(type, "int") == 0 &&
                        !isInteger(val)) {

                        sprintf(
                            semantic[semcount++],
                            "%s → type mismatch",
                            token
                        );
                    }

                    if (strcmp(type, "float") == 0 &&
                        !(isFloat(val) || isInteger(val))) {

                        sprintf(
                            semantic[semcount++],
                            "%s → type mismatch",
                            token
                        );
                    }
                }

                token = strtok(NULL, ",");
            }
        }


        // ---------------- ASSIGNMENT ----------------
        else if (strchr(t, '=')) {

            char left[50];
            char right[100];

            sscanf(
                t,
                "%[^=]=%[^;]",
                left,
                right
            );

            trim(left);
            trim(right);


            if (findVar(left) == -1) {
                sprintf(
                    semantic[semcount++],
                    "%s → undeclared variable",
                    t
                );
                continue;
            }


            if (containsString(right)) {

                if (strstr(right, "+")) {
                    sprintf(
                        semantic[semcount++],
                        "%s → invalid operation",
                        t
                    );
                }
                else {
                    sprintf(
                        semantic[semcount++],
                        "%s → type mismatch",
                        t
                    );
                }
            }
        }
    }



    // ---------------- FINAL OUTPUT ----------------
    printf("\n========= ANALYSIS =========\n\n");


    if (lcount > 0) {
        printf("Lexical Errors:\n");

        for (int i = 0; i < lcount; i++) {
            printf("%s\n", lexical[i]);
        }

        printf("\n");
    }


    if (scount > 0) {
        printf("Syntax Errors:\n");

        for (int i = 0; i < scount; i++) {
            printf("%s\n", syntaxErr[i]);
        }

        printf("\n");
    }


    if (semcount > 0) {
        printf("Semantic Errors:\n");

        for (int i = 0; i < semcount; i++) {
            printf("%s\n", semantic[i]);
        }

        printf("\n");
    }


    if (lcount == 0 &&
        scount == 0 &&
        semcount == 0) {

        printf("No Errors Found.\n");
    }

    printf("\nAll phases completed.\n");

    return 0;
}