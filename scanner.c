//scanner.c
//nick powers
//26 feb 2016
//
//child of driver inheriting FDs
//
//file descriptors:
// 0 (stdin) <-- some file
// 1 (stdout) <-- stdout
// 2 (stderr) <-- stderr
// 3 <-- write to even
// 4 <-- read from even
// 5 <-- write to odd
// 6 <-- read from odd
//
//read words (separated by ' ', \n, \t) from stdin
//strip off non-alphanumerics and determine 
//the word length. 
//write even length words to even, odd to odd
//
//upon EOF from stdin, scanner will close
//output pipes, and enter infinite loop
//printing out an * each second
//when it receives a signal from even
//enter the signal handler:
//read from the even/odd pipes 
//and print to stdout
//
//GeneralDescription:
//     The driver handles the file input,
//     spawns the scanner, even, and odd.
//
//     the scanner pre-processes a text file 
//     and sends words with an even number of characters to even,
//     and odd number to odd. These two processes record (dynamically) the number
//     of words received and the number of times appeared respectively.
//
//	When there is no more input, scanner closes its output pipes
//	and waits for SIGTERM from even, and then receives output to print
// 	from even and then from odd.
//
//	Uses environment variable EODELAY, fork(), dup(), execl()
//***********************************************

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

typedef void (*sighandler_t (int));

//**********************************************
//signal handler
void handler(int unused){

  int k;
  char ch;  
  int r_even = 4;
  int r_odd = 6;

  //fprintf(stderr,"scanner: entered signal handler\n");
  //print the even list, read from even fd
  fprintf(stdout,"\n***************************\n");
  fprintf(stdout,"Words with even letters:\n");

  k = 1;
  while((k = read(r_even,&ch,1)) > 0){
      fprintf(stdout,"%c",ch);
  }//while r_even is not EOF
    

  
  //print odd list, read from odd fd
  fprintf(stdout,"\n***************************\n");
  fprintf(stdout,"Words with odd letters:\n");   

  k = 1;
  while((k = read(r_odd,&ch,1)) > 0){
    fprintf(stdout,"%c",ch);
  }//while r_odd is not EOF
  
  fprintf(stdout,"\n");
  exit(0);
}//signal handler
//********************************************

int main(int argc, char *argv[]){
  char readbuf[130];
  char writebuf[130];
  int i;
  int j;
  int evenodd;//if set to 0, write to even, 1: odd
  int w_val;

  int write2_odd_fd = 5;//fd's
  int write2_even_fd = 3;


  //***************************
  //SIGNAL HANDLER
  signal(SIGTERM,handler);//even will send SIGTERM
  //*************************** 

  //fprintf(stderr,"scanner: opened scanner\n");
  //fprintf(stdout,"scanner: in scanner, print to stdout\n");

  i = 0;
  j = 0;
  //reading input from stdin
  //*********************************************************
  //fprintf(stderr,"scanner: about to read from stdin\n");
  while(scanf("%s",readbuf) != EOF){
    //fprintf(stderr,"scanner: readbuf == %s\n", readbuf);
    i = 0;//reset buffer counter variables
    j = 0;
    while(readbuf[i] != '\0'){
      if(isalnum(readbuf[i])){
	writebuf[j] = tolower(readbuf[i]);
	j++;
      }//if alphanumeric
      i++;//increment readbuf ct to skip over non-alphanumerics
    }//while readbuf[i] != '\0'

    evenodd = j % 2;//determine even/odd length word
    writebuf[j] = ' ';
    j++;

    writebuf[j] = '\0';
    //fprintf(stderr,"%s: writebuf == '%s', %d\n",
    //	    argv[0],writebuf,j);
    
    if(j < 2){   
      //fprintf(stderr,"scanner: word len(%s) < 2\n",writebuf);
    }//if not a word

    else{
      if(evenodd == 0){//write to even on fd 
	//fprintf(stderr,"%s: to even, about to write: '%s'\n",argv[0],writebuf);
	if((w_val = write(write2_even_fd, writebuf, strlen(writebuf))) 
	   != strlen(writebuf)){
	  //fprintf(stderr,"scanner: bad write to even\n");
	}//if bad write to even
      }//if evenodd == 0: write to even
      
      else if(evenodd == 1){
	//fprintf(stderr,"%s: to odd, about to write: '%s'\n",argv[0],writebuf);
	if((w_val = write(write2_odd_fd, writebuf, strlen(writebuf))) 
	   != strlen(writebuf)){
	  //fprintf(stderr,"scanner: bad write to odd\n");
	}//if bad to odd
      }//if evenodd == 1: write to odd
      
      else{
	fprintf(stderr,"scanner: ya done f***ed up...\n");
      }//else evenodd went FUBAR
    }//else strlen >= 1

  }//while more words from file
  //********************************************************
  //fprintf(stdout,"scanner: print to stdout AGAIN!!!\n");
  /*fprintf(stderr,"scanner: finished reading from stdin 
    and writing to even/odd\n");*/

  //done with input, time to close the FD's
  //**************************************************
  //fprintf(stderr,"scanner: about to close fd's\n");

  close(write2_odd_fd);
  close(write2_even_fd);
  //fprintf(stderr,"scanner: close odd returns %d\n",close(write2_odd_fd));
  //fprintf(stderr,"scanner: close even returns %d\n",close(write2_even_fd));

  //fprintf(stderr,"scanner: closed fd's, about to print *'s\n");
  while(1){
    //fprintf(stderr,"*");
    fprintf(stdout,"*");
    fflush(stdout);
    sleep(1);
  }//while true
  exit(1);
}//main
