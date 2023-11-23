#include<stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include<stdlib.h>
#include <sys/wait.h>

int main(void){
  pid_t pid;
  char litera;
  int status;

  pid = fork();
  
  if(pid == -1){//eroare
    perror("Error in fork");
    exit(EXIT_FAILURE);
  }

  
  if(pid == 0){ //cod copil
    execl("./studenti", "studenti", "2", NULL);
    perror("Eroare la execl");
    exit(EXIT_FAILURE);
    
  }

  else{ //cod parinte
    while(1){
      scanf(" %c", &litera);
      usleep(500000);
      
      if (litera == '0') {
	break;
      }
      
      int w = waitpid(pid, &status, WUNTRACED | WCONTINUED);

      if(w == -1){
	perror("Error in waitpid");
	exit(EXIT_FAILURE);
      }
      
      if (WIFEXITED(w)) {
	printf("exited, status=%d\n", WEXITSTATUS(w));
      } else if (WIFSIGNALED(w)) {
	printf("killed by signal %d\n", WTERMSIG(w));
      } else if (WIFSTOPPED(w)) {
	printf("stopped by signal %d\n", WSTOPSIG(w));
      } else if (WIFCONTINUED(w)) {
	printf("continued\n");
      }
    }
  }
  
  return 0;
}
