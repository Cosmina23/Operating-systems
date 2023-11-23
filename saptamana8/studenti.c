#include<stdio.h>
#include<stdlib.h>

int main(void){
  int g = 2;
  switch(g){
  case 1:
    {
      printf("Ana\n");
      printf("Ioana\n");
      printf("Maria\n");
      printf("Iulia\n");
      break;
    }
  case 2:
    {
      printf("Marian\n");
      printf("Iasmin\n");
      printf("Cristian\n");
      break;
    }
  default:
    break;
  }
  return 0;
} 
