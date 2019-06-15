//driver.c
//nick powers
//18 feb 2016
//
//program experiment with forking and coordinated
//data handling among processes.
//
//driver 
//-takes one command line arg
//-checks for successful open with RDONLY, then closes
//-sets up multiple pipes
//-spawns 3 children:
//   scanner, even, odd which then exec.
//-waits for the children to terminate before exit
//scanner is given driver's argv[1] 
//and pipes to read/write to and from even and odd
//scanner maintains inherited stdin/stdout from driver
//
//even and odd's stdin/stdout is set to the pipes 
//to and from scanner
//
//Description:
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


//
//*****************************************************

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

int main(int argc, char *argv[]){

  int scanner;//will become pids for the children
  int even;
  int odd;

  int scan2even[2];//will become fd's for children
  int even2scan[2];
  int scan2odd[2];
  int odd2scan[2];
  int infile;//input file
  int status;//used for exits
  char pid[11];

  //arg checking
  if(argc != 2){
    fprintf(stderr,"driver usage: ./driver inputfile.txt\n");
    exit(1);
  }//if bad argc

  if((infile = open(argv[1],O_RDONLY)) < 0){
    perror("driver");
    exit(1);
  }//if bad file open

  pipe(scan2even);
  pipe(even2scan);
  pipe(scan2odd);
  pipe(odd2scan);
  //fprintf(stdout,"driver: printed to stdout\n");

  //scanner code--------------------------------------------------
  if((scanner = fork()) == 0){
    //fprintf(stdout,"driver: scanner: print to stdout\n");

    //close unneeded pipes
    close(scan2even[0]);//scanner will only write to these pipes
    close(scan2odd[0]); //so close the read parts
    close(even2scan[1]);//scan will only read from these pipes;
    close(odd2scan[1]);//so close the write parts

    close(0);//close scanner's stdin
    dup(infile);//duplicate the fstream to stdin
    close(infile);//close the current instance of the fstream

    //scan will write to even on fd 3
    close(3); dup(scan2even[1]); close(scan2even[1]);

    //scan will read from even on fd 4
    close(4); dup(even2scan[0]); close(even2scan[0]);

    //scan will write to odd on fd 5
    close(5); dup(scan2odd[1]); close(scan2odd[1]);
    
    //scan will read from odd on fd 6
    close(6); dup(odd2scan[0]); close(odd2scan[0]);

    //fprintf(stdout,"driver: scanner: print to stdout AGAIN\n");
    //execute scanner code
    //fprintf(stderr,"driver: scanner: even2scan[0] == %d, odd2scan == %d\n",
    //	    even2scan[0],odd2scan[0]);
    //    fprintf(stderr,"scanner: ");
    
    execl("scanner", "scanner", argv[1], (char*)NULL);
    fprintf(stderr,"driver: scanner returned from execl\n");
    exit(1);
  }//if scanner

  //even code-----------------------------------------------------
  if((even = fork()) == 0){

    //fprintf(stderr,"driver: even\n");
    //even's stdin becomes scan2even pipe
    close(0); dup(scan2even[0]); close(scan2even[0]);  
    //fprintf(stderr,"driver: even scan2even[0] == %d\n",scan2even[0]);
    //even's stdout becomes even2scan pipe
    close(1); dup(even2scan[1]); close(even2scan[1]);
    //fprintf(stderr,"driver: even even2scan[1] == %d\n",even2scan[1]);

    //close useless pipes for even
    close(scan2even[1]);//even will only read here, so close write
    close(even2scan[0]);//even will only write here, so close read

    close(scan2odd[0]);
    close(scan2odd[1]);//even doesn't need odd/scan pipes
    close(odd2scan[0]);
    close(odd2scan[1]);

    sprintf(pid,"%d",scanner);
    
    execl("even","even", pid, (char *)NULL);
    fprintf(stderr,"even failed to execl\n");
    exit(1);
  }//if even

  //odd code---------------------------------------------------
  if((odd = fork()) == 0){
    
    //odd's stdin becomes scan2odd
    close(0); dup(scan2odd[0]); close(scan2odd[0]);

    //odd's stdout becomes odd2scan
    close(1); dup(odd2scan[1]); close(odd2scan[1]);
    
    //close useless pipes for odd
    close(scan2odd[1]);//odd will only read here, so close write
    close(odd2scan[0]);//   ...        write here, so close read

    close(scan2even[0]);
    close(scan2even[1]);//odd doesnt need even/scanner pipes
    close(even2scan[0]);
    close(even2scan[1]);

    execl("odd","odd", (char *)NULL);
    fprintf(stderr,"odd failed execl\n");
    exit(1);
  }//if odd

  //driver --------------------------------------------------------- 
  else{
  //wait for the 3 children
    
    close(scan2even[0]);
    close(scan2even[1]);
    close(even2scan[0]);
    close(even2scan[1]);
    close(scan2odd[0]);
    close(scan2odd[1]);
    close(odd2scan[0]);
    close(odd2scan[1]);
    
    

    wait(&status);
    wait(&status);
    wait(&status);
    
  }//else driver
  exit(0);
}//main