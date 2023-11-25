#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(void){
  int pfd[2];
  pid_t pid;

  //creare pipe
  if(pipe(pfd) == -1){
    perror("Error in pipe");
    exit(EXIT_FAILURE);
  }

  //creare copil
  pid = fork();

  //verificare eroare creare copil
  if(pid == -1){
    perror("Error fork");
    exit(EXIT_FAILURE);
  }

  //functionalitate parinte
  if(pid > 0){
    close(pfd[0]);

    char c;
    printf("Introdu caracter: ");
    scanf("%c", &c);

    write(pfd[1], &c, 1);
    
    close(pfd[1]);

    int status;
    if(wait(&status) == -1){
      perror("Err");
      exit(-1);
    }
  }
  
  //functionalitate
  if(pid == 0){
    close(pfd[1]);
    char c;
    
    // Citește caracter
    read(pfd[0], &c, sizeof(char));
    printf("Caracterul primit este: %c\n", c);
    close(pfd[0]);
  }
  
  
  return 0;
}
