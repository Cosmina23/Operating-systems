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
int checkFileExtension(char* filename) {
  char* file_extension = strrchr(filename, '.');
  if (file_extension != NULL) {
    file_extension++;
    if (strcmp(file_extension, "bmp") == 0) {
      return 1;
    }
  }
    
  return 0;
}

// Function for checking if it's a regular file
void checkFile(char* filename) {
  struct stat info;
  if (stat(filename, &info) == -1) {
    perror("Error in stat function");
    exit(-1);
  }
  
  if (!S_ISREG(info.st_mode) && !S_ISLNK(info.st_mode) && !S_ISDIR(info.st_mode)) {
    printf("%s is not a regular file, symbolic link, or directory\n", filename);
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
    perror("Error allocating memoryin 'drepturi' ");
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

  sprintf(sir + strlen(sir), "\n\n");
  return sir;
}

//extract data from input file
void read_bmp_reg(int filedes_in, char* filename, uint32_t w_h[2], struct stat *data_file){
  
  int return_value = 0; //use to check if it's an error

  //if the input file is bmp fileread width and height 
  if(checkFileExtension(filename) == 1){ 
    return_value = lseek(filedes_in, SEEK_BYTES, SEEK_CUR);//set the currsor
    Error("lseek in 'read_bmp_reg'", return_value);
    
    return_value = read(filedes_in, &w_h[0], sizeof(w_h[0]));
    Error("read width 'read_bmp_reg'", return_value);
    
    return_value = read(filedes_in, &w_h[1], sizeof(w_h[1]));
    Error("read height in 'read_bmp_reg'", return_value);
  }

  return_value = stat(filename, data_file);
  Error("stat in 'read_bmp_reg'", return_value);
}


//function to write data in 'statistica.txt'
void write_bmp_reg(char* filename, uint32_t w_h[2], struct stat *data_file, int filedes_out){
  char result[MAX_R];
  char *rights = drepturi(data_file);

  //the time of the last modification of the content
  //if the name or data had to be modified: st->ctime
  struct tm *dt = gmtime(&data_file->st_mtime);
  if (dt == NULL) {
    perror("Error in gmtime function");
    exit(-1);
  }

  sprintf(result, "nume fisier: %s\n", filename);
  writeOutput(filedes_out, result, "Error write: nume");

  //if the input file is bmp file write height and width
  if(checkFileExtension(filename) == 1){
    sprintf(result, "inaltime: %u\n", w_h[0]);
    writeOutput(filedes_out, result, "Error write: width in write_bmp_reg");
    
    sprintf(result, "lungime: %u\n", w_h[1]);
    writeOutput(filedes_out, result, "Error write: height in write_bmp_reg");
  }
  
  sprintf(result, "dimensiune: %lu\n", data_file->st_size);
  writeOutput(filedes_out, result, "Error write: size in write_bmp_reg");

  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "Error write: uid in write_bmp_reg");

  sprintf(result, "timpul ultimei modificari: %d.%d.%d\n", dt->tm_mday + 1, dt->tm_mon + 1, dt->tm_year + 1900);
  writeOutput(filedes_out, result, "Error write: time in write_bmp_reg");

  sprintf(result, "contorul de legaturi: %ld\n", data_file->st_nlink);
  writeOutput(filedes_out, result, "Error write: nr links in write_bmp_reg");

  writeOutput(filedes_out, rights, "Error write: rigths in write_bmp_reg");
}

void write_link(char* filename_link, char* filePath_input, struct stat *data_file, int filedes_out){
  int return_value = 0;
  char result[MAX_R];
  struct stat target_info;
  
  return_value = stat(filePath_input,&target_info);
  Error("stat in write_link", return_value);

  //check if the target is a regular file
  if(S_ISREG(target_info.st_mode)){
    sprintf(result, "nume legatura: %s\n", filename_link);
    writeOutput(filedes_out, result, "Error write: nume in write_link");
    
    sprintf(result, "dimensiune: %ld\n", data_file->st_size);
    writeOutput(filedes_out, result, "Error write: dimensiune in write_link");
    
    
    sprintf(result, "dimensiune fisier: %lu\n", target_info.st_size);
    writeOutput(filedes_out, result, "Error write: dimensiune fisier in write_link");
    
    
    char* rez = drepturi(data_file);
    return_value = write(filedes_out, rez, strlen(rez));
    Error("write rights in write_link", return_value);
    
    free(rez);
  }
}

void write_directory(char *filename_in, struct stat *data_file, int filedes_out){
  int return_value = 0;
  char result[MAX_R];

  
  sprintf(result, "nume director: %s\n", filename_in);
  writeOutput(filedes_out, result, "Error write: nume director in write_diretory");
  
  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "Error write: uid in write_diretory");
  
  char* rez = drepturi(data_file);
  return_value = write(filedes_out, rez, strlen(rez));
  Error("write rights in write_diretory", return_value);
  
  free(rez);
}

//filePath_input - calea catre fisierul de intrare
//filename_input - numele fisierului
void processFile(char *filePath_input, char* filename_input, int filedes_out){
  int return_value = 0;
  struct stat data_file;
  uint32_t w_h[2];
  
  //used lstat because if it's a link will return link info
  return_value = lstat(filePath_input, &data_file);
  Error("lstat in processFile",return_value);

  //read data from file
  int filedes_in = open(filePath_input, O_RDONLY);
  Error("Open input file in processFile", filedes_in); //check if the file is open

  
  if(S_ISREG(data_file.st_mode)){
    read_bmp_reg(filedes_in,filePath_input, w_h, &data_file);
    write_bmp_reg(filename_input, w_h, &data_file, filedes_out);
  } else if(S_ISLNK(data_file.st_mode)){
    write_link(filename_input, filePath_input, &data_file, filedes_out);
  } else if(S_ISDIR(data_file.st_mode)){
    write_directory(filename_input, &data_file, filedes_out);
  }
  

  return_value = close(filedes_in);
  Error("close input in processFile",return_value);
  
}


int main(int argc, char* argv[]){

  checkArguments(argc, argv); //check number of arguments

  char *dir_name = argv[1];
  //char *filename = argv[1]; //input file name
  struct dirent* entry;
  
  
  //deschide director intrare
  DIR* dir = opendir(dir_name);
  if(dir == NULL){
    perror("Error opening directory in main");
    exit(-1);
  }

  int filedes_out = open(output, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  Error("open output in main", filedes_out);
  

  //parcurgere diretor
  while((entry = readdir(dir)) != NULL){
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      char filePath[PATH_MAX];//make the absolute path for file
      //example: file ex1.c from directory SO: 
      strcpy(filePath, dir_name);//add 'SO' => 'SO'
      strcat(filePath, "/");//add '/' => 'SO/'
      strcat(filePath, entry->d_name); //add file name 'ex1.c' => 'SO/ex1.c'
      processFile(filePath, entry->d_name, filedes_out); 
    }
  }
  
  int return_value = close(filedes_out);
  Error("close output in main",return_value);

  //inchide director intrare
  return_value = closedir(dir);
  Error("close input directory in main", return_value);
    
  return 0;
}
