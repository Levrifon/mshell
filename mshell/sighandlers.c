/* ------------------------------
   $Id: sighandlers.c,v 1.1 2005/03/17 13:00:46 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

#include "jobs.h"
#include "common.h"
#include "sighandlers.h"


/*
 * Signal - wrapper for the sigaction function
 */
int
signal_wrapper(int signum, handler_t *handler) 
{

  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  assert(sigaction(signum,&sa,NULL) != -1);
  return 1;
}


/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void sigchld_handler(int sig) {
  pid_t pid;
  int status;
	struct job_t *job;
  if (verbose)
    printf("sigchld_handler: entering\n");
    
  while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) >0) {
  
    if(WIFEXITED(status))  {
 			if (verbose)
      	printf("Job terminé  pid : %d\n",pid);
      job = jobs_getjobpid(pid);
      jobs_deletejob(pid);
    } else if(WIFSIGNALED(status)) {
   		if (verbose)
    		printf("Job tué  pid : %d\n",pid);
			job = jobs_getjobpid(pid);
			jobs_deletejob(pid);
    } else if(WIFSTOPPED(status)) {	/* if ^Z */
    	if(verbose)
				printf("Job stoppé pid : %d\n",pid); 
     	job = jobs_getjobpid(pid);
     	job->jb_state = ST;
    }
  }
    
  if (verbose)
    printf("sigchld_handler: exiting\n");

  return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void
sigint_handler(int sig) 
{
  pid_t pid;
  if (verbose)
  printf("sigint_handler: entering\n");

  pid = jobs_fgpid();
  printf("%d pid",pid);
  if(pid != 0) {
    kill(pid,sig);
  }
    
  if (verbose)
    printf("sigint_handler: exiting\n");
    
  return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void
sigtstp_handler(int sig) 
{
  pid_t pid;
  if (verbose)
    printf("sigtstp_handler: entering\n");

  pid = jobs_fgpid();
  if(pid != 0) {
    kill(pid,sig);
  }
    
  if (verbose)
    printf("sigtstp_handler: exiting\n");
    
  return;
}
