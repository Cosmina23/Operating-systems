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


#define MAX_R 100 //maximum length for result , in function 'result_char' and 'main'
#define SIZE_D 256 //for the char in which the access rights are saved
#define NAME_SIZE 60

//structura pentru a salva informatiile citire din fisier
typedef struct fileInfo{
  char filename[NAME_SIZE];
  uint32_t width;
  uint32_t height;
  struct stat data;
}fileInfo;

// Function for verification number of arguments
void checkArguments(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage %s <fisier_intrare>\n", argv[0]);
    exit(-1);
  }
}

// Function for verification file extension
void checkFileExtension(char* filename) {
  char* file_extension = strrchr(filename, '.');
  if (file_extension != NULL) {
    file_extension++;
    if (strcmp(file_extension, "bmp") != 0) {
      perror("Input file is not a bmp file");
      exit(-1);
    }
  } else{
    perror("Invalid file extension");
    exit(-1);
  }
}

// Function for checking if it's a regular file
void checkRegularFile(char* filename) {
  struct stat info;
  if (stat(filename, &info) == -1) {
    perror("Error in stat function");
    exit(-1);
  }
  
  if (!S_ISREG(info.st_mode)) {
    printf("%s is not a regular file\n", filename);
    exit(-1);
  }
}


//functie pentru testarea valorii returnate de functiile sistem folosite in program, functioneaza pentru functiile care returneaza -1 in caz de eroare
void Error(const char *function, int ret){
  if(ret == -1){
    printf("Error in %s function\n", function);
    exit(-1);
  }
}


//function to extract access rights
char *drepturi(mode_t mode){  
  char *sir = malloc(SIZE_D);
  if (sir == NULL) {
    perror("Error allocating memory");
    exit(-1); 
  }
  
  sprintf(sir, "drept de acces user: %c%c%c\n",
	  (mode & S_IRUSR) ? 'R' : '-',
	  (mode & S_IWUSR) ? 'W' : '-',
	  (mode & S_IXUSR) ? 'X' : '-');
  
  sprintf(sir + strlen(sir), "drept de acces grup: %c%c%c\n",
	  (mode & S_IRGRP) ? 'R' : '-',
	  (mode & S_IWGRP) ? 'W' : '-',
	  (mode & S_IXGRP) ? 'X' : '-');
  
  sprintf(sir + strlen(sir), "drept de acces altii: %c%c%c\n",
	  (mode & S_IROTH) ? 'R' : '-',
	  (mode & S_IWOTH) ? 'W' : '-',
	  (mode & S_IXOTH) ? 'X' : '-');
  
  return sir;
}


//function for writing the data in the output file
void writeOutput(int file_des, char *output, const char* error_message){
  int return_value = 0;
  
  return_value = write(file_des, output, strlen(output));
  if(return_value == -1){
    Error(error_message, return_value);
  }
  
}



void readDataFromFile(int input_file, struct fileInfo *fileInfo) {
  int return_value = 0;

  return_value = lseek(input_file, 18, SEEK_CUR);
  Error("lseek", return_value);

  return_value = read(input_file, &(fileInfo->width), sizeof(fileInfo->width));
  Error("read(width) ", return_value);

  return_value = read(input_file, &(fileInfo->height), sizeof(fileInfo->height));
  Error("read(height)", return_value);

  return_value = stat(fileInfo->filename, &(fileInfo->data));
  Error("stat", return_value);
}


void writeDataToFile(int output_file, fileInfo *fileInfo) {
  int return_value = 0;
  char result[MAX_R] = "";

  sprintf(result, "nume fisier: %s\n", fileInfo->filename);
  writeOutput(output_file, result, "Error write: nume");

  sprintf(result, "inaltime: %u\n", fileInfo->width);
  writeOutput(output_file, result, "Error write: width");

  sprintf(result, "lungime: %u\n", fileInfo->height);
  writeOutput(output_file, result, "Error write: height");

  sprintf(result, "dimensiune: %lu\n", fileInfo->data.st_size);
  writeOutput(output_file, result, "Error write: size");

  sprintf(result, "identificatorul utilizatorului: %u\n", fileInfo->data.st_uid);
  writeOutput(output_file, result, "Error write: uid");

  struct tm dt;
  dt = *(gmtime(&(fileInfo->data.st_ctime)));
  sprintf(result, "timpul ultimei modificari: %d.%d.%d\n", dt.tm_mday + 1, dt.tm_mon + 1, dt.tm_year + 1900);
  writeOutput(output_file, result, "Error write: time");

  sprintf(result, "contorul de legaturi: %ld\n", fileInfo->data.st_nlink);
  writeOutput(output_file, result, "Error write: nr links");

  char *rez = drepturi(fileInfo->data.st_mode);
  return_value = write(output_file, rez, strlen(rez));
  Error("write rights", return_value);

  free(rez);
}

int main(int argc, char *argv[]){
  checkArguments(argc, argv);//check number of arguments
  
  char* filepath = argv[1];
  
  checkRegularFile(filepath);//check if it's a regular file
  checkFileExtension(filepath);//check if it's a bmp file

  //Open input file
  int input_file = open(filepath, O_RDONLY);
  Error("open input ", input_file);

  fileInfo data;
  strcpy(data.filename, filepath);

  readDataFromFile(input_file, &data);

   int output_file;
  (output_file = open("statistica2.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR));
  Error("open output", output_file);

  writeDataToFile(output_file, &data);

  int return_value = close(input_file);
  Error("close input", return_value);

  return_value = close(output_file);
  Error("close output", return_value);

    
  return 0;
}
