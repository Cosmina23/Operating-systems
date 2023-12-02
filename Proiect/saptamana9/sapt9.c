#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
  C1   -----pipe1------->   C2   ------pipe2----->  P
*/

int main(void){
  pid_t pid1, pid2;
  int pfd1[2], pfd2[2];

  if(pipe(pfd1) < 0){
    printf("Err pfd1");
    exit(1);
  }

  if(pipe(pfd2) < 0){
    printf("Err pfd2");
    exit(1);
  }
  

  if((pid1=fork()) == -1){
    perror("Err pid1");
    exit(-1);
  }

  if(pid1 == 0){
    //cod copil1

    close(pfd2[0]);
    close(pfd2[1]);

    close(pfd1[0]);//inchide capat de citire

    dup2(pfd1[1], 1);
    
    execlp("cat", "cat", "test.txt", NULL);
    perror("Execlp 1");
    exit(1);
  }

  if((pid2=fork()) == -1){
    perror("Err pid2");
    exit(-1);
  }

  if(pid2 == 0){
    //copil2

    close(pfd1[1]);//inchide capat de scriere pentru pip1
    close(pfd2[0]);//inchide capat de citire pentru pip2

    dup2(pfd1[0], 0);//citim de pe pipe1
    dup2(pfd2[1], 1);//iesirea standard sa fie pipe2
    
    execlp("bash", "bash", "sapt9.sh","c", NULL);
    perror("Execlp 2");
    exit(0);
  }

  //cod parinte
  close(pfd1[0]);
  close(pfd1[1]);
  close(pfd2[1]);

  //dup2(pfd2[0], 0);//intrarea standard e pipe2

  /*
  FILE* stream = fdopen(pfd2[0], "r");
  
  int c;
  fscanf(stream, "%d", &c);
  printf("%d\n", c);
  */

  int c;
  char buffer[32]; // Asumming the result from bash won't exceed 32 characters
  int bytesRead = read(pfd2[0], buffer, sizeof(buffer) - 1);
  
  if (bytesRead > 0) {
    buffer[bytesRead] = '\0'; // Null-terminate the string
    sscanf(buffer, "%d", &c); // Convert string to integer
    printf("%d\n", c);
  } else {
    printf("Error reading from pipe\n");
  }
  
  close(pfd2[0]);
  
  waitpid(pid1, NULL, 0);
  waitpid(pid2, NULL, 0);

  return 0;
}
