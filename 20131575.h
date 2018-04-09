#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define OPHASHSIZE	20	// Size of hash
#define INPUTSIZE	80	// size of input
#define MEMORYSIZE	16*65536	// size of memory
#define MNEMONIC_SIZE 10
#define HISTORY_CHAR 50
#define MEMORYLINE	16	// size of memory lines
#define ASCIISET	256	// range of ASCII code (0 ~ 256)
#define NUMSYS		16	
// proj2
#define SYMHASHSIZE	100	// size symbol table hash
#define OBJLINEMAX	60	// maximum length of a single line when printing obj

//opcode structure
typedef struct __OPCODE{
	int cd;
	char mn[MNEMONIC_SIZE];
	char fm[MNEMONIC_SIZE];
	struct __OPCODE *next;
} __opcode;

//history structure
typedef struct __HISTORY{
	char his[HISTORY_CHAR];
	struct __HISTORY *next;
} __history;

//directive structure
struct __DIRECTIVES {
	char dr[6];
	int size;
};
// save of register from menmonic
char reg[][3] = {
	"A", "X", "L", "B", "S", "T", "F", "", "PC", "SW"
};


// predeclared directive commands
struct __DIRECTIVES directive[] = {
	{"RESB", 1}, {"RESW", 3}, {"RESD", 6}, {"RESQ", 12}, 
	{"BYTE", 1}, {"WORD", 3}, {"DWORD", 6}, {"QWORD", 12}
};

//assembly code structure
typedef struct __ASMCODE {
	int line;
	char *inputOrig;
	char *input;
	char *varf;
	char *varr;
	char *op;
	int fmt;
	int adr;
	unsigned int loc;
	char* obc;
	struct __ASMCODE *next;
} __asmcode;

//symbol table structure
typedef struct __SYMTAB {
	char *symbol;
	unsigned int loc;
	struct __SYMTAB *next;
} __symtab;



//compare string
int str_icmp(char *, char *);




// capitalize uncapitalize functins
void str_upr(char *);
void str_lwr(char *);

//print help
void help_print(void);

//print directories
void dir_print(char *);

//save history
void history_input(__history **, char *);

//print history
void history_print(__history *);

//reset history
void history_free(__history *);

//read opcode
void opcode_read(__opcode **);

//return hash structure
int opcode_hashFuncition(char *);

//print menmonic number
__opcode* opcode_mnemonio(__opcode **, char *);

//print memory
void memory_dump(unsigned char *, long, long);

//edit memory
void memory_edit(unsigned char *, long, unsigned char);

//fill memory
void memory_fill(unsigned char *, long, long, unsigned char);
//reset memory
void memory_reset(unsigned char *);

//print opcode list
void opcode_print(__opcode **);


//print symbol
__symtab* assemble_symtabSearch(__symtab **, char *);

//save into symbol table
void assemble_symtabInput(__symtab **, char *, unsigned int);

//return memory of symtab
void assemble_symtabFree(__symtab **);



//print symbol table DESC
void symbol_print(__symtab **);


// symbol compare
int symbol_printCmp(const void *, const void *);


//return memory of opcode list
void opcode_free(__opcode **);



//read type of file and print
int type_print(char *);

//read assemble
int assemble_read(__asmcode *, char *);

//assemble pass1
unsigned int assemble_pass1(__asmcode *, __symtab **, __opcode **);

//assemble pass2
int assemble_pass2(__asmcode *, __symtab **, __opcode **);

//assemble result and make 1st file
int assemble_lstPrint(__asmcode *, char *);

//make obj by assemble
int assemble_objPrint(__asmcode *, __symtab **sm, char *, unsigned int);

// memory return from __asmcode structure
void assemble_free(__asmcode *);

//calculate needed has when making symbol table
unsigned int assemble_symtabHash(char *);

/*
 *
 * Functions down below
 *
 */


//capitalize string
void str_upr(char *s) {
  int length = strlen(s);
  int i;
  i=0;
  while(i < length) {
    if ('a' <= s[i] && s[i] <= 'z'){
      s[i] = s[i] - 'a' + 'A';
    }
    i++;
  }
}


void str_lwr(char *s) {
  int length = strlen(s);
  int i;
  i=0;
  while(i < length) {
    if ('A' <= s[i] && s[i] <= 'Z'){
      s[i] = s[i] - 'A' + 'a';
    }
    i++;
  }
}

int str_icmp(char *s1, char *s2) {
  int leng1 = strlen(s1);
  int leng2 = strlen(s2);
  int flag;
  if (leng1 != leng2){
    flag = 0;
    flag = leng2 - leng1;
    return flag;
  }

  else {
    char *ss1 = (char*)malloc(sizeof(char)*(leng1 + 1));
    char *ss2 = (char*)malloc(sizeof(char)*(leng2 + 1));
    int result;

    strcpy(ss2, s2);
    strcpy(ss1, s1);
    flag = 1;
    str_lwr(ss1);
    str_lwr(ss2);

    result = strcmp(ss1, ss2);

    free(ss1);
    free(ss2);

    return result;
  }
}

void help_print(void) {
  printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\n");
  printf("f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\nassemble filename\ntype filename\nsymbol\n");
}

void dir_print(char *ds) {
  DIR *dirp;
  struct dirent *direntp;
  struct stat dirbuff;
  int i;
  int flag=0;
  i = 0;

  dirp = opendir(ds);

  if (dirp == NULL) {
    printf("dir: No such directory\n");
    flag = -1;
    return;
  }

  printf("\t");
  while ((direntp = readdir(dirp)) != NULL) {
    if (i == 3) {
      flag = 1;
      printf("\n\t");
      flag = 1;
      i = 0;
    }

    lstat(direntp->d_name, &dirbuff);
    if (S_ISDIR(dirbuff.st_mode)) {
      flag = 0;
      if (strcmp(direntp->d_name, ".") && strcmp(direntp->d_name, "..")){
        flag = 1;
        printf("%s/\t", direntp->d_name);
      }

      else{
        flag = 1;
        continue;
      }
    }

    else if (dirbuff.st_mode & S_IXUSR){
      printf("%s*\t", direntp->d_name);
      flag = 1;
    }

    else{
      printf("%s\t", direntp->d_name);
      flag = 1;
    }
    flag = 0;
    i++;
    if(flag==-1){
      return;
    }
  }

  printf("\n");
  closedir(dirp);
}

void history_input(__history **h, char *s) {
  int flag = 0;
  __history* n = (__history*)malloc(sizeof(__history));

  strcpy(n->his, s);
  n->next = NULL;
  if(n->next == NULL){
    flag = 1;
  }
  else{
    flag = 0;
  }
  if(flag==-1)
  {
    return;
  }
  (*h)->next = n;
  (*h) = n;
}

void history_free(__history *h) {
  __history* n;

  while (h != NULL) {
    n = h->next;
    free(h);
    h = n;
  }
}
void history_print(__history *h) {
  int i;
  int flag = 0;


  i = 1;
  h = h->next;

  while (h != NULL) {
    flag = 1;
    printf("%d\t", i);
    printf("%s\n", h->his);
    if(flag==1){
      flag = 0;
    }
    i++;
    h = h->next;
  }
}



void memory_edit(unsigned char *m, long address, unsigned char value) {
  long index;
  index = address;
  m[index] = value;
}

void memory_fill(unsigned char *m, long start, long end, unsigned char value) {
  int i, flag;
  i=start;
  while(i <= end){
    flag = i;
    m[flag] = value;
    i++;
  }
  flag = 0;
}

void memory_dump(unsigned char *m, long start, long end) {
  int lineStart = start / MEMORYLINE;
  int lineEnd = end / MEMORYLINE;
  int flag;
  int index = start;
  int i, l, blankFront, blankRear;
  l = lineStart;
  while(l <= lineEnd) {
    printf("%04X0 ", l);

    if(l == lineStart) {
      blankFront = start - l * MEMORYLINE;
      i=0;
      while(i < blankFront){
        printf("   ");
        flag = 1;
        i = i + 1;
      }
    }
    i=index;
    while((i <= end) && (i < (l + 1) * MEMORYLINE)){
      printf("%02X ", m[i]);
      i++;
      flag = 1;
    }

    if(l == lineEnd) {
      blankRear = (l + 1) * MEMORYLINE - end - 1;
      i=0;
      while(i < blankRear){
        printf("   ");
        i++;
        flag = 1;
      }
    }

    printf("; ");
    if(l == lineStart) {
      i=0;
      while(i < blankFront){
        printf(".");
        i++;
        flag = 1;
      }
    }
    i = index;
    while((i <= end) && (i < (l + 1) * MEMORYLINE)){
      if(!(0x20 <= m[i] && m[i] <= 0x7E)){
        printf(".");
        flag = 1;
      }
      else{
        printf("%c", m[i]);
        flag = 1;
      }
      i++;
    }

    if(l == lineEnd) {
      i=0;
      while(i < blankRear){
        printf(".");
        flag = 1;
        i++;
      }
    }

    printf("\n");
    index = (l + 1) * MEMORYLINE;
    l++;
    flag = 0;
  }
  if(flag==-3){
    return;
  }
}

void memory_reset(unsigned char *m) {
  int i;
  i=0;
  while(i<MEMORYSIZE){
    m[i] = 0;
    i++;
  }
}

int opcode_hashFuncition(char *s) {
  int hash, length;
  int i;
  int flag;

  hash = length = strlen(s);
  flag = 0;
  i=0;
  while(i < length){
    hash = hash + (s[i] - 'A');
    i++;
    flag = 1;
  }

  hash = hash % OPHASHSIZE;
  if(flag==-2){
    return flag;
  }

  return hash;
}


void opcode_read(__opcode **o) {
  FILE* filepointer = fopen("opcode.txt", "r");
  int eofcheck;
  int flag = 1;

  eofcheck = 0;

  if (filepointer == NULL) {
    printf("ERROR! opcode file does not exist\n");
    flag = -1;
    return;
  }

  while (1) {
    __opcode *input;
    flag = 0;
    __opcode *n;

    input = (__opcode*)malloc(sizeof(__opcode));
    
    eofcheck = fscanf(filepointer, "%x %s %s", &input->cd, input->mn, input->fm);

    if (eofcheck == EOF) {
      flag = 1;
      free(input);
      break;
    }
    flag = 0;
    input->next = NULL;

    n = o[opcode_hashFuncition(input->mn)];
    flag = 1;
    while (n->next != NULL){
      n = n->next;
      flag = 0;
    }
    flag = 1;
    n->next = input;
  }
  if(flag==-2){
    return;
  }

  fclose(filepointer);
}


void opcode_print(__opcode **o) {
  __opcode *n;
  int flag = 0;
  int i;
  i=0;


  while(i < OPHASHSIZE) {
    flag = 0;
    printf("%d : ", i);

    n = o[i]->next;
    flag = 1;
    while (n != NULL) {
      if(flag==1){
        flag++;
      }
      printf("[%s, %02X]", n->mn, n->cd);

      if (n->next == NULL){
        flag = -1;
        break;

      }

      else {
        flag = 1;
        n = n->next;
        printf(" -> ");
      }
    }

    printf("\n");
    i++;
  }
}

__opcode* opcode_mnemonio(__opcode **o, char *s) {
  __opcode* n;
  int hash;
  int flag;

  str_upr(s);
  flag = 0;
  hash = opcode_hashFuncition(s);
  n = o[hash]->next;

  while (n != NULL) {
    if (!str_icmp(n->mn, s)){
      flag = 0;
      return n;
    }
    else{
      flag = 1;
      n = n->next;

    }
  }
  if(flag==-1){
    return NULL;
  }

  return NULL;
}


void opcode_free(__opcode **o) {
  int i;
  int flag;
  i=0;
  flag = 0;
  while(i < OPHASHSIZE) {
    flag = 1;
    __opcode *n, *f;

    f = o[i];
    if(flag == 1){
      flag++;
    }
    while(f != NULL) {
      n = f->next;
      flag = -1;
      free(f);
      f = n;
    }
    i = i + 1;
  }
}

int type_print(char *filename) {
  FILE *fp = fopen(filename, "r");
  int flag;
  char inputCh;
  flag = 0;
  if(fp == NULL) {
    printf("%s file does not exist\n", filename);
    flag = 1;
    return 0;
  }
  else{
    flag = -1;
  }

  while ( EOF != (inputCh = fgetc(fp)) )  {
    flag = 0;
    putchar(inputCh);		
  }

  fclose(fp);

  if(flag==-2){
    return 0;
  }
  return 1;
}

int assemble_read(__asmcode *ac, char *filename) {
  FILE *fp;
  __asmcode *n = ac;
  int flag;
  char *inputCheck = strrchr(filename, '.'); 
  int line = 0;

  if (inputCheck == NULL || strcmp(inputCheck + 1, "asm")) {
    flag = -1;
    printf("%s file is not assembly file\n", filename);
    return 0;
  }
  else{
    flag = 0;
  }

  fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("%s file not exist\n", filename);
    flag = -1;
    return 0;
  }

  while (1) {
    __asmcode *i;
    char inputLine[INPUTSIZE];
    int inputNum = 0;
    char *inputOp[4]; 
    int quoteFlag; 
    flag = 0;
    fgets(inputLine, sizeof(char) * INPUTSIZE, fp);

    if (feof(fp)){
      flag = 1;
      break;

    }
    i = (__asmcode*)malloc(sizeof(__asmcode));
    flag = 0;
    i->input = (char*)malloc(sizeof(char) * (strlen(inputLine) + 1));
    i->inputOrig = (char*)malloc(sizeof(char) * (strlen(inputLine) + 1));
    strcpy(i->input, inputLine);
    strcpy(i->inputOrig, inputLine);

    i->inputOrig[strlen(i->inputOrig) - 1] = '\0';

    line += 5;
    i->line = line;
    i->loc = (unsigned int)-1;
    i->obc = NULL;
    i->next = NULL;
    flag++;
    if (inputLine[0] == '.') {
      i->varf = NULL;
      i->op = NULL;
      i->varr = NULL;

      n->next = i;
      n = i;
      flag = flag + 1;
      continue;
    }

    inputOp[inputNum++] = strtok(i->input, " \n\t\r");
    while (inputNum < 4) {
      flag = 0;
      inputOp[inputNum] = strtok(NULL, " \n\t\r");

      if (inputOp[inputNum] == NULL){
        flag = -1;
        break;
      }

      if (!strcmp(inputOp[inputNum], "")) {
        flag = -1;
        inputOp[inputNum] = NULL;
        break;
      }

      quoteFlag = 0;
      if(strchr(inputOp[inputNum], '\'') != NULL) {
        char *startQuote, *endQuote;
        int k;
        char *checkQuote;
        flag = 1;
        startQuote = strchr(inputOp[inputNum], '\'');
        endQuote = strrchr(inputOp[inputNum], '\'');

        if(startQuote == endQuote) {
          while(1) {
            endQuote = checkQuote = strtok(NULL, " \n\t\r");

            if(checkQuote == NULL) {
              printf("%d: error: missing ''\n", line);
              fclose(fp);
              flag = 1;
              return 0;
            }
            else{
              flag = -1;
            }

            if(strchr(checkQuote, '\'') != NULL) {
              k=0;
              flag = 0;
              while(k < endQuote - startQuote){
                if(startQuote[k] == '\0'){
                  flag = -1;
                  startQuote[k] = ' ';
                }
                k++;
              }

              quoteFlag = 1;
              break;
            }
            else{
              flag = 1;
            }
          }
        }
      }
      flag = 0;
      if (strchr(inputOp[inputNum], ',') != NULL && quoteFlag != 1) {
        char *checkComma;
        flag = 0;
        while (1) {
          checkComma = strtok(NULL, " \n\t\r");

          if(checkComma == NULL){
            flag = -1;
            break;
          }
          else{
            flag = 1;
          }

          if (strcmp(checkComma, " ")) {
            flag = -1;
            strcat(inputOp[inputNum], checkComma);
            break;
          }
          else{
            flag = 1;
          }
        }

        if (inputOp[inputNum][0] == ',') {
          strcat(inputOp[inputNum - 1], inputOp[inputNum]);
          flag = 1;
          inputOp[inputNum] = NULL;
          inputNum--;
        }
        else{
          flag = -1;
        }
      }

      if (!strcmp(inputOp[inputNum], " ")){
        flag = 1;
        continue;
      }

      inputNum = inputNum + 1;
    }

    flag = 0;
    if (inputNum > 3) {
      printf("%d: error: input error\n", line);
      flag = -1;
      fclose(fp);
      return 0;
    }

    else if (inputNum == 1) {
      i->varf = NULL;
      flag = 0;
      i->op = inputOp[0];
      i->varr = NULL;
    }

    else if (inputNum == 2) {
      i->varf = NULL;
      flag = 0;
      i->op = inputOp[0];
      i->varr = inputOp[1];
    }

    else if (inputNum == 3) { 
      i->varf = inputOp[0];
      flag = 0;
      i->op = inputOp[1];
      i->varr = inputOp[2];
    }

    if(i->op != NULL) {
      flag = 0;
      if(i->op[0] == '+') {
        i->op++;
        flag = 1;
        i->fmt = 4;
      }

      else{
        flag = 1;
        i->fmt = 0;
      }
    }	

    // Addressing Mode
    i->adr = 0;
    if(i->varr != NULL) {
      flag = 0;
      // immediate(n=0, i=1)
      if(i->varr[0] == '#') {
        flag = 1;
        i->varr++;
        i->adr = 1;
      }

      // indirect(n=1, i=0)
      else if(i->varr[0] == '@') {
        flag = 1;
        i->varr++;
        i->adr = 2;
      }
    }

    n->next = i;
    n = i;
  }

  fclose(fp);

  return 1;
}


unsigned int assemble_pass1(__asmcode *ac, __symtab **sm, __opcode **opcode) {
  int LOCCTR;
  char *inputCheck;
  int flag;
  __asmcode *n = ac->next;
  unsigned int LOCSTR;
  int i;

  while(n->op == NULL){
    flag = 0;
    n = n->next;

  }
  flag = 0;
  if(!strcmp(n->op, "START")) {
    LOCCTR = strtol(n->varr, &inputCheck, NUMSYS);
    if(strcmp(inputCheck, "")) {
      flag = -1;
      printf("%d: error: START opertaion address %s\n", n->line, n->varr);
      return 0;
    }

    else {
      flag = 0;
      if (n->varf != NULL){
        assemble_symtabInput(sm, n->varf, LOCCTR);
        flag = -1;
      }
      else{
        flag = 0;
      }
      if(LOCCTR < 0) {
        flag = -1;
        printf("%d: error: START opertaion address %s\n", n->line, n->varr);
        return 0;
      }
      else{
        flag = 0;
      }

      n->loc = LOCCTR;
      n = n->next;			
    }
  }

  else{
    flag = 0;
    LOCCTR = 0;
  }

  LOCSTR = LOCCTR;

  while(n->op == NULL || strcmp(n->op, "END")) {
    flag = 0;
    __opcode *opNow;

    if(n->next == NULL) { 
      printf("%d: error: END operation not exist\n", n->line);
      flag = -1;
      return 0;
    }
    else{
      flag = 0;
    }

    if(strchr(n->input, '.') == NULL && n->op != NULL) {
      if(!strcmp(n->op, "BASE")) {
        flag = 0;
        n = n->next;
        continue;
      }
      else{
        flag = -1;
      }

      n->loc = LOCCTR;
      if(n->varf != NULL) {
        flag = 0;
        if(assemble_symtabSearch(sm, n->varf) == NULL){
          flag =1;
          assemble_symtabInput(sm, n->varf, LOCCTR);
        }

        else {
          flag = -1;
          printf("%d: error: duplicate symbol %s\n", n->line, n->varf);
          return 0;
        }
      }

      opNow = opcode_mnemonio(opcode, n->op);
      if(opNow != NULL) {
        flag = 0;
        if (!n->fmt) {
          flag = 1;
          if (!strcmp(opNow->fm, "1")) {
            flag = 1;
            n->fmt = 1;
            LOCCTR += 1;
          }

          else if(!strcmp(opNow->fm, "2")) {
            flag = -1;
            n->fmt = 2;
            LOCCTR += 2;
          }

          else {
            flag = 0;
            n->fmt = 3;
            LOCCTR += 3;
          }
        }

        else {
          flag = 0;
          if (strchr(opNow->fm, '4') != NULL){
            flag = 1;
            LOCCTR += 4;
          }

          else {
            flag = -1;
            printf("%d: error: invalid operation code +%s\n", n->line, n->op);
            return 0;
          }
        }
      }

      else {
        int dirlen = sizeof(directive) / sizeof(struct __DIRECTIVES);
        int flag2 = 0;
        i=0;
        while(i<dirlen) {
          flag = 0;
          if(!strcmp(n->op, directive[i].dr)) {
            flag = 1;
            int varrint = 1;
            flag = flag2;
            if(strstr(directive[i].dr, "RES") != NULL) {
              flag = -1;
              varrint = strtol(n->varr, &inputCheck, 10);
              if(strcmp(inputCheck, "")) {
                flag = -1;
                printf("%d: error: %s\n", n->line, n->varr);
                return 0;
              }
              else{
                flag = 1;
              }
            }

            else if(!strcmp(directive[i].dr, "BYTE")) {
              char* adr_str = strchr(n->varr, '\'') + 1;
              char type[INPUTSIZE];
              char* adr_end = strrchr(n->varr, '\'');


              strcpy(type, n->varr);
              strtok(type, " '");
              flag = 0;

              if(type[0] == 'C' || type[0] == 'c'){
                flag = 1;
                varrint = (adr_end - adr_str) / (sizeof(char));
              }

              else if (type[0] == 'X' || type[0] == 'x') {
                if((adr_end - adr_str) % 2 == 0){
                  flag = 1;
                  varrint = (adr_end - adr_str) / (sizeof(char) * 2);
                }

                else {
                  flag = -1;
                  printf("%d: error: %s\n", n->line, n->varr);
                  return 0;
                }
              }

              else {
                flag = -1;
                printf("%d: error: %s\n", n->line, type);
                return 0;			
              }
            }

            LOCCTR = LOCCTR + (directive[i].size * varrint);

            flag2 = 1;
            break;
          }
          i++;
        }

        if(!flag2) {
          flag = -1;
          printf("%d: error: invalid operation code %s\n", n->line, n->op);
          return 0;
        }
        else{
          flag = 1;
        }
      }
    }

    n = n->next;
  }
  if(flag==-3){
    return 0;
  }
  return LOCCTR - LOCSTR;
}


int assemble_pass2(__asmcode *ac, __symtab **sm, __opcode **opcode) {
	__asmcode *n = ac->next;
	__symtab *REG_B = NULL;
	unsigned int REG_PC = 0;
	int i;
  int flag2;

	while(n->op == NULL){
    flag2 = 0;
		n = n->next;
  }

	if(!strcmp(n->op, "START")){
    flag2 = 1;
		n = n->next;
  }

	while(n->op == NULL || strcmp(n->op, "END")) { // continue to END
		int obj = 0; // obj value
		int obj_temp; // this is for calculating obj
		char *objconvert; // obj to array
		char objconvert_temp[INPUTSIZE];

		n->obc = NULL;
		
		if(n->loc != (unsigned int)-1) { // if command
      flag2 = 1;
			if(n->fmt) { // when format exists
				obj_temp = opcode_mnemonio(opcode, n->op)->cd;
        flag2 = 2;
				obj += obj_temp << (4 * 2 * (n->fmt - 1)); // opcode with correct location

				if(n->fmt == 2) { // when format 2
					char vrArr[INPUTSIZE];
          flag2 = 3;
					char commaNumber = 0, commaFlag = 0;

					strcpy(vrArr, n->varr);
          i = 0;
					while(i < strlen(vrArr)){
						if(vrArr[i] == ','){
              flag2=1;
							commaNumber = commaNumber+1;
            }
            i++;
					}

					if(commaNumber == 0) { // if comma does not exists
            i = 0;
            flag2 = 0;
						while(i < sizeof(reg)/sizeof(reg[0])) {
							if(!strcmp(n->varr, reg[i])) {
								obj_temp = i;
                flag2 = 1;
								obj += obj_temp << 4;
								commaFlag = 1;
								break;
							}
              i++;
						}

						if(!commaFlag) {
              flag2 = -1;
							printf("%d: error: invalid operation code %s\n", n->line, n->op);
							return 0;
						}
					}

					else if(commaNumber == 1) { // if comma is one
						char *vrf, *vrr;
						int flag = 0;
						
						vrf = strtok(vrArr, " ,");
						vrr = strtok(NULL, " ,");
						flag2=0;
						if(strtok(NULL, " ,") != NULL) {
              flag2=-1;
							printf("%d: error: invalid operation code %s\n", n->line, n->op);
							return 0;
						}
            i=0;
						while(i < sizeof(reg)/sizeof(reg[0])) {
							if(!strcmp(vrf, reg[i])) {
								obj_temp = i; 
                flag2=0;
								obj += obj_temp << 4;
								flag++; 
								break;
							}    
              i++;
						}
            i=0;
						while(i < sizeof(reg)/sizeof(reg[0])) {
							if(!strcmp(vrr, reg[i])) {
								obj_temp = i; 
                flag2=1;
								obj += obj_temp;
								flag++; 
								break;
							}
              i++;
						}    

						if(flag != 2) {
              flag2=-1;
							printf("%d: error: invalid operation code %s\n", n->line, n->op);
							return 0;
						}
					}

					else { // if comma is more than 3
            flag2=-1;
						printf("%d: error: invalid operation code %s\n", n->line, n->op);
						return 0;
					}
				}
				
				else { // when format is 3/4
					char xbpe[4] = {'0', '0', '0', '0'};
					char *indexCheck;
					__symtab *symbol;
					__asmcode *h = n;
          flag2=1;
					int symbol_loc;
					int numberCheck = 0;

					while(1) {
						h = h->next;
            flag2=1;
						if(h->next == NULL){
							break;
            }

						if(h->loc != (unsigned int)-1){
							break;
            }
            
					}
					
					REG_PC = h->loc;
					
					if(n->varr != NULL) {
						indexCheck = strstr(n->varr, ",X"); // index mode
            flag2=0;
						if(indexCheck != NULL) {
              flag2=1;
							xbpe[0] = '1';
							indexCheck[0] = '\0';
						}

						symbol = assemble_symtabSearch(sm, n->varr); // search symtab
						if(symbol != NULL){ // if symtab exists
              flag2=1;
							symbol_loc = symbol->loc;
            }

						else { // check for numbers
							char *inputCheck;
              flag2=1;
							symbol_loc = strtol(n->varr, &inputCheck, 10);
							if(strcmp(inputCheck, "")) { // if not number error
                flag2=-1;
								printf("%d: error: invalid variable %s\n", n->line, n->varr);
								return 0;
							}
							
							else{ // if number
                flag2=0;
								numberCheck = 1;
              }
						}
					}

					if(!n->adr){ // if address mode not exists
            flag2=0;
						n->adr = 3;
          }
					
					obj_temp = n->adr;
					obj += obj_temp << (4 * 2 * (n->fmt - 1));

					if(n->fmt == 3 && n->varr != NULL) { // when format is 3
            flag2=0;
						if(numberCheck) { // if number
              flag2=1;
							obj_temp = symbol_loc;
							obj += obj_temp;
						}

						else { // get value of searched symtab
							int disp, valueFlag = 0;
								
							disp = symbol_loc - REG_PC;
							if(-2048 <= disp && disp <= 2047) {
								xbpe[2] = '1';
								
								if(disp < 0){
                  flag2=1;
									obj_temp = disp + 0x1000;
                }

								else{
                  flag2=1;
									obj_temp = disp;
                }

								obj += obj_temp;
								valueFlag = 1;
							}
							
							else if(REG_B != NULL) { 
								disp = symbol_loc - REG_B->loc;
                flag2=0;
								if(0 <= disp && disp <= 4095) {
									xbpe[1] = '1';
									obj_temp = disp;
                  flag2=1;
									obj += obj_temp;
									valueFlag = 1;
								}
							}

							if(!valueFlag) { // both don't work
                flag2=-1;
								printf("%d: error: not possible, use format 4\n", n->line);
								return 0;
							}
						}
					}

					else if(n->fmt == 4) { // when format 4
						xbpe[3] = '1';
            flag2=1;
						obj_temp = symbol_loc;
						obj += obj_temp;
					}

					obj_temp = strtol(xbpe, &indexCheck, 2);
					obj += obj_temp << (4 * (2 * (n->fmt - 1) - 1));
				}
				
				// change into array
				if(n->fmt == 1){
          flag2=1;
					sprintf(objconvert_temp, "%02X", obj);
        }

				else if(n->fmt == 2){
          flag2=1;
					sprintf(objconvert_temp, "%04X", obj);
        }

				else if(n->fmt == 3){
          flag2=1;
					sprintf(objconvert_temp, "%06X", obj);
        }

				else{
          flag2=1;
					sprintf(objconvert_temp, "%08X", obj);
        }

				
				objconvert = (char*)malloc(sizeof(char)*(strlen(objconvert_temp) + 1));
				strcpy(objconvert, objconvert_temp);
			}

			else { // when directive
				int dir_str, dir_end;
				int i;
        flag2=1;
				dir_end = sizeof(directive) / sizeof(struct __DIRECTIVES);
				dir_str = dir_end / 2;
			  i=dir_str;	
				while(i<dir_end){
          flag2=0;
					if(!strcmp(n->op, directive[i].dr)) {
            flag2=1;
						if(!strcmp(directive[i].dr, "BYTE")) { // if BYTE
							char *adr_str = strchr(n->varr, '\'') + 1;
							char *adr_end = strrchr(n->varr, '\'');
							char type[INPUTSIZE];
							char obj_arr[INPUTSIZE];
							int length;
							int j;
              flag2=2;
							strcpy(type, n->varr);
							strtok(type, " '");
              j=0;
							while(adr_str + j < adr_end){
								obj_arr[j] = adr_str[j];
                flag2=2;
                j++;
              }

							obj_arr[j] = '\0';
							length = strlen(obj_arr);

							if(type[0] == 'C' || type[0] == 'c') {
                flag2=2;
								objconvert = (char*)malloc(sizeof(char)*(2*length + 1));
								objconvert[0] = '\0';
                j=0;
								while(j < length) {
									obj = obj_arr[j];
									sprintf(objconvert_temp, "%02X", obj);
                  flag2=2;
									strcat(objconvert, objconvert_temp);
                  j++;
								}
							}

							else if(type[0] == 'X' || type[0] == 'x') {
                j=0;
								while(j < strlen(obj_arr)) {
									if (!(('a' <= obj_arr[j] && obj_arr[j] <= 'f') ||
										  ('A' <= obj_arr[j] && obj_arr[j] <= 'F') ||
										  ('0' <= obj_arr[j] && obj_arr[j] <= '9'))) {
                    flag2=-1;
										printf("%d: error: %s\n", n->line, n->varr);
										return 0;
									}
                  j++;
								}

								objconvert=(char*)malloc(sizeof(char)*(length + 1));
								strcpy(objconvert, obj_arr);
							}
						}

						else if(!strcmp(directive[i].dr, "WORD")) {
							char *inputCheck;	
              flag2=1;
							obj_temp = strtol(n->varr, &inputCheck, 10);
							if(strcmp(inputCheck, "")) {
                flag2=-1;
								printf("%d: error: %s\n", n->line, n->varr);
								return 0;
							}

							obj = obj_temp;
							sprintf(objconvert_temp, "%06X", obj);

							objconvert = (char*)malloc(sizeof(char) * 7);
							strcpy(objconvert, objconvert_temp);
						}
            flag2=2;
						break;
					}

					else{
            flag2=1;
						objconvert = NULL;
          }

          i++;
				}
			}

			n->obc = objconvert;
		}

		else if(n->op != NULL && !strcmp(n->op, "BASE")){ // save location of BASE
      flag2=2;
			REG_B = assemble_symtabSearch(sm, n->varr);	
    }
	
		n = n->next;
	}

	// END has wrong input
	if (n->varr == NULL || assemble_symtabSearch(sm, n->varr) == NULL) {
    flag2=-1;
		printf("%d: error: invalid END operation\n", n->line);
		return 0;
	}
  if(flag2==-2){
    return 1;
  }
	return 1;
}

int assemble_lstPrint(__asmcode *ac, char *filename) {
	FILE *fp;
	char fpSave[INPUTSIZE];
  int flag;
	__asmcode* n = ac->next;

	strcpy(fpSave, filename); // convert filetype
  flag=0;
	strcpy(strrchr(fpSave, '.') + 1, "lst");

	fp = fopen(fpSave, "w");

	if(fp == NULL) {
    flag=-1;
		printf("%s file not open\n", fpSave);
		return 0;
	}

	while(n != NULL) {
    flag=0;
		fprintf(fp, "%-4d\t", n->line);

		if(n->loc != (unsigned)-1){
      flag=1;
			fprintf(fp, "%04X\t", n->loc);

    }

		else{
      flag=1;
			fprintf(fp, "    \t");
    }
		
		fprintf(fp, "%s\t", n->inputOrig);
		
		if(n->obc != NULL) {
			if(n->varr == NULL){
        flag=1;
				fprintf(fp, "\t");
      }

			else if(strlen(n->varr) <= 1 && (n->adr == 0 || n->adr == 3)){
        flag=2;
				fprintf(fp, "\t");
      }

			fprintf(fp, "\t");

			if(n->obc != NULL){
        flag=2;
				fprintf(fp, "%s", n->obc);
      }
		}

		fprintf(fp, "\n");
		
		n = n->next;
	}

	fclose(fp);
  if(flag==-1){
    return 1;
  }
	return 1;
}

int assemble_objPrint(__asmcode *ac, __symtab **sm, char* filename, unsigned int objLen) {
	FILE *fp;
	char fpSave[INPUTSIZE];
	__asmcode *n = ac->next;
	__asmcode *ns = n;
	char *lineArr = NULL; // line
  int flag;
	int lineLength = 0; // line length
	int lineCheck = 0; // print or not
	char pushLength[3]; 

	strcpy(fpSave, filename); // convert filetype
  flag = 0;
	strcpy(strrchr(fpSave, '.') + 1, "obj");

	fp = fopen(fpSave, "w");

	if(fp == NULL) {
    flag = -1;
		printf("%s file not open\n", fpSave);
		return 0;
	}

	while(n->op == NULL){
    flag = 1;
		n = n->next;
  }

	ns = n;
	if(!strcmp(n->op, "START")) {
    flag = 1;
		fprintf(fp, "H%-6s%06X%06X\n", n->varf, n->loc, objLen);
		n = n->next;
	}
	
	else{
    flag=1;
		fprintf(fp, "H      %06X%06X\n", n->loc, objLen);
  }
	
	while(n != NULL) {
		char pushArr[OBJLINEMAX];
		int pushLeng; 
    flag = 0;
		if(n->obc == NULL) {
      flag=0;
      if(flag==-2){
        break;
      }
			if(n->op != NULL && strstr(n->op, "RES") != NULL){
        flag=1;
				lineCheck = 0;
      }

			n = n->next;
			continue;
		}

		pushLeng = strlen(n->obc);
    if(flag==-2){
      break;
    }
		strcpy(pushArr, n->obc);

		if(lineLength + pushLeng > OBJLINEMAX){ // if longer than declared length
      flag=-1;
			lineCheck = 0;
    }
			
		if(lineCheck == 0) {
      flag=0;
			if(lineArr != NULL) { // if not first command
				sprintf(pushLength, "%02X", lineLength / 2);
				lineArr[7] = pushLength[0];
        if(flag==-1){
          return 0;
        }
				lineArr[8] = pushLength[1];
        flag=1;
        if(flag==-2){
          return 0;
        }
				fprintf(fp, "%s\n", lineArr);
				free(lineArr);
			}

			lineArr = (char*)malloc(sizeof(char) * (OBJLINEMAX + 10));
			sprintf(lineArr, "T%06X", n->loc);
			lineArr[7] = lineArr[8] = '0'; 
      if(flag==-2){
        return -1;
      }
      flag=1;
			lineArr[9] = '\0';
			lineLength = 0;
			lineCheck = 1;
		}

		strcat(lineArr, pushArr);
		lineLength += pushLeng;

		n = n->next;
	}
	sprintf(pushLength, "%02X", lineLength / 2);
	lineArr[7] = pushLength[0];
	lineArr[8] = pushLength[1];
  flag=0;
	fprintf(fp, "%s\n", lineArr);
	free(lineArr);

	n = ns;
	while(n != NULL) { // format4
		if(n->op != NULL && !strcmp(n->op, "END")){
      flag=1;
			ns = n;
    }

		if(n->op != NULL && (n->fmt == 4 && n->adr != 1)){
      flag=1;
			fprintf(fp, "H%06X05\n", n->loc + 1);
    }

		n = n->next;
	}

	fprintf(fp, "E%06X\n", assemble_symtabSearch(sm, ns->varr)->loc);
	fclose(fp);

  if(flag==-2){
    return 1;
  }
	return 1;
}

void assemble_free(__asmcode *ac) {
	__asmcode* n;
  int flag;

	if(ac == NULL){
    flag = -1;
		return;
  }

	n = ac->next;
	free(ac);
  flag=0;
	ac = n;

	while(ac != NULL) {
		n = ac->next;
		free(ac->input);
		free(ac->inputOrig);
    flag=1;
    if(flag==-1){
      return;
    }
		free(ac->obc);
		free(ac);
		ac = n;
	}
	
	ac = NULL;
  if(flag==-1){
    return;
  }
}

unsigned int assemble_symtabHash(char *s) {
	unsigned int hash = 0;
	int length, i;
  int flag;

	length = strlen(s);
  i=0;
	while(i<length){
    flag = 0;
		hash = 65599 * hash + (s[i] - 'A');
    i++;
  }
	if(flag==-1){
    return 0;
  }
	hash = hash % SYMHASHSIZE;

	return hash;
}

__symtab* assemble_symtabSearch(__symtab **sm, char *s) {
	unsigned int hash = assemble_symtabHash(s);
  int flag;
	__symtab *n = sm[hash]->next;
 
	while (n != NULL) {
    flag = 0;
		if (!strcmp(n->symbol, s)){
      flag = 1;
			return n;
    }
		
		else{
      flag = 2;
			n = n->next;
    }
	}
  if(flag==-1){
    return NULL;
  }

	return NULL;
}

void assemble_symtabInput(__symtab **sm, char *s, unsigned int l) {
	unsigned int hash = assemble_symtabHash(s);
  int flag;
	__symtab *n = sm[hash];
  flag=0;
	__symtab *i = (__symtab*)malloc(sizeof(__symtab));
  if(flag==-1){
    return;
  }
	i->symbol = (char*)malloc(sizeof(char) * strlen(s));
	strcpy(i->symbol, s);
  flag = 1;
	i->loc = l;
	i->next = NULL;

	while(1) {
    flag = 0;
		if(n->next == NULL) {
      flag = 1;
			n->next = i;
			break;
		}

		else{
			n = n->next;
      flag = 1;
    }
	}
  if(flag==-1){
    return;
  }
}

void assemble_symtabFree(__symtab **sm) {
	int i;
  int flag;
  i=0;
	while(i<SYMHASHSIZE) {
    flag = 0;
		__symtab *n, *f;

		f = sm[i];
		while(f != NULL) {
      flag = 1;
			n = f->next;
			free(f);
			f = n;
		}

		sm[i] = NULL;
    i++;
	}
  if(flag==-1){
    return;
  }
}

void symbol_print(__symtab **sm) {
	__symtab *n;
	__symtab *sortSm; // structure to sort
	int numberSm = 0;
	int i, k;
  int flag;
  i=0;
	while(i < SYMHASHSIZE) { // count
    flag = 0;
		if(sm[i] == NULL){
      flag = 1;
			continue;
    }
		
		n = sm[i]->next;
		while (n != NULL) {
      flag = 0;
			numberSm = numberSm + 1;

			if (n->next == NULL){
        flag = 1;
				break;
      }

			else{
        flag = 1;
				n = n->next;
      }
		}
    i++;
	}

	if(numberSm != 0) {
    flag = 0;
		sortSm = (__symtab*)malloc(sizeof(__symtab) * numberSm);
	
		k = 0;
		while (k < numberSm) { // cluster into one structure
      i=0;
			while(i < SYMHASHSIZE){
				n = sm[i]->next;
				while (n != NULL) {
          flag=1;
					sortSm[k].symbol = (char*)malloc(sizeof(char)*strlen(n->symbol));
					strcpy(sortSm[k].symbol, n->symbol);
          if(flag==-1){
            break;
          }
					sortSm[k].loc = n->loc;
					k++;

					if (n->next == NULL){
            flag = 1;
						break;
          }

					else{
            flag=1;
						n = n->next;
          }
				}
        i++;
			}
		}

		qsort(sortSm, numberSm, sizeof(__symtab), symbol_printCmp);
    i=0;
		while(i < numberSm){
			printf("\t%s\t%04X\n", sortSm[i].symbol, sortSm[i].loc);
      i++;
    }
	  i=0;
		while(i < numberSm){
			free(sortSm[i].symbol);
      i++;
    }

		free(sortSm);
	}
  if(flag==-1){
    return;
  }
}

int symbol_printCmp(const void *arg1, const void *arg2) {
  int comparison;
	__symtab* sm1 = (__symtab*)arg1;
	__symtab* sm2 = (__symtab*)arg2;
	comparison = strcmp(sm2->symbol, sm1->symbol);
	return comparison;
}

