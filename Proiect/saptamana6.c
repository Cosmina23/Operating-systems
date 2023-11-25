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

#define NR_A 2 //number of arguments
#define SIZE_D 256 //for access rights char
#define SEEK_BYTES 18 //number of bytes to be skipped
#define MAX_R 4096 //maximum length for result

char *output = "statistica.txt"; //output file name

// Function for verification number of arguments
void checkArguments(int argc, char* argv[]) {
  if (argc != NR_A) {
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

//extract data from input file
void read_bmp(char* filename, uint32_t w_h[2], struct stat *data_file){
  int filedes_in = open(filename, O_RDONLY);
  Error("Open input file ", filedes_in); //check if the bmp file is open

  int return_value = 0; //use to check if it's an error

  return_value = lseek(filedes_in, SEEK_BYTES, SEEK_CUR);//set the currsor
  Error("leek", return_value);

  return_value = read(filedes_in, &w_h[0], sizeof(w_h[0]));
  Error("read width", return_value);

  return_value = read(filedes_in, &w_h[1], sizeof(w_h[1]));
  Error("read height", return_value);

  return_value = stat(filename, data_file);
  Error("stat", return_value);

  return_value = close(filedes_in);
  Error("close input file", return_value);
}

//function for writing the data in the output file
void writeOutput(int file_des, char *output, const char* error_message){
  int return_value = 0;
  
  return_value = write(file_des, output, strlen(output));
  if(return_value == -1){
    Error(error_message, return_value);
  }
}


//function to extract access rights
char *drepturi(struct stat *data_file){  
  char *sir = malloc(SIZE_D);
  mode_t mode = data_file->st_mode;
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


//function to write data in 'statistica.txt'
void write_bmp(char* filename, uint32_t w_h[2], struct stat *data_file){
  int return_value = 0;
  char result[MAX_R];
  char *rights = drepturi(data_file);

  //open output_file
  int filedes_out = open(output, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  Error("open output", filedes_out);

  //the time of the last modification of the content
  //if the name or data had to be modified: st->ctime
  struct tm *dt = gmtime(&data_file->st_mtime);
  if (dt == NULL) {
    perror("Error in gmtime function");
    exit(-1);
  }

  sprintf(result, "nume fisier: %s\n", filename);
  writeOutput(filedes_out, result, "Error write: nume");

  sprintf(result, "inaltime: %u\n", w_h[0]);
  writeOutput(filedes_out, result, "Error write: width");

  sprintf(result, "lungime: %u\n", w_h[1]);
  writeOutput(filedes_out, result, "Error write: height");

  sprintf(result, "dimensiune: %lu\n", data_file->st_size);
  writeOutput(filedes_out, result, "Error write: size");

  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "Error write: uid");

  sprintf(result, "timpul ultimei modificari: %d.%d.%d\n", dt->tm_mday + 1, dt->tm_mon + 1, dt->tm_year + 1900);
  writeOutput(filedes_out, result, "Error write: time");

  sprintf(result, "contorul de legaturi: %ld\n", data_file->st_nlink);
  writeOutput(filedes_out, result, "Error write: nr links");

  writeOutput(filedes_out, rights, "Error write: rigths");
  
  //close output file
  return_value = close(filedes_out);
  Error("close output", return_value);
}

int main(int argc, char* argv[]){

  checkArguments(argc, argv); //check number of arguments

  char *filename = argv[1]; //input file name
  uint32_t w_h[2]; //width = w_h[0]  and  height = w_h[1]
  struct stat data_info;
  
  checkRegularFile(filename);
  checkFileExtension(filename);

  read_bmp(filename, w_h, &data_info);
  write_bmp(filename, w_h, &data_info);
  
  return 0;
}
