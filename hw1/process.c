#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>

/**
 * Executes the process p.
 * If the shell is in interactive mode and the process is a foreground process,
 * then p should take control of the terminal.
 */
void launch_process(process *p)
{
  SET_SIGNALS(SIG_DFL);
  dup2( p->stdin ,STDIN_FILENO);
  dup2( p->stdout ,STDOUT_FILENO );
  exec ( p->argv[0] ,p->argv);
  perror ("exec");
  exit(EXIT_FAILURE);

   
}

/**
 * Put a process in the foreground. This function assumes that the shell
 * is in interactive mode. If the cont argument is true, send the process
 * group a SIGCONT signal to wake it up.
 */
void
put_process_in_foreground (process *p, int cont)
{
   if(tcsetpgrp(shell_terminal,p->pid) <0)
    printf("Failed to set process group terminal" );
    
   if(resume) {
      if( !p->completed ) 
      p->stopped = false;
     
    if( kill (- p->pid, SIGCONT ) <0 )
     perror( "kill (SIGCONT)");
   }
    wait_for_process( p );
    
     if(tcsetpgrp(shell_terminal, shell_pgid) < 0)
    printf("Failed to set process group terminal\n" );
     if(tcsettr(shell_terminal,&p->tmodes) <0)
    printf("Failed to retrieve terminal attributes" );
     if( tcsettr(shell_terminal,TCSADRAIN, &shell_tmodes) <0)
    printf("Failed to set attributes" );
    
}  

/**
 * Put a process in the background. If the cont argument is true, send
 * the process group a SIGCONT signal to wake it up. 
 */
void
put_process_in_background (process *p, int cont)
{
   if(resume) {
      if( !p->completed ) 
      p->stopped = false;
     
    if( kill (- p->pid, SIGCONT ) <0 )
     perror( "kill (SIGCONT)");
   }

}
