#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_BUF_SIZE 512

int multicheck(char* symbol, char* copy, char* temp, char* str, int i, char c, int cases){
  if(cases==1){ // check if special character is presented ones or more than ones (locate errors like '&&&' instead of '&&')
  if((symbol[i]==c && copy[temp-str+strlen(temp)+1]==c && copy[temp-str+strlen(temp)+2]==c) || (symbol[i]==c && copy[temp-str+strlen(temp)+1]!=c  )  ){
    return 0;
  }
}else if(cases==2){
  if(symbol[i]==c && copy[temp-str+strlen(temp)+1]==c){
    return 0;
  }
}else if(cases==3){
  if(symbol[i]==c && copy[temp-str+strlen(temp)+1]==c && copy[temp-str+strlen(temp)+2]==c  ){
    return 0;
  }else if(symbol[i]==c && copy[temp-str+strlen(temp)+1]==c){
    return 2;
  }
}
return 1;
}

char** split(char* str, const char* splitter, char* symbol){ // splits commands by symbols specified by "splitter" - keeps these symbols in "symbol"

int i=0;
char * temp= (char*)malloc(sizeof(char)*MAX_BUF_SIZE);
char ** commands = (char**)malloc(sizeof(char*)*MAX_BUF_SIZE);
char *copy = strdup(str);


temp = strtok (str,splitter);
if(temp==NULL) return NULL;
while (temp)
{
  commands[i]=temp;
  symbol[i]= copy[temp-str+strlen(temp)];

  if(multicheck(symbol,copy, temp, str, i,'&',1)==0) return NULL; // check if this character is presented ones (locate errors like '&&&' instead of '&&')
  if(multicheck(symbol,copy, temp, str, i,'|',2)==0) return NULL;
  if(multicheck(symbol,copy, temp, str, i,';',2)==0) return NULL;
  if(multicheck(symbol,copy, temp, str, i,'<',3)==0) return NULL;
  if(multicheck(symbol,copy, temp, str, i,'>',3)==0) return NULL;

  if(multicheck(symbol,copy, temp, str, i,'>',3)==2){
    i++;
    symbol[i]='>';
  }
  temp = strtok (NULL, splitter);
  i++;
}


if(strcmp(commands[0],str)==0) symbol=NULL;


free(temp);
free(copy);
return commands;
}

void child(char* cmd){ // uses execvp() to execute command
  char** words=(char**)malloc(MAX_BUF_SIZE*sizeof(char*) );
  char* sym=(char*)malloc(MAX_BUF_SIZE*sizeof(char*));

  if(cmd==NULL) exit(-1);
  words=split(cmd," ",sym);
  if( words==NULL) exit(-1);

  execvp(words[0], words);
  free(sym);
  free(words);
  exit(-1);
}


void EXECfunc(char* cmd,int * status){ // creates child proccess to execute command
  int pid=fork();
  if(pid==0){
    child(cmd);
  }
  wait(status);
  if(status[0]!=0) printf("Error Command \"%s\" is invalid \n",cmd);
}


void greaterfunc(char* cmd1,char* cmd2, int* status,const char* job){ // redirection >>, > implementation
  FILE* fp;
  char* temp=(char*)malloc(sizeof(char));

  cmd2=split(cmd2," ",temp)[0]; // split by spaces
  free(temp);

  fp=fopen(cmd2,job);
  if (fp==NULL) return;

  int pid=fork();
  if(pid==0){
    dup2(fileno(fp),fileno(stdout)); // redirect output from stdout to file
    child(cmd1);
    close(fileno(fp));
    exit(0);
  }
  fclose(fp);
  wait(status);

  if(status[0]!=0) printf("Error Command \"%s\" is invalid \n",cmd1);
}

void lessfunc(char* cmd1,char* cmd2, int* status,const char* job){ // redirection < implementation
  FILE* fp;
  char* temp=(char*)malloc(sizeof(char));

  cmd2=split(cmd2," ",temp)[0]; // split by spaces
  free(temp);

  fp=fopen(cmd2,job);
  if (fp==NULL){
    printf("There is no such file or directory \n");
    return;
  }

  int pid=fork();
  if(pid==0){
    dup2(fileno(fp),fileno(stdin)); // redirect input from stdin to file
    child(cmd1);
    close(fileno(fp));
    exit(0);
  }
  fclose(fp);
  wait(status);

  if(status[0]!=0) printf("Error Command \"%s\" is invalid \n",cmd1);
}

void PIPEfunc(char* cmd1,char* cmd2, int* status){
  int* fd = (int*)malloc(2*sizeof(int));;
  pipe(fd); // create pipe

  int pid=fork();
  if(pid==0){
    close(fd[0]);
    dup2(fd[1],1); // redirect output from parent to pipe
    close(fd[1]);
    child(cmd1);
  }

  pid=fork();
  if(pid==0){
    close(fd[1]);
    dup2(fd[0],0); // redirect input from parent to pipe
    close(fd[1]);
    child(cmd2);
  }
  close(fd[0]);
  close(fd[1]);

  wait(status);
  if(status[0]!=0) printf("Error Command \"%s|%s\" is invalid \n",cmd1,cmd2);
  wait(status);
  if(status[0]!=0) printf("Error Command \"%s\" is invalid \n",cmd2);

}

void my_cd(char* cmd,int * status){ // cd command implementation
  char** words=(char**)malloc(MAX_BUF_SIZE*sizeof(char*) );
  char* sym=(char*)malloc(MAX_BUF_SIZE*sizeof(char));

  words=split(cmd," ",sym); // split by spaces
  if(chdir(words[1])!=0) printf("There is no such file or directory \n");
}

void loop(char* arg){
  char* input=(char*)malloc(MAX_BUF_SIZE*sizeof(char) );
  char** cmd=(char**)malloc(MAX_BUF_SIZE*sizeof(char*) );
  char** redir=(char**)malloc(MAX_BUF_SIZE*sizeof(char*) );
  char *symbol=(char*)malloc(MAX_BUF_SIZE*sizeof(char) );
  char *symbol2=(char*)malloc(MAX_BUF_SIZE*sizeof(char) );
  int index,j,count,count2;
  int* status=(int*)malloc(sizeof(int) );
  FILE* fp ;
  char c;
  count2=0;

  if(arg!=NULL) { // batch mode
    fp = fopen(arg,"r"); // read from batch file
    if (fp==NULL) {
      printf("Error File \"%s\" is invalid \n",arg);
      exit(-1);
    }else{
      count=0; // count letters of file
      while((c=fgetc(fp))!=EOF){
        count++;
      }
      fclose(fp);
      if(count==0) exit(0); //empty file
      fp = fopen(arg,"r"); // reopen file
    }
  } // otherwise interactive mode
  while(1){ // endless loop - next command

    index=0;

    if(arg!=NULL){ // batch mode
      j=0; // reset index of buffer for every line
      memset(input, 0, MAX_BUF_SIZE); // clear buffer - getting ready for the next line
      while(count2<=count){ // get next char until EOF
        c=fgetc(fp);
        input[j]=c;
        j++; // index of buffer
        count2++; // iterate through file
        if(c=='\n') { // after every line break the loop to get next command
          break;
        }
      }

      if(count2==count) {
        exit(0); // if last line is not quit, exit normally
      }
    }else{ // interactive mode
      printf("letros_8851> ");
      fgets(input,MAX_BUF_SIZE,stdin);
    }

    if(strcmp(input,"\n")==0 || strcmp(input,"")==0){
      continue; // skip empty line
    }
    if(strlen(input)>=MAX_BUF_SIZE-1){
      printf("Error Buffer Overflow\n"); // avoid Buffer Overflow
      continue;
    }

    input[strlen(input)-1]='\0'; // add \0 character for execvp()

    cmd=split(strdup(input),"&;",symbol); // split command by these symbols

    if (cmd==NULL) {
      printf("Error Command \"%s\" is invalid \n",input); // avoid errors, such as '&&' without command after it
      continue;
    }

    if(strtok(strdup(cmd[0])," ")==NULL) continue;


    j=0;
    do{

      redir=split(strdup(cmd[j]),"<>|",symbol2); // split command by these symbols
      if(redir==NULL){
        printf("Error Command \"%s\" is invalid \n",input);
        j++;
        index++;
        continue;
      }
      if(strcmp(redir[0],cmd[j])!=0){
        if(symbol2[0]=='>'){
          if(symbol2[1]=='>'){
            greaterfunc(redir[0],redir[2],status,"a"); // >> append to file
          }else{
            greaterfunc(redir[0],redir[1],status,"w"); // > write to file
          }
        }else if(symbol2[0]=='<'){ // read form file
          lessfunc(redir[0],redir[1],status,"r");
        }else if(symbol2[0]=='|'){ // pipeline
          PIPEfunc(redir[0],redir[1],status);
        }
      }else{
        if(strtok(strdup(cmd[j])," ")==NULL){
          printf("Error Command \" \" is invalid \n");
          j++;
          index++;
          continue;
        }

        if(strcmp(strtok(strdup(cmd[j])," "),"quit")==0)  exit(0); // check for quit

        if(strcmp(strtok(strdup(cmd[j])," "),"cd")==0)  { // check for cd
          my_cd(cmd[j],status);
          j++; //move to next part of command
          index++; // check next special symbol (&;|<>)
          continue;
        }

        if(symbol[index]=='&'){
          EXECfunc(cmd[j],status);
          if(status[0]!=0) { //skip next command if invalid
            j++;
          }
        }else if(symbol[index]==';'){
          EXECfunc(cmd[j],status);
        }else{
          EXECfunc(cmd[j],status);
        }
      }
      index++;
      j++;
    }while(cmd[j]!=NULL); // until all parts of current command are executed
  }
  free(input);
  free(symbol);
  free(status);
  free(input);
  free(cmd);
  if(arg!=NULL) fclose(fp);
  return;
}

int main(int argc, char **argv) {
  if(argc>2){ // if more than one batch file is given as input, exit
    return -1;
  }else if(argc==2){ // if batch file is given, enable batch mode
    loop(argv[1]);
  }
  else{ // else enable interactive mode
    loop(NULL);
  }
  return 0;
}
