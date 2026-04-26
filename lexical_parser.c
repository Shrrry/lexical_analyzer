#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX 200

struct Token {
    char type[20];
    char value[50];
};

struct Symbol {
    char name[50];
    char type[20];
};

struct Node {
    char value[50];
    struct Node *left, *right;
};

// ✅ FUNCTION DECLARATIONS (IMPORTANT FIX)
void printTree(struct Node* root, int depth);

struct Token tokens[MAX];
int tokenCount = 0;

struct Symbol symtab[MAX];
int symCount = 0;

struct Node* newNode(char *val) {
    struct Node* n = (struct Node*)malloc(sizeof(struct Node));
    strcpy(n->value, val);
    n->left = n->right = NULL;
    return n;
}

char *keywords[] = {
    "int","float","char","if","else","while","for","return","void"
};

int isKeyword(char *str) {
    int n = sizeof(keywords)/sizeof(keywords[0]);
    for(int i=0;i<n;i++)
        if(strcmp(str,keywords[i])==0) return 1;
    return 0;
}

void addToken(char *type, char *value) {
    strcpy(tokens[tokenCount].type, type);
    strcpy(tokens[tokenCount].value, value);
    tokenCount++;
}

void addSymbol(char *name, char *type) {
    for(int i=0;i<symCount;i++)
        if(strcmp(symtab[i].name,name)==0) return;

    strcpy(symtab[symCount].name,name);
    strcpy(symtab[symCount].type,type);
    symCount++;
}

void tokenize(char *input) {
    int i=0;
    char buffer[50];

    while(input[i] != '\0') {

        if(isalpha(input[i])) {
            int j=0;
            while(isalnum(input[i]))
                buffer[j++] = input[i++];
            buffer[j]='\0';

            if(isKeyword(buffer))
                addToken("KEYWORD", buffer);
            else {
                addToken("IDENTIFIER", buffer);
                addSymbol(buffer, "int");
            }
        }

        else if(isdigit(input[i])) {
            int j=0;
            while(isdigit(input[i]))
                buffer[j++] = input[i++];
            buffer[j]='\0';

            addToken("NUMBER", buffer);
        }

        else if(strchr("+-*/=", input[i])) {
            buffer[0]=input[i];
            buffer[1]='\0';
            addToken("OPERATOR", buffer);
            i++;
        }

        else if(strchr(";(){}", input[i])) {
            buffer[0]=input[i];
            buffer[1]='\0';
            addToken("SYMBOL", buffer);
            i++;
        }

        else if(isspace(input[i])) i++;
        else i++;
    }
}

struct Node* buildExpression(int start, int end) {
    if (start > end) return NULL;

    if (start == end)
        return newNode(tokens[start].value);

    for (int i = end; i >= start; i--) {
        if (strcmp(tokens[i].type, "OPERATOR") == 0 &&
            strcmp(tokens[i].value, "=") != 0) {

            struct Node* node = newNode(tokens[i].value);
            node->left = buildExpression(start, i - 1);
            node->right = buildExpression(i + 1, end);
            return node;
        }
    }

    return newNode(tokens[start].value);
}

void parseAll() {
    int i = 0;
    int treeNum = 1;

    while (i < tokenCount - 1) {

        if (strcmp(tokens[i].type, "IDENTIFIER") == 0 &&
            strcmp(tokens[i+1].value, "=") == 0) {

            struct Node* root = newNode("=");
            root->left = newNode(tokens[i].value);

            int j = i + 2;
            while (j < tokenCount && strcmp(tokens[j].value, ";") != 0) {
                j++;
            }

            root->right = buildExpression(i + 2, j - 1);

            printf("\nPARSE TREE %d\n", treeNum++);
            printTree(root, 0);

            i = j + 1;
        } else {
            i++;
        }
    }
}

void printTree(struct Node* root, int depth) {
    if (root == NULL) return;

    for (int i = 0; i < depth; i++)
        printf("    ");

    printf("|-- %s\n", root->value);

    printTree(root->left, depth + 1);
    printTree(root->right, depth + 1);
}

int main() {

    setbuf(stdout, NULL);

    char input[1000], temp[200];
    input[0]='\0';

    while(1) {
        fgets(temp, sizeof(temp), stdin);
        if(temp[0]=='~') break;
        strcat(input,temp);
    }

    tokenize(input);

    printf("----- TOKENS -----\n");
    for(int i=0;i<tokenCount;i++)
        printf("%s -> %s\n", tokens[i].value, tokens[i].type);

   printf("\n=========== SYMBOL TABLE ===========\n\n");
printf("%-8s %-20s %-15s\n", "Index", "Name", "Type");
printf("------------------------------------------------\n");

for(int i=0; i<symCount; i++) {
    printf("%-8d %-20s %-15s\n",
           i + 1,
           symtab[i].name,
           symtab[i].type);
}

printf("====================================\n");

    parseAll();

    return 0;
}