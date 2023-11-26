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


#define NR_A 3 //number of arguments
#define SIZE_D 256 //for access rights char
#define SEEK_BYTES 18 //number of bytes to be skipped
#define MAX_R 4096 //maximum length for result

char *output = "_statistica.txt"; //output file name

// Function for verification number of arguments
void checkArguments(int argc, char* argv[]) {
  if (argc != NR_A) {
    printf("Usage %s <director_intrare> <director_iesrie>\n", argv[0]);
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

// Function for verification file extension
int checkFileExtension(char* filename) {
  char* file_extension = strtok(filename, ".");
  file_extension = strtok(NULL,".");
  if (file_extension != NULL) {
    if (strcmp(file_extension, "bmp") == 0) {
      return 1;
    }
  }
  return 0;
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


//function for writing the data in the output file
void writeOutput(int file_des, char *output, const char* error_message){
  int return_value = 0;
  
  return_value = write(file_des, output, strlen(output));
  if(return_value == -1){
    Error(error_message, return_value);
  }
}

//extract data from input file
void read_bmp(char* filepath_in, char* filename, uint32_t w_h[2], struct stat *data_file){
  char result[MAX_R];

  int filedes_in = open(filepath_in, O_RDONLY);
  sprintf(result,"open %s in read_bmp", filepath_in);
  Error(result, filedes_in);
  
  int return_value = 0; //use to check if it's an error
  return_value = lseek(filedes_in, SEEK_BYTES, SEEK_CUR);//set the currsor
  Error("lseek in 'read_bmp'", return_value);
  
  return_value = read(filedes_in, &w_h[0], sizeof(w_h[0]));
  Error("read width 'read_bmp'", return_value);
    
  return_value = read(filedes_in, &w_h[1], sizeof(w_h[1]));
  Error("read height in 'read_bmp'", return_value);
  
  return_value = stat(filepath_in, data_file);
  Error("stat in 'read_bmp'", return_value);

  return_value = close(filedes_in);
  sprintf(result, "close %s in read_bmp", filepath_in);
  Error(result, return_value);
}


//function to write bmp data
void write_bmp(char* dirpath_out,char* filename, uint32_t w_h[2], struct stat *data_file){
  int return_value = 0; //used to check 'close' 
  char result[MAX_R];
  char *rights = drepturi(data_file);
  char file_name[100];
  strcpy(file_name, filename);

  printf("CevA");

  //get the file name without extension
  char out_file[100];
  strcpy(out_file, strtok(filename,"."));
  strcat(out_file, output);

  //make the relative path from the current directory for output file
  char *aux = dirpath_out;
  strcat(aux,"/");
  strcat(aux, out_file);
  strcpy(out_file, aux);

  //open the output file
  int filedes_out = open(out_file ,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  sprintf(result, "open %s in rite_bmp", out_file);
  Error(result, filedes_out);

  //the time of the last modification of the content
  //if the name or data had to be modified: st->ctime
  struct tm *dt = gmtime(&data_file->st_mtime);
  if (dt == NULL) {
    perror("Error in gmtime function");
    exit(-1);
  }

  //PRINT FILE NAME--------------------------------
  printf("File: %s\n", file_name);
  sprintf(result, "nume fisier: %s\n", file_name);
  writeOutput(filedes_out, result, "Error write: nume");

  //if the input file is bmp file write height and width
  sprintf(result, "inaltime: %u\n", w_h[0]);
  writeOutput(filedes_out, result, "Error write: width in write_bmp_reg");
  
  sprintf(result, "lungime: %u\n", w_h[1]);
  writeOutput(filedes_out, result, "Error write: height in write_bmp_reg");
  
  sprintf(result, "dimensiune: %lu\n", data_file->st_size);
  writeOutput(filedes_out, result, "Error write: size in write_bmp_reg");

  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "Error write: uid in write_bmp_reg");

  sprintf(result, "timpul ultimei modificari: %d.%d.%d\n", dt->tm_mday + 1, dt->tm_mon + 1, dt->tm_year + 1900);
  writeOutput(filedes_out, result, "Error write: time in write_bmp_reg");

  sprintf(result, "contorul de legaturi: %ld\n", data_file->st_nlink);
  writeOutput(filedes_out, result, "Error write: nr links in write_bmp_reg");

  writeOutput(filedes_out, rights, "Error write: rigths in write_bmp_reg");

  return_value = close(filedes_out);
  sprintf(result, "close %s in write_bmp", out_file);
  Error(result, return_value);
}


int main(int argc, char* argv[]){
  checkArguments(argc, argv);

  char* dirpath_in = argv[1];
  char* dirpath_out = argv[2];
  int return_value = 0;
  uint32_t w_h[2];
  struct stat data_file;
  DIR* dir = opendir(dirpath_in);//open directory
  struct dirent* entry;

  if (dir == NULL) {//check if the directory exists
    perror("Error opening directory");
    exit(-1);
  }
  
  //read file one by one from the directory
  while((entry = readdir(dir)) != NULL){
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      char filePath[PATH_MAX];//make the absolute path for file
      //example: file ex1.c from directory SO:
      strcpy(filePath, dirpath_in);//add 'SO' => 'SO'
      strcat(filePath, "/");//add '/' => 'SO/'
      strcat(filePath, entry->d_name); //add file name 'ex1.c' => 'SO/ex1.c'
      
      struct stat file_type;
      return_value = lstat(filePath, &file_type);
      Error("lstat in main", return_value);
      
      int pid = fork();
      if(pid == -1){
	perror("Error in fork");
	exit(-1);
      }
      if(pid == 0){//child code
	if(S_ISREG(file_type.st_mode)){
	  printf("%s\n", entry->d_name);
	  if(checkFileExtension(entry->d_name)){
	    printf("b\n");
	    //function to write data for bmp file
	    read_bmp(filePath, entry->d_name, w_h, &data_file);
	    write_bmp(dirpath_out,entry->d_name, w_h, &data_file);
	  }
	  else{
	    printf("r\n");
	    //function to write fregular file
	  }
	}
	if(S_ISDIR(file_type.st_mode)){
	  //function to write directory
	}
	if(S_ISLNK(file_type.st_mode)){
	  //function to write link
	}
	//else;
	exit(0);
      }
      //if bmp mai fa un proces
    }
  }
  while(wait(NULL) > 0);
  
  return_value = closedir(dir);
  Error("close input directory in main", return_value);
  
  return 0;
}
