//read_test.c
//27 feb 2016
//nick powers
//
//test code for system call lseek(), open(2), 
//read(),write()
//
//1 command line arg: ./read_test inputfile.txt
//output: prints the file
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>


int main(int argc, char *argv[]){

  int input;
  int r;
  int i;
  char buf[128];
  

  if(argc != 2){
    fprintf(stderr,"read(2)_test: argc != 2\n");
    exit(1);
  }//no agument

  else if((input = open(argv[1],O_RDONLY)) < 0){
    perror("lseek_test");
    exit(1);
  }//if bad open
  //fprintf(stderr,"input value: %d\n", input);

  //read returns int of units successfully copied
  while((r = read(input,buf,10)) > 0){
    //fprintf(stderr,"[loop]");
    
    //when the retval < request copy, you're at EOF
    //this if reads the remainder
    if(r < 127){
      //fprintf(stderr,"partial read\n");
      i = 0;
      
      //while more to print from remainder
      while(i != r){
	fprintf(stderr,"%c",buf[i]);
	i++;
      }//while i != r, i.e. not at end of partial copy
    }//if partial read
   
    //else just print the full buffer
    else{
    fprintf(stderr,"%s", buf);
    }//else print all
  }//while read >= 0 
  exit(0);
}//main