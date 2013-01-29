#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 255

#define DEBUG 1
#define DEBUG_PRINT(...) \
   do { if (DEBUG) fprintf(stderr,##__VA_ARGS__); } while(0)

enum
{
   DONE,OK,EMPTY_LINE
};

int isOpcode(char * s)
{
  const char *opcodes[] = {"add","and","br","halt","jmp","jsr","jsrr","ldb","ldw","lea","nop","not","ret","lshf","rshfl","rshfa","rti","stb","stw","trap","xor"};
  int k;

  for(k = 0; k < 21;k++)
  {
     if(strcmp(s,opcodes[k]) == 0)
     {
        return 1;
     }
   }
  if(strlen(s) >=2 && strncmp(s,"br",2) == 0)
  {
     return 0;
  }
  return -1;
}

int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1,
       char ** pArg2, char ** pArg3, char ** pArg4)
{
   char * lRet, * lPtr;
   int i;
   if(!fgets(pLine,MAX_LINE_LENGTH,pInfile))
       return(DONE);
   for(i=0;i< strlen(pLine); i++)
       pLine[i] = tolower(pLine[i]);

   /*convert entire line to lowercase/ What does this line do?*/
   *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
   
   lPtr = pLine;

   while(*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n')
      lPtr++;

   *lPtr = '\0';
   if(!(lPtr = strtok(pLine,"\t\n ,")))
      return(EMPTY_LINE);

   if(isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
   {
      *pLabel = lPtr;
      if(!(lPtr = strtok(NULL,"\t\n ," ))) return(OK);
   }

   *pOpcode = lPtr;

   if(!(lPtr = strtok(NULL,"\t\n ,"))) return(OK);

   *pArg1 = lPtr;

   if(!(lPtr = strtok(NULL,"\t\n ,"))) return(OK);

   *pArg2 = lPtr;
   
   if(!(lPtr = strtok(NULL,"\t\n ,"))) return(OK);

   *pArg3 = lPtr;

   if(!(lPtr = strtok(NULL,"\t\n ,"))) return(OK);

   *pArg4 = lPtr;

   return (OK);
}

int toNum(char * pStr)
{
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if(*pStr == '#')
   {
      pStr++;
      if(*pStr == '-')
      {
         lNeg = 1;
         pStr++;
      }
      t_ptr = pStr;
      t_length = strlen(t_ptr);
      for(k = 0; k < t_length; k++)
      {
         if(!isdigit(*t_ptr))
         {
            printf("Error: invalid decimal operand, %s\n",orig_pStr);
            exit(4);
         }
         t_ptr++;
      }
      lNum = atoi(pStr);
      if(lNeg)
         lNum = -lNum;
      return lNum;
   }
   else if( *pStr == 'x')
   {
      pStr++;
      if(*pStr == '-')
      {
         lNeg = 1;
         pStr++;
      }
      t_ptr = pStr;
      t_length = strlen(t_ptr);
      for(k = 0; k < t_length; k++)
      {
         if(!isxdigit(*t_ptr))
         {
            printf("Error: invalid hex operand, %s\n",orig_pStr);
            exit(4);
         }
         t_ptr++;
      }
      lNumLong = strtol(pStr,NULL,16);
      lNum = (lNumLong > INT_MAX)?INT_MAX:lNumLong;
      if(lNeg)
         lNum = -lNum;
      return lNum;
   }
   else
   {
      printf("Error: invlaid operand,%s\n",orig_pStr);
      exit(4);
   }
}

typedef struct
{
   int address;
   char *label;
   char *opcode;
   char *arg1;
   char *arg2;
   char *arg3;
   char *arg4;
   
}opCodeLine;

void debug_printOpCodeLines(opCodeLine s[],int len)
{
   int k;
   for(k = 0; k < len; k++)
   {
      if(s[k].address == 0)
      {
         continue;
      }
      if(k%20 == 0)
      {
         printf("address\tlabel\topcode\targ1\targ2\targ3\targ4\n");
      }
      printf("0x%x\t%s\t%s\t%s\t%s\t%s\t%s\n",s[k].address,s[k].label,s[k].opcode,s[k].arg1,s[k].arg2,s[k].arg3,s[k].arg4);
   }
}
void init_opCodeLine(opCodeLine s[], int len)
{
   int k;
   for(k = 0; k < len; k++)
   {
      s[k].address = 0;
      s[k].label = NULL;
      s[k].opcode = NULL;
      s[k].arg1 = NULL;
      s[k].arg2 = NULL;
      s[k].arg3 = NULL;
      s[k].arg4 = NULL;
   }
}
/*This function takes in the opCodeLine struct and a label name, after which it will search and return the address of
 * the label. If no label is found it will halt the program with an error code based on error found
 */
int find_opCodeLine(opCodeLine s[], char* label)
{
   int k;
   DEBUG_PRINT("Checking for duplicates for '%s'\n",label);
   for(k = 0;k < MAX_LINE_LENGTH + 1; k++)
   {
      if(s[k].label != NULL && strcmp(s[k].label,"") != 0  && strcmp(s[k].label,label) == 0)
      {
         return s[k].address;
      }
   }
   return -1;
}
void set_opCodeLine(opCodeLine * s,int address, char* label, char* opcode, char* arg1, char* arg2, char* arg3, char* arg4)
{
   DEBUG_PRINT("Setting opcodes into struct\n\t%X\t%s\t%s\t%s\t%s\t%s\t%s\n",address,label,opcode,arg1,arg2,arg3,arg4);
   char *tP;
   unsigned int len;

   s->address = address;
   
   len = strlen(label);
   tP = malloc(sizeof(char)*len+1);
   strcpy(tP,label);

   DEBUG_PRINT("%s - LEN:%d Tpm: %s\n",label,len,tP);
   s->label = tP;

   len = strlen(opcode);
   tP = malloc(sizeof(char)*len+1);
   strcpy(tP,opcode);

   DEBUG_PRINT("%s - LEN:%d Tpm: %s\n",opcode,len,tP);
   s->opcode = tP;

   len = strlen(arg1);
   tP = malloc(sizeof(char)*len+1);
   strcpy(tP,arg1);

   s->arg1 = tP;

   len = strlen(arg2);
   tP = malloc(sizeof(char)*len+1);
   strcpy(tP,arg2);

   s->arg2 = tP;

   len = strlen(arg3);
   tP = malloc(sizeof(char)*len+1);
   strcpy(tP,arg3);

   s->arg3 = tP;

   len = strlen(arg4);
   tP = malloc(sizeof(char)*len+1);
   strcpy(tP,arg4);

   s->arg4 = tP;
}
int get_r_value(char * reg)
{
   int registarnumber;
   reg++;
   registarnumber = reg[0] - '0';
   if(registarnumber > 8)
   {
      printf("Invalid Registar");
      exit(4);
   }
   return registarnumber;
}
int get_object_code(opCodeLine s[], int c)
{
   int object = 0;
   int temp = 0;
   if(strcmp(s[c].opcode,"add") == 0)
   {
      /*add the add 0001 opcode to the output*/
      object = 0x01;

      object = object << 3;
      /*add the dr to the object code*/
      object += get_r_value(s[c].arg1);
      DEBUG_PRINT("Add DR Registar: %d\n",get_r_value(s[c].arg1));
      
      object = object << 3;
      object+=get_r_value(s[c].arg2);
      
      /*check if we are adding between registars or imm*/
      DEBUG_PRINT("Add Mode %s\n",s[c].arg3);

      if(s[c].arg3[0] == 'r')
      {
         object = object << 6;
         object += get_r_value(s[c].arg3);
         DEBUG_PRINT("Add SR2 Registar: %d\n",get_r_value(s[c].arg3));
      }
      else
      {
         object = object << 1;
         object += 1;
         object = object << 5;
         /*check if imm5 is valud*/
         temp = toNum(s[c].arg3);
         if(temp > 32 || temp <= -32)
         {
            printf("imm5 is out of bounds for add");
            exit(4);
         }
         DEBUG_PRINT("imm value added %x\n",temp);
         temp = temp & 0x001F;
         object += temp;
      }
   }
   else if(strcmp(s[c].opcode,"and") == 0)
   {
      object = 0x5;

      DEBUG_PRINT("AND DR registar: %d\n",get_r_value(s[c].arg1));

      object = object << 3;
      object += get_r_value(s[c].arg1);

      DEBUG_PRINT("AND SR1 Registar: %d\n",get_r_value(s[c].arg2));

      object = object << 3;
      object += get_r_value(s[c].arg2);

      DEBUG_PRINT("AND ARG3 Mode %s\n",s[c].arg3);

      if(s[c].arg3[0]=='r')
      {
         object = object << 6;
         object += get_r_value(s[c].arg3);
         DEBUG_PRINT("AND ARG3 mode R%d\n",get_r_value(s[c].arg3));
      }
      else
      {
         object = object << 1;
         object +=1;
         object = object << 5;
         DEBUG_PRINT("AND ARG3 Mode imm %x\n",toNum(s[c].arg3));
         temp = toNum(s[c].arg3);
         temp = temp & 0x001F;
         object += temp;
      }
   }
   else if(strncmp(s[c].opcode,"br",2) == 0)
   {
      DEBUG_PRINT("BR opcode found '%s'\n",s[c].opcode);
      char * pBR = s[c].opcode + 2;
      if(pBR[0] == 0)
      {
         DEBUG_PRINT("BR nzp Case\n");
         object = 0;
         object = object << 7;
         object +=3;
         temp = find_opCodeLine(s,s[c].arg1) - s[c].address;
         DEBUG_PRINT("BR offset from %s:%x to %s:%x is %d\n",s[c].label,s[c].address,s[c].arg1,find_opCodeLine(s,s[c].arg1),temp);
         temp = temp & 0x01FF;
         /*since we have the address space in mod 2, we need to div this number by 2*/
         temp /=2;
         object = object << 9;
         object += temp;
      }
      else
      {
         object = 0;
         if(strchr(pBR,'n') != 0)
         {
            object += 0x4;
         }
         if(strchr(pBR,'z') != 0)
         {
            object += 0x2;
         }
         if(strchr(pBR,'p') != 0)
         {
            object += 0x1;
         }
         temp = find_opCodeLine(s,s[c].arg1) - s[c].address;
         DEBUG_PRINT("BR regular case: nzp - %x\n",object&0x7);
         DEBUG_PRINT("BR offset from %s:%x to %s:%x is %d\n",s[c].label,s[c].address,s[c].arg1,find_opCodeLine(s,s[c].arg1),temp);
         temp = temp & 0x01FF;
         temp /= 2;
         /*todo check bounds*/
         object = object << 9;
         object += temp;

      }
   }
   else if(strncmp(s[c].opcode,"jmp",3) == 0)
   {
      object = 0;
      DEBUG_PRINT("JMP opcode found '%s'\n",s[c].opcode);
      object = 0xc;
      DEBUG_PRINT("JMP stage0 built 0x%x\n",object);
      object = object << 6;
      object += get_r_value(s[c].arg1);
      DEBUG_PRINT("JMP stage1 built 0x%x\n",object);
      DEBUG_PRINT("JMP registar #%d\n",get_r_value(s[c].arg1));
      object = object << 6;
      DEBUG_PRINT("JMP stage2 built 0x%x\n",object);
   }
   else if(strncmp(s[c].opcode,"ret",3) == 0)
   {
      DEBUG_PRINT("RET opcode found '%s'\n", s[c].opcode);
      object = 0xc;
      object = object << 6;
      object += 7;
      object = object << 6;
   }
   else if(strcmp(s[c].opcode,"jsr") == 0)
   {
      DEBUG_PRINT("JSR opcode found '%s'\n",s[c].opcode);
      object = 0x4;
      object = object<< 1;
      object += 1;
      object = object  <<11;

      temp = find_opCodeLine(s,s[c].arg1) - s[c].address;
      DEBUG_PRINT("JSR offset %d\n",temp);
      temp /=2;

      temp = temp&0x7FF;
      /*checkbounds*/
      object += temp;
   }
   else if(strcmp(s[c].opcode,"jsrr") == 0)
   {
      DEBUG_PRINT("JSRR opcode found '%s'\n", s[c].opcode);
      object = 0x4;
      object = object << 6;
      object += get_r_value(s[c].arg1);
      DEBUG_PRINT("JSRR registar '%d'\n",get_r_value(s[c].arg1));
      object = object << 6;
   }
   else if(strcmp(s[c].opcode,"ldb") == 0)
   {
      DEBUG_PRINT("LDB opcode found '%s'\n",s[c].opcode);
      object = 0x2;
      object <<= 3;
      DEBUG_PRINT("LDB DR %d\n",get_r_value(s[c].arg1));
      object += get_r_value(s[c].arg1);
      object <<=3;
      DEBUG_PRINT("LDB BaseR %d\n",get_r_value(s[c].arg2));
      object += get_r_value(s[c].arg2);
      object <<=6;
      temp = toNum(s[c].arg3);
      DEBUG_PRINT("LDB boffset6 %d\n",temp);
      if(temp > 64 || temp <= -64)
      {
         printf("LDB boffset to large\n");
         exit(4);
      }
      temp &= 0x3F;
      object += temp;
   }
   else if(strcmp(s[c].opcode,"ldw") == 0)
   {
      DEBUG_PRINT("LDW opcode found '%s'\n",s[c].opcode);
      object = 0x6;
      object <<= 3;
      DEBUG_PRINT("LDW DR %d\n",get_r_value(s[c].arg1));
      object += get_r_value(s[c].arg1);
      object <<=3;
      DEBUG_PRINT("LDW BaseR %d\n",get_r_value(s[c].arg2));
      object += get_r_value(s[c].arg2);
      object <<=6;
      temp = toNum(s[c].arg3);
      DEBUG_PRINT("LDW boffset6 %d\n",temp);
      if(temp > 64 || temp <= -64)
      {
         printf("LDW boffset to large\n");
         exit(4);
      }
      temp &= 0x3F;
      object += temp;
   }
   else if(strcmp(s[c].opcode,"lea") == 0)
   {
      DEBUG_PRINT("LEA opcode found '%s'\n",s[c].opcode);
      object = 0xE;
      object <<=3;
      DEBUG_PRINT("LEA DR %d\n",get_r_value(s[c].arg1));
      object += get_r_value(s[c].arg1);
      object <<=9;
      temp = find_opCodeLine(s,s[c].arg2) - s[c].address;
      DEBUG_PRINT("LEA PCOffset %d\n",temp);
      if(temp > 511 || temp < -512)
      {
         printf("LEA PC offset to large\n");
         exit(4);
      }
      temp &= 0x1FF;
      object += temp;
   }
   else if(strcmp(s[c].opcode,"not") == 0)
   {
      DEBUG_PRINT("NOT opcode found '%s'\n",s[c].opcode);
      object = 0x9;
      object <<=3;
      DEBUG_PRINT("NOT DR %d\n",get_r_value(s[c].arg1));
      object += get_r_value(s[c].arg1);
      object <<= 3;
      DEBUG_PRINT("NOT SR %d\n",get_r_value(s[c].arg2));
      object += get_r_value(s[c].arg1);
      object <<=6;
      object += 0x3F;
   }
   else if(strcmp(s[c].opcode,"rti") == 0)
   {
      DEBUG_PRINT("RTI opcode found '%s'\n",s[c].opcode);
      object = 0x8;
      object <<=12;    
   }
   else if(strcmp(s[c].opcode,"lshf") == 0)
   {
      
   }

      return object;
}
int main(int argc, char* argv[])
{
   char *prgName = NULL;
   char *iFileName = NULL;
   char *oFileName = NULL;

   /*check for missing inputs*/
   if(argc < 3)
   {
      printf("%s\n","Missing arguments");
      exit(4);
   }
   prgName = argv[0];
   iFileName = argv[1];
   oFileName = argv[2];

   char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
   opCodeLine symboles[MAX_LINE_LENGTH+1];
   init_opCodeLine(symboles,MAX_LINE_LENGTH+1);

   int lRet;
   int address;
   int k = 0,c = 0;
   FILE * lInFile;

   lInFile = fopen(iFileName,"r");
   address = 0;
   /*Preform Pass 1 to gain symbol table*/
   do
   {
      lRet = readAndParse(lInFile,lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
      if(lRet != DONE && lRet != EMPTY_LINE)
      {
         DEBUG_PRINT("Working on %s\n",lLine);
         DEBUG_PRINT("label: %s, opcode: %s, arg1: %s, arg2: %s, arg3: %s, arg4: %s\n",lLabel,lOpcode,lArg1,lArg2,lArg3,lArg4);
         /*Check for .ORIG assimblier instruction*/
         if(strcmp(lOpcode,".end") == 0)
         {
            DEBUG_PRINT("Found .end Command, starting phase 2 scan\n");
            lRet = DONE;
         }
         else if(strcmp(lOpcode,".orig") == 0)
            
         {
            address = toNum(lArg1);
            DEBUG_PRINT("Found .ORIG Statment\n");
         }
         else
         {   
            /*check for Address depended instrctions*/
            if(find_opCodeLine(symboles,lLabel)!= -1)
            {
               DEBUG_PRINT("Found a duplicate label\n");
               printf("Duplicate Label: '%s'\n",lLabel);  
               exit(4);
            }
            set_opCodeLine(&symboles[k],address,lLabel,lOpcode,lArg1,lArg2,lArg3,lArg4);

            address+=2;
            k+=1;
         }
      }
   }while(lRet != DONE);

   debug_printOpCodeLines(symboles,MAX_LINE_LENGTH+1);
   /*Phase 2 Scan, Convert to Object Code*/
   printf("OBJ: 0x%x\n",symboles[0].address);
   for(c = 0; c < k; c++)
   {
      printf("OBJ: 0x%x\n",get_object_code(symboles,c));
   }
}

