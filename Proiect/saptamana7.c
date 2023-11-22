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

#define MAX_R 100 //maxim size for result
#define SIZE_D 256 //maxim size, used in function 'drepturi'
#define NAME_SIZE 60 //mazim size for name, used in 'fileInfo' structure 

//structure to save file information
typedef struct fileInfo {
  char filename[NAME_SIZE];
  uint32_t width;
  uint32_t height;
  struct stat data;
} fileInfo;

//function to check number of arguments
void checkArguments(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage %s <director_intrare>\n", argv[0]);
    exit(-1);
  }
}

//function to check if it's a bmp file
void checkFileExtension(char* filename) {
  char* file_extension = strrchr(filename, '.');
  if (file_extension != NULL) {
    file_extension++;
    if (strcmp(file_extension, "bmp") != 0) {
      perror("Input file is not a bmp file");
      exit(-1);
    }
  }
}

//check if it's a regular file
void checkRegularFile(const char* filename) {
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

//function to check and print if it's an error
void Error(const char* function, int ret) {
  if (ret == -1) {
    printf("Error in %s function\n", function);
    exit(-1);
  }
}

//function to extract the access rights
char* drepturi(mode_t mode) {
  char* sir = malloc(SIZE_D);
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
  
  sprintf(sir + strlen(sir), "\n");
  return sir;
}


//function to write the output in a file using file descriptor
void writeOutput(int file_des, char* output, const char* error_message) {
  int return_value = 0;
  
  return_value = write(file_des, output, strlen(output));
  Error(error_message, return_value);
}

//write infos in output file if the input file is a regular or a bmp file
//argument bmp is 1 when is a bmp file, 0 if is a regular file
void writeFileInfo(int output_file, fileInfo* fileInfo, int bmp) {
  char result[MAX_R] = "";
  
  sprintf(result, "nume fisier: %s\n", fileInfo->filename);
  writeOutput(output_file, result, "Error write: nume");

  //if is a bmp file write width and height
  if(bmp){
    sprintf(result, "inaltime: %u\n", fileInfo->width);
    writeOutput(output_file, result, "Error write: width");
    
    sprintf(result, "lungime: %u\n", fileInfo->height);
    writeOutput(output_file, result, "Error write: height");
  }
  
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
  
  int return_value = 0;
  char *rez = drepturi(fileInfo->data.st_mode);
  return_value = write(output_file, rez, strlen(rez));
  Error("write rights", return_value);
  
  free(rez);
}

//function for write infos for LINK
void writeLinkInfo(int output_file, fileInfo* fileInfo) {
  int return_value = 0;
  char result[MAX_R] = "";
  
  sprintf(result, "nume legatura: %s\n", fileInfo->filename);
  writeOutput(output_file, result, "Error write: nume");
  
  sprintf(result, "dimensiune legatura: %ld\n", fileInfo->data.st_size);
  writeOutput(output_file, result, "Error write: dimensiune legatura");
  
  struct stat file_info;
  stat(fileInfo->filename,&file_info);
  
  sprintf(result, "dimensiune fisier target: %ld\n", file_info.st_size);
  writeOutput(output_file, result, "Error write: dimensiune legatura");
  
  
  char* rez = drepturi(fileInfo->data.st_mode);
  return_value = write(output_file, rez, strlen(rez));
  Error("write rights", return_value);
  
  free(rez);
}

//function for write infos about Directory
void writeDirectoryInfo(int output_file, fileInfo* fileInfo) {
  int return_value = 0;
  char result[MAX_R] = "";
  
  sprintf(result, "nume director: %s\n", fileInfo->filename);
  writeOutput(output_file, result, "Error write: nume director");
  
  sprintf(result, "identificatorul utilizatorului: %u\n", fileInfo->data.st_uid);
  writeOutput(output_file, result, "Error write: uid");
  
  char* rez = drepturi(fileInfo->data.st_mode);
  return_value = write(output_file, rez, strlen(rez));
  Error("write rights", return_value);
  
  free(rez);
}


void processFile(int output_file, const char* filePath, char *filename) {
  fileInfo data;
  strcpy(data.filename, filePath);
  int bmp = 0;
  
  char* file_extension = strrchr(filename, '.');
  if (file_extension != NULL) {
    file_extension++;
    if (strcmp(file_extension, "bmp") == 0) {
      bmp = 1;
    }
  }
  
  int file_descriptor = open(filePath, O_RDONLY);
  if (file_descriptor == -1) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }
  
  if (bmp) {
    int return_value = lseek(file_descriptor, 18, SEEK_CUR);
    Error("lseek", return_value);
    
    return_value = read(file_descriptor, &(data.width), sizeof(data.width));
    Error("read(width)", return_value);
    
    return_value = read(file_descriptor, &(data.height), sizeof(data.height));
    Error("read(height)", return_value);
  }
  
  close(file_descriptor);
  
  int return_value = lstat(data.filename, &(data.data));
  Error("stat", return_value);
  
  if (S_ISREG(data.data.st_mode)) {
    writeFileInfo(output_file, &data, bmp);
  } else if (S_ISLNK(data.data.st_mode)) {
    writeLinkInfo(output_file, &data);
  } else if (S_ISDIR(data.data.st_mode)) {
    writeDirectoryInfo(output_file, &data);
  }
}


//function for going through the directory
void processDirectory(const char* dirPath, int output_file) {
  DIR* dir = opendir(dirPath);//open directory
  struct dirent* entry;

  if (dir == NULL) {//check if the directory exists
    perror("Error opening directory");
    exit(-1);
  }

  //read file one by one from the directory
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      char filePath[PATH_MAX];//make the absolute path for file
      //example: file ex1.c from directory SO: 
      strcpy(filePath, dirPath);//add 'SO' => 'SO'
      strcat(filePath, "/");//add '/' => 'SO/'
      strcat(filePath, entry->d_name); //add file name 'ex1.c' => 'SO/ex1.c'
      processFile(output_file, filePath, entry->d_name);
      
    }
  }
  
  closedir(dir);
}

int main(int argc, char* argv[]) {
  
  checkArguments(argc, argv);
  
  char* dirPath = argv[1];
  struct stat st;
  if(stat(dirPath, &st) == -1){
    perror("Error in stat");
    exit(EXIT_FAILURE);
  }
  
  // Open output file
  int output_file = open("statistica.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  Error("open output", output_file);
  
  if (S_ISDIR(st.st_mode)) {
    processDirectory(dirPath, output_file);
  } else {
    printf("%s is not a directory\n", dirPath);
    exit(-1);
  }
  
  int return_value = close(output_file);
  Error("close output", return_value);
  
  return 0;
}
