//evenodd.c
//nick powers
//24 feb 2016
//
//This code will be executed by the driver,
//either as "even" which behaves one way with input,
//or as "odd". The behavior itself is trivial:
//
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
#include <ctype.h>

//definition for linked lists
//adapted from Kearns' slide set 1, slides 31-33
struct wordlist{
  char *word;
  int ct;
  struct wordlist *next;
};


int main(int argc, char *argv[], char *envp[]){

  //int evenodd;
  int pid;
  int eodelay;
  int i,k,inword;
  char ch;
  char buf[128];
  char *delaystr;
  char *end;
  char w_buf[1024];

  struct wordlist *list_head =NULL;
  struct wordlist *list_tail = NULL;
  struct wordlist *newnode = NULL;
  struct wordlist *temp = NULL;
  struct wordlist *trav = NULL;


  //*****************************************
  //if running in even
  if(strstr(argv[0], "even") != NULL){
    //fprintf(stderr,"in even\n");
    
    if(argc != 2){
      fprintf(stderr,"evenodd usage: ./even 0 < inputfile\n");
      exit(1);
    }//if bad arg count
    
    pid = strtol(argv[1], &end, 0);
    if(*end != '\0'){
      fprintf(stderr,"evenodd: arg[1] contains invalid characters for dec input\n");
      exit(1);
    }//if arg1bad input
    else{
      //fprintf(stderr,"even: pid == %d\n",pid);
    }//else good
  }//if even
  
  else if(strstr(argv[0],"odd") != NULL){
    //fprintf(stderr,"in odd\n");
  }//else if odd

  else {
    fprintf(stderr,"evenodd: something weird happened\n");
    exit(1);
  }//else something weird happened
  //done with command line checking

  if((delaystr = getenv("EODELAY")) != NULL){
    //fprintf(stderr,"%s: got EODELAY\n",argv[0]);
    eodelay = strtol(delaystr, &end, 0);
    if(*end != '\0'){
      fprintf(stderr,"%s: strtol failed\n",argv[0]);
      eodelay = 0;
    }//if string to long failed
    else if(eodelay > 0){
      //fprintf(stderr,"%s: eodelay == %d\n",argv[0], eodelay);
    }//else successful positive integer returned
  }//if eodelay is set
  else{
    //fprintf(stderr,"%s: no eodelay found\n",argv[0]);
    eodelay = 0;}
  
  //fprintf(stderr,"%s: begin reading\n",argv[0]);
  //words from scanner
  //****************************************************
  i = 0;
  //j = 0;
  trav = temp = list_head;
  //reading from stdin
  k = 1;//initialize k to enter read loop
  while(k > 0){//outer while loop
    //fprintf(stderr,"%s: reading, k > 0\n",argv[0]);
    i = 0;//index variable
    inword = 1;//
    while(inword == 1){
      if((k = read(0,&ch,1)) != 1){
	//fprintf(stderr,"%s: k == %d\n",argv[0],k);
	inword = -1;
      } //if EOF or bad read
      else{
	//fprintf(stderr,"%s: c == '%c'\n",argv[0],ch);
	if((ch != ' ') && (ch != '\n') && (ch != '\t')){
	  buf[i] = ch;
	  i++;
	}//if not white space (valid char)
	else{
	  buf[i] = '\0';
	  inword = 0;
	}//else white space, clean up end of buf 
      }//else valid read

    }//while char read was alpha numeric 
      
    
    if((i == 0) || (k == 0) ){
      //k = read(0,&ch,1);//do nothing
    }//if word not found
    
    else{//append word or inc word counter


      //******************************************
      //sleep(eodelay) after receiving word
      if(eodelay != 0){
	sleep(eodelay);
      }//sleep for eodelay seconds
      
      if((newnode = (struct wordlist *)malloc(sizeof(struct wordlist))) == NULL){
	fprintf(stderr,"%s: malloc failure for newnode\n",argv[0]);
	exit(1);
      }//if bad malloc for newnode
      if((newnode->word = malloc(strlen(buf)+1)) == NULL){
	fprintf(stderr,"%s: malloc failure for newnode \n",argv[0]);
	exit(1);
      }//if bad malloc for newnode->word
      if(strncpy(newnode->word, buf, strlen(buf)+1) == NULL){
	fprintf(stderr,"%s: strncpy problem for newnode\n", argv[0]);
	exit(1);
      }//if bad strncpy
      //fprintf(stderr,"successful mallocs\n");
      if(list_tail == NULL){
	//fprintf(stderr,"first word\n");
	list_head = list_tail = newnode;
	newnode->ct = 1;
	newnode->next = NULL;
      }//if list_tail == NULL: first word
      else{
	//fprintf(stderr,"another word\n");
	trav = temp = list_head;
	
	//Traverse the List and Add Words
	while(trav != NULL){
	  //fprintf(stderr,"traversing the wordlist\n");
	  if(strcmp(newnode->word,trav->word) > 0){
	    //fprintf(stderr,"neword > travword\n");
	    if(trav->next == NULL){
	      //fprintf(stderr,"%s: %s == new list_tail\n",argv[0],trav->word);
	      trav->next = newnode;
	      list_tail = newnode;
	      newnode->ct = 1;
	      newnode->next = NULL;
	      trav = NULL;
	    }//if trav == NULL: at end of list, append
	    else{
	      //fprintf(stderr,"else continue traversing\n");
	      temp = trav;
	      trav = trav->next;
	    }//else continue traversing
	  }//if (newnode->word) > (trav->word)
	  
	  else if(strcmp(newnode->word,trav->word) == 0){
	    //fprintf(stderr,"match found\n");
	    //fprintf(stderr,"%s: '%s', ct == %d in list\n",
	    //argv[0],trav->word,trav->ct);
	    trav->ct++;
	    trav = NULL;
	  }//if newnode->word found in list
	  
	  else if(strcmp(newnode->word,trav->word) < 0){
	    //fprintf(stderr,"neword < travword\n");
	    if(trav == list_head){
	      //fprintf(stderr,"%s: %s == new list_head\n",argv[0],trav->word);
	      newnode->ct = 1;
	      newnode->next = trav;
	      list_head = newnode;
	    }//if beginning of list, head becomes newnode
	    else{
	      //fprintf(stderr,"%s: inserted '%s' in list\n",argv[0],trav->word);
	      newnode->ct = 1;
	      newnode->next = temp->next;
	      temp->next = newnode;
	    }//else insert newnode into list
	    trav = NULL;//word inserted, escape traversal
	  }//else new entry in between nodes
	}//while trav != NULL: still traversing list
      }//else not the first addition to the list
      //fprintf(stderr,"%s: inserted '%s'\n",argv[0],newnode->word);
    }//else do list stuff
    //fprintf(stderr,"%s: inserted word, k == %d\n",argv[0],k);
  }//while there is more data to read
  //***************************************************
  
  //fprintf(stderr,"%s: input == EOF\n",argv[0]);
  //if (scanner called me) && (I'm even)
  
  if(pid != 0){
    sleep(10);         //sleep 10 seconds
    kill(pid,SIGTERM); //signal scanner
  }//if ./even && even has a parent
  else{
    if(strcmp(argv[0],"./even") == 0){
      fprintf(stdout,"\n***************************\n");
      fprintf(stdout,"Words:\n");
    }//
  }//print header
  //fprintf(stderr,"%s: after signal code\n",argv[0]);
  //write to stdout the (already alphabetized) word/ct pairs
  //formatted with tabs etc.
  trav = list_head;
  if(trav == NULL){
    //fprintf(stderr,"%s: list head is NULL\n",argv[0]);
  }//if head == NULL

  while(trav != NULL){
    sprintf(w_buf,"%s\t\t%d\n",trav->word,trav->ct);
    i = 0;
    //fprintf(stderr,"%s: about to print: %s\n",argv[0],trav->word);
    while(i < strlen(w_buf)){
      if((k = write(1,&w_buf[i],1)) != 1){
	//fprintf(stderr,"evenodd: bad write in word\n");
      }//if bad write
      i++;
    }//while i < strlen(wbuf): while not at \0
    trav = trav->next;
  }//while still more to print

  //fprintf(stderr,"%s: normal exit\n",argv[0]);
  exit(0);
}//main