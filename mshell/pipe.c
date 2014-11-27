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
	int fds[MAXCMDS][2];
	int pid;
	int i = nbcmd -1, j;
	char * bob;
	assert(pipe(fds[0]) != -1);
	
	switch(pid = fork()) {
		case -1:
			printf("erreur, mon cerveau explose\n");
			assert(0);
		case 0 :
			/*commande 0*/
			setpgid(0,0);
			/* on redirige la sortie vers la sortie standard */
			dup2(fds[0][1],STDOUT_FILENO);
			close(fds[0][1]);
			close(fds[0][0]);
			execvp(cmds[0][0],cmds[0]);
			assert(0);
	}
	
	for(i = 1; i<nbcmd- 1; i++){
		assert(pipe(fds[i]) != -1);
		
		switch(fork()){
			case -1:
				printf("erreur, mon cerveau explose\n");
				assert(0);
		case 0 :
			/*commande 0*/
			setpgid(0,pid);
			/* on redirige la sortie vers la sortie standard */
			dup2(fds[i][1],STDOUT_FILENO);
			dup2(fds[i-1][0],STDIN_FILENO);
			for(j=0; j<i; j++){
				close(fds[j][0]);
				close(fds[j][1]);
			}
			execvp(cmds[i][0],cmds[i]);
			assert(0);
		}
	}
	
	switch(fork()){
		case -1:
			printf("erreur, mon cerveau explose\n");
			assert(0);
		case 0 :
			/*commande n*/
			setpgid(0,pid);
			dup2(fds[i-1][0],STDIN_FILENO);
			for(j=0; j<i; j++){
				close(fds[j][0]);
				close(fds[j][1]);
			}
			
			execvp(cmds[i][0],cmds[i]);
			assert(0);
	}
	
	bob = strcat(cmds[0][0], "|"); 
	bob = strcat(bob, cmds[1][0]);
	for(j=0; j<i; j++){
		close(fds[j][0]);
		close(fds[j][1]);
	}
	if(bg == 1) {
		jobs_addjob(pid,BG,bob); 
	} else {
		jobs_addjob(pid,FG,bob); 
		waitfg(pid);
	}
	
	return;
}
