#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define INPUT_STRING_SIZE 80

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

process* f_process= NULL;

int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_help(tok_t arg[]);

/* Command Lookup Table Structures */
typedef int cmd_fun_t (tok_t args[]); // cmd functions take token array and return int
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;


// add more commands to this lookup table!
fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_cd , "cd" , "change directory"},
  {cmd_wait , "wait" , "Waiting for process to finish"},
  {cmd_fg , "fg" , "process in foreground"},
  {cmd_bg , "bg" , "process in background"},
  
};

int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

void init_shell()
{
  // Check if we are running interactively
  shell_terminal = STDIN_FILENO;

  // Note that we cannot take control of the terminal if the shell is not interactive
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    // force into foreground
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    // Put shell in its own process group
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    // Take control of the terminal
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  
  SET_SIGNALS(SIG_IGN);
  // ignore signals
}

/**
 * Add a process to our process list
 */
void add_process(process* p)
{
  
    if(f_process == NULL){
        f_process =p;
        p->next =p;
        p->prev = p;
  }
   else{
        f_process->prev->next=p;
        p->prev= f_process->prev;
        f_process->prev = p;
        p->next = f_process; 
     }
}

/**
 * Creates a process given the tokens parsed from stdin
 *
 */
process* create_process(char* inputString)
{
  int outfile = STDOUT_FILENO; 
  
  char *out_redirect = strstr( inputString, ">" );
  if( out_redirect != NULL ){
    
    *out_redirect = NUL; 
    char *output = out_redirect + 1; 
    
    char *file = getToks(output)[0];
    outfile = open( file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
   
     if( outfile < 0 ) {
       perror( file );
       return NULL;
     }
  }

  int infile = STDIN_FILENO; 
 
  char *in_redirect = strstr(inputString, "<");
  if( in_redirect != NULL ){
    
    *in_redirect = NUL; 
    char *input = in_redirect + 1; 
   
    char *file = getToks(input)[0];
    infile = open( file, O_RDONLY );
    
    if( infile < 0 ){
      perror( file );
      return NULL;
    }
  }


  bool is_background = false;
  char* bg = strchr(inputString, '&');
  if( bg != NULL ){
    is_background = true;
    bg = NUL; 
  }

  
  tok_t *t = getToks( inputString );  
  int fundex = lookup(t[0]);
  if (fundex >= 0) { 
    
    cmd_table[fundex].fun(&t[1]);
    return NULL;
  }
  else { 
    process* p = (process*) calloc( 1, sizeof(process) );
    p->stdin = infile;
    p->stdout = outfile;
    p->stderr = STDERR_FILENO;
    p->background = is_background;
    p->next = p->prev = NULL;
    
    int argc = 0;
    for( tok_t *tok = t; *tok != NUL; ++tok )
      ++argc;
    p->argc = argc;
    p->argv = t;
    
    add_process( p );
    return p;
  }
}
  
}



int shell (int argc, char *argv[]) {
  pid_t pid = getpid();		 // get current process's PID
  pid_t ppid = getppid();	 // get parent's PID
  pid_t cpid;                // use this to store a child PID

  char *s = malloc(INPUT_STRING_SIZE+1); // user input string
  tok_t *t;			                     // tokens parsed from input
  // if you look in parse.h, you'll see that tokens are just c-style strings
  
  int lineNum = 0;
  int fundex = -1;
  
  // perform some initialisation
  init_shell();

  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  // change this to print the current working directory before each line as well as the line number.
  fprintf(stdout, "%d: ", lineNum);
  while ((s = freadln(stdin))){
    t = getToks(s); // break the line into tokens
    fundex = lookup(t[0]); // Is first token a shell literal
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {
       // replace this statement to call a function that runs executables
      fprintf(stdout, "This shell only supports built-ins. Replace this to run programs as commands.\n");
    }
    // change this to print the current working directory before each line as well as the line number.
    fprintf(stdout, "%d: ", lineNum);
  }
  return 0;
}
