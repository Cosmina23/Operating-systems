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
#define MAX_LENGTH 100

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
void read_bmp_reg(char* filepath_in, char* filename, uint32_t w_h[2], struct stat *data_file, int bmp){
  char result[MAX_R];

  int filedes_in = open(filepath_in, O_RDONLY);
  sprintf(result,"open %s in read_bmp_reg", filepath_in);
  Error(result, filedes_in);
  
  int return_value = 0; //use to check if it's an error
  if(bmp){
    return_value = lseek(filedes_in, SEEK_BYTES, SEEK_CUR);//set the currsor
    Error("lseek in 'read_bmp_reg'", return_value);
    
    return_value = read(filedes_in, &w_h[0], sizeof(w_h[0]));
    Error("read width 'read_bmp_reg'", return_value);
    
    return_value = read(filedes_in, &w_h[1], sizeof(w_h[1]));
    Error("read height in 'read_bmp_reg'", return_value);
  }
  
  return_value = stat(filepath_in, data_file);
  Error("stat in 'read_bmp_reg'", return_value);

  return_value = close(filedes_in);
  sprintf(result, "close %s in read_bmp_reg", filepath_in);
  Error(result, return_value);
}


//function to write bmp data
int write_bmp_reg(char* dirpath_out,char* filename, uint32_t w_h[2], struct stat *data_file, int bmp){
  int nr_lines = 0;
  int return_value = 0; //used to check 'close' 
  char result[MAX_R];
  char *rights = drepturi(data_file);
  char file_name[MAX_LENGTH];
  strcpy(file_name, filename);


  //get the file name without extension
  char out_file[MAX_LENGTH];
  strcpy(out_file, strtok(filename,"."));
  strcat(out_file, output);

  //make the relative path from the current directory for output file
  char aux[MAX_LENGTH];
  strcpy(aux, dirpath_out);
  strcat(aux,"/");
  strcat(aux, out_file);
  strcpy(out_file, aux);

  //open the output file
  int filedes_out = open(out_file ,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  sprintf(result, "open %s in write_bmp_reg", out_file);
  Error(result, filedes_out);

  //the time of the last modification of the content
  //if the name or data had to be modified: st->ctime
  struct tm *dt = gmtime(&data_file->st_mtime);
  if (dt == NULL) {
    perror("Error in gmtime function");
    exit(-1);
  }

  //PRINT FILE NAME--------------------------------
  sprintf(result, "nume fisier: %s\n", file_name);
  writeOutput(filedes_out, result, "Error write: nume in write_bmp_reg");
  nr_lines++;

  //if the input file is bmp file write height and width
  if(bmp){
    sprintf(result, "inaltime: %u\n", w_h[0]);
    writeOutput(filedes_out, result, "Error write: width in write_bmp_reg");
    nr_lines++;
    
    sprintf(result, "lungime: %u\n", w_h[1]);
    writeOutput(filedes_out, result, "Error write: height in write_bmp_reg");
    nr_lines++;
  }
  
  sprintf(result, "dimensiune: %lu\n", data_file->st_size);
  writeOutput(filedes_out, result, "Error write: size in write_bmp_reg");
  nr_lines++;

  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "Error write: uid in write_bmp_reg");
  nr_lines++;

  sprintf(result, "timpul ultimei modificari: %d.%d.%d\n", dt->tm_mday + 1, dt->tm_mon + 1, dt->tm_year + 1900);
  writeOutput(filedes_out, result, "Error write: time in write_bmp_reg");
  nr_lines++;

  sprintf(result, "contorul de legaturi: %ld\n", data_file->st_nlink);
  writeOutput(filedes_out, result, "Error write: nr links in write_bmp_reg");
  nr_lines++;

  writeOutput(filedes_out, rights, "Error write: rigths in write_bmp_reg");
  nr_lines += 3;

  return_value = close(filedes_out);
  sprintf(result, "close %s in write_bmp_reg", out_file);
  Error(result, return_value);

  return nr_lines;
}


int write_link(char* dirpath_out, char* filename_link, char* filePath_input, struct stat *data_file){
  int nr_lines = 0;
  int return_value = 0;
  char result[MAX_R];
  struct stat target_info;
  char filename[MAX_LENGTH];
  strcpy(filename, filename_link);

  //get the file name without extension
  char out_file[MAX_LENGTH];
  strcpy(out_file, strtok(filename,"."));
  strcat(out_file, output);

  //make the relative path from the current directory for output file
  char aux[MAX_LENGTH];
  strcpy(aux, dirpath_out);
  strcat(aux,"/");
  strcat(aux, out_file);
  strcpy(out_file, aux);

  return_value = stat(filePath_input,&target_info);
  sprintf(result, "stat for %s in write_link", filename_link);
  Error(result, return_value);
  
  if(S_ISREG(target_info.st_mode)){
    
    int filedes_out = open(out_file ,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    sprintf(result, "open %s in write_link", out_file);
    Error(result, filedes_out);
    
    return_value = lstat(filePath_input, data_file);
    sprintf(result, "lstat for %s in write_link", filename_link);
    Error(result, return_value);
    
    //check if the target is a regular file
    sprintf(result, "nume legatura: %s\n", filename_link);
    writeOutput(filedes_out, result, "Error write: nume in write_link");
    nr_lines++;
    
    sprintf(result, "dimensiune: %ld\n", data_file->st_size);
    writeOutput(filedes_out, result, "Error write: dimensiune in write_link");
    nr_lines++;
    
    sprintf(result, "dimensiune fisier: %lu\n", target_info.st_size);
    writeOutput(filedes_out, result, "Error write: dimensiune fisier in write_link");
    nr_lines++;
    
    char* rez = drepturi(data_file);
    return_value = write(filedes_out, rez, strlen(rez));
    sprintf(result, "write rights for %s in write_link", filename_link);
    Error(result, return_value);
    nr_lines += 3;
    
    free(rez);
    
    return_value = close(filedes_out);
    sprintf(result,"close %s in write_link", filename_link);
    Error(result, return_value);
  }
  return nr_lines;
}


int write_directory(char* dirpath_out, char *filename_in, char* filepath, struct stat *data_file){
  int return_value = 0;
  char result[MAX_R];
  int nr_lines = 0;

  //get the file name without extension
  char out_file[MAX_LENGTH];
  strcpy(out_file, strtok(filename_in,"."));
  strcat(out_file, output);

  //make the relative path from the current directory for output file
  char aux[MAX_LENGTH];
  strcpy(aux, dirpath_out);
  strcat(aux,"/");
  strcat(aux, out_file);
  strcpy(out_file, aux);

  return_value = lstat(filepath, data_file);

  //open the output file
  int filedes_out = open(out_file ,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
  sprintf(result, "open %s in write_directory", out_file);
  Error(result, filedes_out);

  
  sprintf(result, "nume director: %s\n", filename_in);
  writeOutput(filedes_out, result, "Error write: nume director in write_diretory");
  nr_lines += 1;
  
  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "Error write: uid in write_diretory");
  nr_lines += 1;
  
  char* rez = drepturi(data_file);
  return_value = write(filedes_out, rez, strlen(rez));
  sprintf(result, "write drepturi for %s in write_directory", filename_in);
  Error(result, return_value);
  nr_lines += 3;
  
  free(rez);
  
  return_value = close(filedes_out);
  sprintf(result,"close %s in write_directory", filename_in);
  Error(result, return_value);

  return nr_lines;
}

   
//BitCount = nr de bits pe pixel si e pe 2 bytes, setem cursorul la pozitia 28 din file si citim BitCount, dupa care mutam cursorul la pozitia 54 in functie de inceputul fisierului 
void convertToGray(char* filename, char* filepath){
  //citim primii 54 de bytes din fisier, pentru ca se cere sa se citeasca tot, altfel puteam direct cu lseek;
  int return_value = 0;
  char result[MAX_LENGTH];
  
  int filedes_in = open(filepath, O_RDWR);
  sprintf(result, "open %s in convertToGray", filepath);
  Error(result, filedes_in);

  printf("%s\n", filepath);
  
  uint32_t width = 0 , height = 0;
  unsigned char pixel[3];
  
  return_value = lseek(filedes_in, 28, SEEK_SET);//set the currsor
  Error("lseek in 'read_bmp_reg'", return_value);

  uint16_t BitCount;
  return_value = read(filedes_in, &BitCount, sizeof(BitCount));
  Error("Bit count read", return_value);
  printf("%u\n", BitCount);
  //aici facem un switch sa vedem cati bits per pixel avem, daca BitCount = 8 avem 256 si asa mai departe
  
  
  return_value = lseek(filedes_in, SEEK_BYTES, SEEK_SET);//set the currsor la pozitia 18
  Error("lseek in 'read_bmp_reg'", return_value);

  //citire width si height pentru imagine,deoarece inca nu a fost salvata valoarea in h_w[2], aprocesul 2 pt imaginea bmp se face inaintea procesului 1
  //puteam sa si salvez h si w in w_h[2] si sa nu mai citesc dupa
  return_value = read(filedes_in, &width, sizeof(width));
  Error("read width 'read_bmp_reg'", return_value);
    
  return_value = read(filedes_in, &height, sizeof(height));
  Error("read height in 'read_bmp_reg'", return_value);
  lseek(filedes_in, 54, SEEK_SET);

   
  printf("%u %u\n", width, height);
  
  int size = width*height;
  
  
  //mai jos ar fi in C folosind functii de biblioteca. Noi facem cu functii sistem, definim un uint8_t sau cat ar trebui pentru fiecare, citim fiecare pixel si dupa il rescriem dupa formatare
  
  
  //mai jos ar fi in C folosind functii de biblioteca. Noi facem cu functii sistem, definim un uint8_t sau cat ar trebui pentru fiecare, citim fiecare pixel si dupa il rescriem dupa formatare
  if(BitCount > 8) {//color table present
    
    for(int i = 0; i < size; ++i){
      return_value = read(filedes_in, pixel, 3);
      Error("read from bmp %d", return_value);
      unsigned char gri = (0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
      lseek(filedes_in, -3, SEEK_CUR);
      pixel[0]=pixel[1]=pixel[2] =gri;
      return_value = write(filedes_in, pixel, 3);
      Error("write in bmp %d", return_value);
    }
  }
  if(BitCount <= 8){
    //1 byte=r , 1 byte = g, 1 byte = b
    //avem un tabel de 1024 bytes
    uint8_t padding;
    for(int i = 0; i < 256; ++i){
      return_value = read(filedes_in, pixel, sizeof(pixel));
      Error("read pixel 8x8", return_value);
      return_value = lseek(filedes_in, -3, SEEK_CUR);
      Error("lseek in 8x8", return_value);
      unsigned char gri = (0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
      pixel[0] = pixel[1] = pixel[2] = gri;
      return_value = write(filedes_in, pixel, 3);
      Error("write pixel in 8x8", return_value);
      return_value = read(filedes_in, &padding, sizeof(padding));
      Error("read padding", return_value);
    }
  }
  
  close(filedes_in);
}



void gray(char* filepath, char* filename){
  int return_value = 0;
  char result[MAX_LENGTH];
  
  int filedes_in = open(filepath, O_RDWR);
  sprintf(result, "open %s in convertToGray", filepath);
  Error(result, filedes_in);
  
  printf("%s\n", filepath);
  
  uint32_t width = 0 , height = 0;
  unsigned char pixel[3];
  return_value = lseek(filedes_in, 28, SEEK_SET);//set the currsor
  Error("lseek in 'read_bmp_reg'", return_value);

  uint16_t BitCount;
  return_value = read(filedes_in, &BitCount, sizeof(BitCount));
  Error("Bit count read", return_value);
  printf("%u\n", BitCount);
  
  //citire width si height pentru imagine,deoarece inca nu a fost salvata valoarea in h_w[2], aprocesul 2 pt imaginea bmp se face inaintea procesului 1
  //puteam sa si salvez h si w in w_h[2] si sa nu mai citesc dupa
  return_value = lseek(filedes_in, SEEK_BYTES, SEEK_SET);
  return_value = read(filedes_in, &width, sizeof(width));
  Error("read width 'read_bmp_reg'", return_value);
  
  //  return_value = read(filedes_in, &height, sizeof(height));
  Error("read height in 'read_bmp_reg'", return_value);
  
  printf("%u %u\n", width, height);
  
  lseek(filedes_in, 54, SEEK_SET);
  
  //mai jos ar fi in C folosind functii de biblioteca. Noi facem cu functii sistem, definim un uint8_t sau cat ar trebui pentru fiecare, citim fiecare pixel si dupa il rescriem dupa formatare
  //if(bitDepth <= 8) //color table present
  
  if(BitCount > 8){ 
    int size = width*height;
    
    for(int i = 0; i < size; ++i){
      return_value = read(filedes_in, pixel, 3);
      Error("read from bmp %d", return_value);
      unsigned char gri = (0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
      lseek(filedes_in, -3, SEEK_CUR);
      pixel[0]=pixel[1]=pixel[2] =gri;
      return_value = write(filedes_in, pixel, 3);
      Error("write in bmp %d", return_value);
    }
    
    close(filedes_in);
  }
}
  
  
void processDirectory(char *dirpath_in, char *dirpath_out) {
  int return_value = 0;
  uint32_t w_h[2];    //4 bytes width, 4 bytes height
  struct stat data_file;
  DIR *dir = opendir(dirpath_in); // open directory
  struct dirent *entry;
  char entry_name[MAX_LENGTH];
  int nr_lines = 0;
  char aux[100];
  
  if (dir == NULL) { // check if the directory exists
    perror("Error opening directory");
    exit(-1);
  }
  
  
  //read file one by one from the directory
  while((entry = readdir(dir)) != NULL){
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      strcpy(entry_name, entry->d_name);
      char filePath[PATH_MAX];//make the absolute path for file
      //example: file ex1.c from directory SO:
      strcpy(filePath, dirpath_in);//add 'SO' => 'SO'
      strcat(filePath, "/");//add '/' => 'SO/'
      strcat(filePath, entry->d_name); //add file name 'ex1.c' => 'SO/ex1.c'
      strcpy(aux, filePath);
      
      struct stat file_type;
      return_value = lstat(filePath, &file_type);
      Error("lstat in main", return_value);
      
      pid_t pid = fork();
      pid_t pid2;
      pid_t w ;
      int wstatus;
      
      if(pid == -1){//error code
	perror("Error in fork");
	exit(EXIT_FAILURE);
      }

      
      if(pid == 0){//child 1  code
        //printf("C1: %d p-> %d\n", getpid(), getppid());
	if(S_ISREG(file_type.st_mode)){
	  //function to write data for bmp file or a regular file
	  read_bmp_reg(filePath, entry->d_name, w_h, &data_file, checkFileExtension(entry_name));
	  strcpy(entry_name, entry->d_name);//copy entry->d_name in entry_name because entry_name will be modificate after checkFileExtension function
	  nr_lines = write_bmp_reg(dirpath_out,entry->d_name, w_h, &data_file, checkFileExtension(entry_name));
	  strcpy(entry_name, entry->d_name);
	}
	if(S_ISDIR(file_type.st_mode)){
	  nr_lines = write_directory(dirpath_out, entry_name, filePath, &data_file);
	}
	if(S_ISLNK(file_type.st_mode)){
	  nr_lines = write_link(dirpath_out, entry_name, filePath, &data_file);
	}
	exit(nr_lines);
      }
      
      if(pid > 0){ //parent code
	
	if(checkFileExtension(entry_name)){
	  strcpy(entry_name, entry->d_name);
	  pid2 = fork();
	  
	  if(pid2 == -1){
	    perror("Error fork 2");
	    exit(EXIT_FAILURE);
	  }
	  
	  if(pid2 == 0){ //child 2 code
	    //printf("C2: %d p-> %d\n",getpid(), getppid());
	     convertToGray(entry_name, aux);
	    exit(0);
	  }
	  
	}

	
	if(checkFileExtension(entry_name)){//se putea si cu pid2>0, dar ar fi ramas pid2 > 0 de la un copil2 atunci ramane acelasi si intra in wait(pid2), dar nu eexista pid2 => afiseaza "waited fpr second child"
	  // așteaptă al doilea copil
	  strcpy(entry_name, entry->d_name);
	  w = waitpid(pid2, &wstatus, WUNTRACED | WCONTINUED);
	  if (w == -1) {
	    perror("waitpid for second child");
	    exit(EXIT_FAILURE);
	  }
	  if (WIFEXITED(wstatus)) {
	    printf("S-a incheiat procesul cu PID2-ul %i si codul %d\n",pid2, WEXITSTATUS(wstatus));
	  } else if (WIFSIGNALED(wstatus)) {
	    printf("second child killed by signal %d\n", WTERMSIG(wstatus));
	  } else if (WIFSTOPPED(wstatus)) {
	    printf("second child stopped by signal %d\n", WSTOPSIG(wstatus));
	  } else if (WIFCONTINUED(wstatus)) {
	    printf("second child continued\n");
	  }
	}
	w = waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
	if (w == -1) {
	  perror("waitpid");
	  exit(EXIT_FAILURE);
	}
	if (WIFEXITED(wstatus)) {
	  printf("S-a incheiat procesul cu PID-ul %i si codul %d\n",pid, WEXITSTATUS(wstatus));
	} else if (WIFSIGNALED(wstatus)) {
	  printf("killed by signal %d\n", WTERMSIG(wstatus));
	} else if (WIFSTOPPED(wstatus)) {
	  printf("stopped by signal %d\n", WSTOPSIG(wstatus));
	} else if (WIFCONTINUED(wstatus)) {
	  printf("continued\n");
	}
	
      }	
    }
  }
  
  return_value = closedir(dir);
  Error("close input directory in main", return_value);
  
  exit(EXIT_SUCCESS);//folosit pentru a incheia procesul parinte cu un cod de iesire corespunzator
}


int main(int argc, char* argv[]){
  checkArguments(argc, argv);
  
  char* dirpath_in = argv[1];
  char* dirpath_out = argv[2];
  
  processDirectory(dirpath_in, dirpath_out);
  
  return 0;
}
