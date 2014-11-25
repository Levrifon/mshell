/* ------------------------------
   $Id: pipe.c,v 1.2 2005/03/29 09:46:52 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "jobs.h"
#include "pipe.h"
#include "cmd.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
	int fds[2];
	int pid;
	char * bob;
	assert(pipe(fds) != -1);
	switch(pid = fork()) {
		case -1 : 
			assert(0);
		case 0	:
			/*commande n*/
			setpgid(0,0);
			dup2(fds[0],STDIN_FILENO);
			close(fds[0]);
			close(fds[1]);
			execvp(cmds[nbcmd-1][0], cmds[nbcmd-1]);
			assert(0);
	}
	switch(fork()) {
		case -1:
			assert(0);
		case 0 :
			/*commande 0*/
			setpgid(0,pid);
			/* on redirige la sortie vers la sortie standard */
			dup2(fds[1],STDOUT_FILENO);
			close(fds[0]);
			close(fds[1]);
			execvp(cmds[0][0],cmds[0]);
			assert(0);
	}
	bob = strcat(cmds[0][0], "|"); 
	bob = strcat(bob, cmds[1][0]);
	close(fds[0]);
	close(fds[1]);
	if(bg == 1) {
		jobs_addjob(pid,BG,bob); 
	} else {
		jobs_addjob(pid,FG,bob); 
		waitfg(pid);
	}
	
	return;
}
