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

char lexical[50][200];
char syntaxErr[50][200];
char semantic[50][200];

int lcount = 0;
int scount = 0;
int semcount = 0;


// ---------------- UTIL ----------------
void trim(char *s) {
    int i=0,j=strlen(s)-1;

    while(isspace(s[i])) i++;
    while(j>=i && isspace(s[j])) j--;

    int k=0;
    for(int x=i;x<=j;x++)
        s[k++]=s[x];

    s[k]='\0';
}

int isValidIdentifier(char *s){
    if(!isalpha(s[0]) && s[0]!='_')
        return 0;

    for(int i=1;s[i];i++){
        if(!isalnum(s[i]) && s[i]!='_')
            return 0;
    }
    return 1;
}

int findVar(char *name){
    for(int i=0;i<count;i++){
        if(strcmp(table[i].name,name)==0)
            return i;
    }
    return -1;
}

void addVar(char *type,char *name){
    strcpy(table[count].name,name);
    strcpy(table[count].type,type);
    count++;
}

int isInteger(char *s){
    if(strlen(s)==0) return 0;

    for(int i=0;s[i];i++){
        if(!isdigit(s[i]))
            return 0;
    }
    return 1;
}

int isFloat(char *s){
    int dot=0;

    for(int i=0;s[i];i++){
        if(s[i]=='.')
            dot++;
        else if(!isdigit(s[i]))
            return 0;
    }

    return dot==1;
}

int containsString(char *s){
    return strchr(s,'"')!=NULL;
}


// checks arithmetic expression like a+b
int isArithmeticExpression(char *expr){
    if(strchr(expr,'+') ||
       strchr(expr,'-') ||
       strchr(expr,'*') ||
       strchr(expr,'/')){
        return 1;
    }
    return 0;
}


// ---------------- MAIN ----------------
int main(){

    char line[200][200];
    int n=0;

    printf("Enter Java code (~ to stop):\n");

    while(1){
        fgets(line[n],200,stdin);

        if(line[n][0]=='~')
            break;

        n++;
    }

    int braces=0;


    // ---------------- SYNTAX ----------------
    for(int i=0;i<n;i++){
        char t[200];
        strcpy(t,line[i]);
        trim(t);

        if(strlen(t)==0)
            continue;

        for(int j=0;t[j];j++){
            if(t[j]=='{') braces++;
            if(t[j]=='}') braces--;
        }

        if(strstr(t,"class") && !strchr(t,'{')){
            sprintf(syntaxErr[scount++],
            "%s -> invalid class declaration",t);
        }

        if(strstr(t,"main") && !strstr(t,"String[] args")){
            sprintf(syntaxErr[scount++],
            "%s -> invalid main method declaration",t);
        }

        if(t[0]!='{' &&
           t[0]!='}' &&
           !strstr(t,"class") &&
           !strstr(t,"main") &&
           !strchr(t,'{') &&
           t[strlen(t)-1]!=';'){

            sprintf(syntaxErr[scount++],
            "%s -> missing semicolon",t);
        }
    }

    if(braces!=0){
        sprintf(syntaxErr[scount++],
        "Unbalanced braces in program");
    }


    // ---------------- SEMANTIC ----------------
    for(int i=0;i<n;i++){
        char t[200];
        strcpy(t,line[i]);
        trim(t);

        if(strlen(t)==0)
            continue;

        if(strstr(t,"class") ||
           strstr(t,"main") ||
           strstr(t,"System.out.println") ||
           strstr(t,"return"))
            continue;


        // declaration
        if(strncmp(t,"int ",4)==0 ||
           strncmp(t,"float ",6)==0 ||
           strncmp(t,"double ",7)==0 ||
           strncmp(t,"String ",7)==0){

            char type[20],var[50],val[100];

            sscanf(t,"%s",type);

            if(strchr(t,'=')){
                sscanf(t,"%*s %s = %[^;]",var,val);
                trim(val);
            }
            else{
                sscanf(t,"%*s %[^;]",var);
                val[0]='\0';
            }

            trim(var);

            if(!isValidIdentifier(var)){
                sprintf(lexical[lcount++],
                "%s -> invalid identifier",var);
                continue;
            }

            if(findVar(var)!=-1){
                sprintf(semantic[semcount++],
                "%s -> multiple declaration",var);
                continue;
            }

            addVar(type,var);
        }


        // assignment
        else if(strchr(t,'=')){

            char left[50],right[100];

            sscanf(t,"%[^=]=%[^;]",left,right);

            trim(left);
            trim(right);

            int idx=findVar(left);

            if(idx==-1){
                sprintf(semantic[semcount++],
                "%s -> undeclared variable",left);
                continue;
            }

            char varType[20];
            strcpy(varType,table[idx].type);


            // FIXED PART
            if(strcmp(varType,"int")==0){

                if(isInteger(right)){
                    continue;
                }

                if(isArithmeticExpression(right)){
                    continue;
                }

                if(findVar(right)!=-1){
                    continue;
                }

                sprintf(semantic[semcount++],
                "%s -> type mismatch",left);
            }


            else if(strcmp(varType,"String")==0 &&
                    !containsString(right)){

                sprintf(semantic[semcount++],
                "%s -> type mismatch",left);
            }
        }
    }


    printf("\n========= ANALYSIS =========\n");

    if(lcount){
        printf("\nLexical Errors:\n");
        for(int i=0;i<lcount;i++)
            printf("%s\n",lexical[i]);
    }

    if(scount){
        printf("\nSyntax Errors:\n");
        for(int i=0;i<scount;i++)
            printf("%s\n",syntaxErr[i]);
    }

    if(semcount){
        printf("\nSemantic Errors:\n");
        for(int i=0;i<semcount;i++)
            printf("%s\n",semantic[i]);
    }

    if(lcount==0 && scount==0 && semcount==0){
        printf("\nNo errors found.\n");
    }

    printf("\nJava compilation phases completed.\n");

    return 0;
}