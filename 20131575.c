#include "20131575.h"

unsigned char memory[MEMORYSIZE];

int main(void) {

  //declare variables and initialize variables
  long dumpAddress;
  int symtabFlag;
  int i, j;
  int flag;
  __history *history, *historyNow;
  __opcode *opcode[OPHASHSIZE];
  __symtab *symtab[SYMHASHSIZE];
  __asmcode *asmcode = NULL;
  history = (__history*)malloc(sizeof(__history));
  historyNow = history;
  history->next = NULL;
  dumpAddress = -1;
  symtabFlag = 0;
  i=0;
  while(i < OPHASHSIZE) {
    opcode[i] = (__opcode*)malloc(sizeof(__opcode));
    opcode[i]->next = NULL;
    i++;
    flag = 0;
  }
  i=0;
  while(i < SYMHASHSIZE){
    symtab[i] = NULL;
    i++;
    flag = 0;
  }


  // read opcode and set memory
  memory_reset(memory);
  opcode_read(opcode);

  while (1) {
    char* inputCopy = (char*)malloc(sizeof(char) * INPUTSIZE);
    char* input = (char*)malloc(sizeof(char) * INPUTSIZE);
    char* commandOption[4];	//input option
    char* command;  //input
    /*
     * -1 : wrong input
     *  0 : command option does not exist
     *  1 ~ : command has some options
     */
    int commandOptionFlag = 0;
    //count comma
    int commaNumber = 0;



    /*
     * -1 : do not save in history
     *  0 : exit simulator
     *  1 : save in history
     */
    int commandFlag = 1;
    flag = 0;

    printf("sicsim> ");
    fgets(input, sizeof(char) * INPUTSIZE, stdin);
    strcpy(inputCopy, input);

    inputCopy = strtok(inputCopy, "\n");

    // split command and command options
    command = strtok(input, " \n\t\r");		
    for(i=0;i < 4;i++) {
      char *change;
      commandOption[i] = strtok(NULL, " ,\n\t\r");

      if (commandOption[i] == NULL){
        flag = -1;
        break;

      }
      if (!strcmp(commandOption[i], "")) {
        commandOption[i] = NULL;
        flag = -1;
        break;
      }

      if (!strcmp(commandOption[i], ",") || !strcmp(commandOption[i], " ")){
        flag = -1;
        continue;
      }

      change = strchr(commandOption[i], ',');
      if (change != NULL){
        flag = 0;
        change[0] = '\0';
      }

    }
    flag = i;
    commandOptionFlag = flag;

    // 명령어 옵션 배열 초기화
    i=commandOptionFlag;
    while(i < 4){
      commandOption[i] = NULL;
      i++;
      flag++;
    }

    // 전체 ',' 값의 개수를 확인
    if (inputCopy != NULL) {
      i=0;
      while(i < strlen(inputCopy)) {
        if (inputCopy[i] == ',')
          commaNumber = commaNumber + 1;
        i++;
        flag++;
      }
    }

    // 입력된 ','값의 위치가 적절한지 확인 
    flag = commandOptionFlag;
    if (flag > 0) {
      int startPos, endPos;
      int commaPos = strlen(command);
      int commaFlag = 1;
      i = 0;
      while(i < commandOptionFlag - 1) {
        startPos = commandOption[i] - input;
        endPos = commandOption[i + 1] - input;
        j = commaPos;
        flag = 0;
        while(j < strlen(inputCopy)) {
          if (inputCopy[j] == ',' && startPos <= j && j <= endPos) {
            flag = 0;
            commaFlag = flag;
            break;
          }
          j++;
        }

        if (commaFlag) {
          flag = -1;
          commandOptionFlag = flag;
          break;
        }

        else{
          flag = 0;
          commaPos = j;
        }
        i++;
      }
    }

    // 적절한 명령어 옵션이 아닐 시 or 명령어와 ',' 수의 짝이 맞지 않을 때
    if (commandOptionFlag - 1 != commaNumber &&commandOptionFlag != 0){
      flag = -1;
      commandOptionFlag = flag;
    }

    /***
      명령 처리 시작
     ***/

    // 입력값이 없을 시
    if (command == NULL){
      flag = -1;
      commandFlag = flag;
    }

    // 명령에 맞게 함수 실행
    else if (!strcmp(command, "h") || !strcmp(command, "help")) {
      if (commandOptionFlag){
        flag = -1;
        commandOptionFlag = flag;
      }

      else{
        help_print();
      }
    }

    else if (!strcmp(command, "d") || !strcmp(command, "dir")) {
      if (commandOptionFlag){
        flag = -1;
        commandOptionFlag = flag;
      }

      else{
        dir_print(".");
      }
    }

    else if (!strcmp(command, "q") || !strcmp(command, "quit")) {
      if (commandOptionFlag){
        flag = -1;
        commandOptionFlag = flag;
      }

      else{
        flag = 0;
        commandFlag = flag;
      }
    }

    else if (!strcmp(command, "hi") || !strcmp(command, "history")) {
      if (commandOptionFlag){
        flag = -1;
        commandOptionFlag = flag;
      }

      else {
        history_input(&historyNow, input);
        history_print(history);
        flag = -1;
        commandFlag = flag;
      }
    }

    else if (!strcmp(command, "du") || !strcmp(command, "dump")) {
      flag = 0;

      char *inputCheck;
      long adr_str = 0, adr_end = 0;	

      if(commandOptionFlag < 0 || commandOptionFlag > 2){
        flag = -1;
        commandOptionFlag = flag;

      }

      else {
        flag = 0;
        int adr_end_flag = 0;

        if (commandOptionFlag == 0) {
          adr_str = (dumpAddress + 1) % (MEMORYSIZE);
          flag = 1;
          adr_end_flag = flag;
        }

        else {
          adr_str = strtol(commandOption[0], &inputCheck, NUMSYS);
          if(strcmp(inputCheck, "")){
            flag = -1;
            commandOptionFlag = flag;

          }

          if (commandOptionFlag == 1){
            flag = 1;
            adr_end_flag = flag;
          }

          else if (commandOptionFlag == 2) {
            flag = 0;
            adr_end = strtol(commandOption[1], &inputCheck, NUMSYS);

            if(strcmp(inputCheck, "")){
              flag = -1;
              commandOptionFlag = flag;
            }
          }
        }

        if (commandOptionFlag != -1) {
          flag = 0;
          if (adr_end_flag) {
            adr_end = adr_str + 10 * MEMORYLINE - 1;
            flag = adr_end_flag;
            if (adr_end >= MEMORYSIZE){
              adr_end = MEMORYSIZE - 1;
              flag = adr_end;
            }
          }
          
          if (adr_str < 0 || adr_str > adr_end ||
              adr_str >= MEMORYSIZE || adr_end >= MEMORYSIZE) {
            printf("dump: exception error\n");
            flag = -1;
            commandFlag = flag; 
          }
        }
      }

      if(commandOptionFlag != -1 && commandFlag != -1) {
        flag = 0;
        memory_dump(memory, adr_str, adr_end);
        dumpAddress = adr_end;
      }
    }

    else if (!strcmp(command, "e") || !strcmp(command, "edit")) {

      char *inputCheck;
      long address;
      long value_temp;
      unsigned char value;

      // command option check
      if(commandOptionFlag != 2){
        flag = -1;
        commandOptionFlag = flag; 

      }

      // command option process
      else {
        address = strtol(commandOption[0], &inputCheck, NUMSYS);
        flag = 0;
        if(strcmp(inputCheck, "")){
          flag = -1;
          commandOptionFlag = flag;
        }

        value_temp = strtol(commandOption[1], &inputCheck, NUMSYS);
        flag = 0;
        if(strcmp(inputCheck, "")){
          flag = 1;
          commandOptionFlag = 1;
        }

        // address command option check
        if(address < 0 || address >= MEMORYSIZE) {
          printf("edit: exception error\n");
          flag = -1;
          commandFlag = flag;
        }

        // vlaue command option check
        if(value_temp < 0 || value_temp >= ASCIISET) {
          printf("edit: vlaue is invalid\n");
          flag = -1;
          commandFlag = flag;
        }

        else{
          flag = 0;
          value = (unsigned char)value_temp;
        }
      }

      if(commandOptionFlag != -1 && commandFlag != -1) {
        flag = 0;
        memory_edit(memory, address, value);
      }
    }

    else if (!strcmp(command, "f") || !strcmp(command, "fill")) {
      long adr_str, adr_end;
      char *inputCheck;
      long value_temp;
      unsigned char value;
      // command option check
      if(commandOptionFlag != 3){
        flag = -1;
        commandOptionFlag = flag;
      }

      // command option process
      else {
        adr_str = strtol(commandOption[0], &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          flag = -1;
          commandOptionFlag = flag;
          break;
        }

        adr_end = strtol(commandOption[1], &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          flag = -1;
          commandOptionFlag = flag;
          break;
        }

        value_temp = strtol(commandOption[2], &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          flag = -1;
          commandOptionFlag = flag;
          break;
        }

        // value command opiton check
        if(value_temp < 0 || value_temp >= ASCIISET) {
          printf("fill: value is invalid\n");
          flag = -1;
          commandFlag = flag;
        }

        else{
          value = (unsigned char)value_temp;
        }
        if(adr_str < 0 || adr_str > adr_end ||
            adr_str >= MEMORYSIZE || adr_end >= MEMORYSIZE) {
          printf("fill: exception error\n");
          flag = -1;
          commandFlag = flag;
        }
      }

      if(commandOptionFlag != -1 && commandFlag != -1){
        memory_fill(memory, adr_str, adr_end, value);
      }
    }

    else if (!strcmp(command, "reset")) {
      if (commandOptionFlag){
        flag = -1;
        commandOptionFlag = flag;

      }

      else{
        memory_reset(memory);
      }
    }

    else if (!strcmp(command, "opcode")) {
      if (commandOptionFlag != 1){
        flag = -1;
        commandOptionFlag = flag;
      }

      else {
        __opcode *opcodeResult = opcode_mnemonio(opcode, commandOption[0]);

        if (opcodeResult == NULL) {
          printf("opcode is not exist\n");
          flag = -1;
          commandFlag = flag;
        }

        else{
          printf("opcode is %02X\n", opcodeResult->cd);
        }
      }
    }

    else if (!strcmp(command, "opcodelist")) {
      if (commandOptionFlag){
        flag = -1;
        commandOptionFlag = flag;
      }

      else{
        opcode_print(opcode);
      }
    }

    // proj2
    else if (!strcmp(command, "type")) {
      if (commandOptionFlag != 1){
        flag = -1;
        commandOptionFlag = flag; 

      }

      else {
        if (!type_print(commandOption[0])){
          flag = -1;
          commandFlag = flag; 
        }
      }
    }

    else if (!strcmp(command, "assemble")) {
      if (commandOptionFlag != 1){
        flag = -1;
        commandOptionFlag = flag;
      }

      else {
        unsigned int lengthObj;

        assemble_free(asmcode);
        flag = 0;
        asmcode = (__asmcode*)malloc(sizeof(__asmcode));
        asmcode->next = NULL;

        if(!assemble_read(asmcode, commandOption[0])){
          flag = -1;
          commandFlag = flag;

        }

        else{
          if (symtabFlag != 0){
            flag = 0;
            assemble_symtabFree(symtab);

          }
          i=0;
          while(i< SYMHASHSIZE) {
            symtab[i] = (__symtab*)malloc(sizeof(__symtab));
            symtab[i]->next = NULL;
            i++;
            flag = 0;
          }

          lengthObj = assemble_pass1(asmcode, symtab, opcode);
          if(!lengthObj){
            flag = -1;
            commandFlag = flag;

          }

          else {
            if(!assemble_pass2(asmcode, symtab, opcode)){
              flag = -1;
              commandFlag = flag;
            }

            else{
              flag = 1;
              symtabFlag = flag;
            }
          }
        }

        if(commandFlag == -1) {
          assemble_free(asmcode);
          asmcode = NULL;
          assemble_symtabFree(symtab);
          flag = 0;
          symtabFlag = flag;
        }
        else {
          if(assemble_lstPrint(asmcode, commandOption[0]) &&
              assemble_objPrint(asmcode, symtab, commandOption[0], lengthObj)) {
            char savedfilename[INPUTSIZE];
            strcpy(savedfilename, commandOption[0]);
            strcpy(strrchr(savedfilename, '.'), "\0");
            flag = 0;
            printf("\toutput file : [%s.lst], [%s.obj]\n", savedfilename, savedfilename);
          }

          else{
            flag = -1;
            commandFlag = flag;
          }
        }
      }
    }

    else if (!strcmp(command, "symbol")) {
      if (commandOptionFlag != 0){
        flag = -1;
        commandOptionFlag = flag;
      }

      else {
        if(symtabFlag != 0){
          flag = 0;
          symbol_print(symtab);
        }

        else {
          printf("symbol table is not exist\n");
          flag = -1;
          commandFlag = flag;
        }
      }
    }

    else {
      printf("%s: command not found\n", command);
      flag = -2;
      commandFlag = flag;
    }

    // flag에 따른 결과 처리
    if (commandFlag != -2 && commandOptionFlag == -1){
      printf("%s: invalid option\n", command);
      flag = -1;
    }

    else if (commandFlag == 1){
      flag = 0;
      history_input(&historyNow, inputCopy);

    }
    flag = 0;
    free(input);
    free(inputCopy);

    if (commandFlag == 0){
      flag = -1;
      break;
    }
  }
  flag = 0;
  history_free(history);
  opcode_free(opcode);
  return 0;
}
