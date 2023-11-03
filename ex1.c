#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char *argv[]){

  if(argc != 4){
    perror("Error command arg");
    exit(-1);
  }

  char *in = argv[1];
  char *out = argv[2];
  char c = *argv[3];
  char aux;
  int l = 0, L = 0, nr = 0, t = 0;
  int count_ch = 0;

  int fd_in = open(in, O_RDONLY);
  if(fd_in == -1){
    perror("Error opening input file");
    exit(-1);
  }

  while(read(fd_in,&aux,1) > 0){
    //printf("%c\n",aux);
    t++;

    if(isdigit(aux))
      nr++;
    if(islower(aux))
      l++;
    if(isupper(aux))
      L++;

    if (aux == c) {
      count_ch++;
    }
  }
  //printf("%d\n",L);

  if(close(fd_in) == -1){
    perror("Error for close file");
  }

  int fd_out = open(out, O_WRONLY);
  if (fd_out == -1) {
    perror("Error opening output file");
    exit(1);
  }

  char output_text[200];
  int s = 0;
  
  struct stat st;
  if (stat(in, &st) == 0) {//succes
    s = st.st_size;
  } else {
    perror("Error for size");
  }
  
  sprintf(output_text, "Numar liere mici %d\n", l);
  write(fd_out, output_text, strlen(output_text));

  sprintf(output_text, "Numar litere mari %d\n", L);
  write(fd_out, output_text, strlen(output_text));
  
  sprintf(output_text, "Numar cifre %d\n", nr);
  write(fd_out, output_text, strlen(output_text));
  
  sprintf(output_text, "Numar aparitii caracter '%c' %d\n", c,count_ch);
  write(fd_out, output_text, strlen(output_text));
  
  sprintf(output_text, "Dimensiune fisier %d\n", s);
  write(fd_out, output_text, strlen(output_text));
  
  
  if(close(fd_out) == -1){
    perror("Error for close file");
  }
    
  return 0;
}
