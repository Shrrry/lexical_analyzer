#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 100

typedef struct {
    char name[50];
    char type[20];
} Symbol;

Symbol table[MAX];
int count = 0;

// Error storage
char lexical[50][200];
char syntaxErr[50][200];
char semantic[50][200];

int lcount = 0;
int scount = 0;
int semcount = 0;


// ---------------- UTILITY FUNCTIONS ----------------
void trim(char *s) {
    int i = 0, j = strlen(s) - 1;

    while (isspace(s[i])) i++;
    while (j >= i && isspace(s[j])) j--;

    int k = 0;
    for (int x = i; x <= j; x++)
        s[k++] = s[x];

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
    strcpy(table[count].name, name);
    strcpy(table[count].type, type);
    count++;
}

int isInteger(char *s) {
    if (strlen(s) == 0) return 0;

    for (int i = 0; s[i]; i++) {
        if (!isdigit(s[i]))
            return 0;
    }

    return 1;
}

int isFloat(char *s) {
    int dot = 0;

    for (int i = 0; s[i]; i++) {
        if (s[i] == '.')
            dot++;
        else if (!isdigit(s[i]))
            return 0;
    }

    return dot == 1;
}

int isString(char *s) {
    return strchr(s, '"') != NULL ||
           strchr(s, '\'') != NULL;
}


// ---------------- MAIN ----------------
int main() {

    char line[200][200];
    int n = 0;

    printf("Enter Python code (~ to stop):\n");

    while (1) {
        fgets(line[n], 200, stdin);

        if (line[n][0] == '~')
            break;

        n++;
    }


    // ---------------- LEXICAL + SYNTAX ----------------
    for (int i = 0; i < n; i++) {

        char t[200];
        strcpy(t, line[i]);
        trim(t);

        if (strlen(t) == 0)
            continue;

        // skip print statements
        if (strncmp(t, "print(", 6) != 0 &&
            strchr(t, '=')) {

            char left[100];
            sscanf(t, "%[^=]", left);
            trim(left);

            if (!isValidIdentifier(left)) {
                sprintf(
                    lexical[lcount++],
                    "%s -> invalid identifier",
                    left
                );
            }
        }

        // missing colon
        if ((strncmp(t, "if ", 3) == 0 ||
             strncmp(t, "for ", 4) == 0 ||
             strncmp(t, "while ", 6) == 0 ||
             strncmp(t, "def ", 4) == 0) &&
            t[strlen(t)-1] != ':') {

            sprintf(
                syntaxErr[scount++],
                "%s -> missing ':'",
                t
            );
        }
    }


    // ---------------- SEMANTIC / RUNTIME ----------------
    for (int i = 0; i < n; i++) {

        char t[200];
        strcpy(t, line[i]);
        trim(t);

        if (strlen(t) == 0)
            continue;

        if (strncmp(t, "if ", 3) == 0 ||
            strncmp(t, "for ", 4) == 0 ||
            strncmp(t, "while ", 6) == 0 ||
            strncmp(t, "def ", 4) == 0)
            continue;


        // ---------------- PRINT CHECK ----------------
        if (strncmp(t, "print(", 6) == 0) {

            char content[100];
            sscanf(t, "print(%[^)])", content);
            trim(content);

            // print(variable)
            if (!strchr(content, ',')) {

                if (!isString(content) &&
                    findVar(content) == -1) {

                    sprintf(
                        semantic[semcount++],
                        "%s -> undefined variable",
                        content
                    );
                }
            }

            // print("text", variable)
            else {

                char var[100];
                sscanf(
                    strrchr(content, ',') + 1,
                    "%s",
                    var
                );

                trim(var);

                if (findVar(var) == -1) {
                    sprintf(
                        semantic[semcount++],
                        "%s -> undefined variable",
                        var
                    );
                }
            }

            continue;
        }


        // invalid type conversion
        if (strstr(t, "= int(\"")) {

            char left[50];
            sscanf(t, "%[^=]", left);
            trim(left);

            sprintf(
                semantic[semcount++],
                "%s -> invalid type conversion",
                left
            );

            continue;
        }


        // ---------------- ASSIGNMENT ----------------
        if (strchr(t, '=')) {

            char left[50], right[100];

            sscanf(
                t,
                "%[^=]=%[^\n]",
                left,
                right
            );

            trim(left);
            trim(right);

            if (!isValidIdentifier(left))
                continue;


            // string + int mismatch
            if (strstr(right, "+")) {

                char op1[50], op2[50];

                sscanf(
                    right,
                    "%[^+]+%s",
                    op1,
                    op2
                );

                trim(op1);
                trim(op2);

                if ((isString(op1) && isInteger(op2)) ||
                    (isInteger(op1) && isString(op2))) {

                    sprintf(
                        semantic[semcount++],
                        "%s -> type mismatch",
                        left
                    );

                    continue;
                }
            }


            // store variable
            if (findVar(left) == -1) {

                if (isInteger(right))
                    addVar("int", left);

                else if (isFloat(right))
                    addVar("float", left);

                else if (isString(right))
                    addVar("string", left);

                else
                    addVar("unknown", left);
            }
        }
    }


    // ---------------- OUTPUT ----------------
    printf("\n========= ANALYSIS =========\n");

    if (lcount > 0) {
        printf("\nLexical Errors:\n");
        for (int i = 0; i < lcount; i++) {
            printf("%s\n", lexical[i]);
        }
    }

    if (scount > 0) {
        printf("\nSyntax Errors:\n");
        for (int i = 0; i < scount; i++) {
            printf("%s\n", syntaxErr[i]);
        }
    }

    if (semcount > 0) {
        printf("\nSemantic/Runtime Errors:\n");
        for (int i = 0; i < semcount; i++) {
            printf("%s\n", semantic[i]);
        }
    }

    if (lcount == 0 &&
        scount == 0 &&
        semcount == 0) {

        printf("\nNo errors found.\n");
    }

    printf("\nPython interpretation phases completed.\n");

    return 0;
}