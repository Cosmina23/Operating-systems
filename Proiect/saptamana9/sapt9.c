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
C1 ----pfd1---> C2 ---pfd2--->
*/

#define NR_A 4 //number of arguments
#define SIZE_D 256 //for access rights char
#define SEEK_BYTES 18 //number of bytes to be skipped
#define MAX_R 4096 //maximum length for result
#define MAX_LENGTH 100

char *output = "_statistica.txt"; //output file name
char* script = "sapt9.sh";

// Function for verification number of arguments
void checkArguments(int argc, char* argv[]) {
  if (argc != NR_A) {
    printf("Usage %s <director_intrare> <director_iesrie> <c> \n", argv[0]);
    exit(-1);
  }
}


//functie pentru testarea valorii returnate de functiile sistem folosite in program, functioneaza pentru functiile care returneaza -1 in caz de eroare
//function -> functia de biblioteca/sistem utilizata
//local_function -> numele functiei definite de noi in program, unde am utilizat functia sistem/de biblioteca
//filename -> numele fisierului pentru care s-a utilizat functia 'function'
//return_value -> valoarea returnata de 'function'
void Error(char* function, char* local_function, char* filename, int return_value){
  char message[MAX_LENGTH];
  sprintf(message, "Error in %s->%s for %s\n",function, local_function, filename);
  if(return_value == -1){
    perror(message);
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
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
void writeOutput(int file_des, char *output_char, char* function, char* local_function, char* filename){
  int return_value = 0;
  
  return_value = write(file_des, output_char, strlen(output_char));
  if(return_value == -1){
    Error(function, local_function, filename, return_value);
  }
}

//extract data from input file
void read_bmp_reg(char* filepath_in, char* filename, uint32_t w_h[2], struct stat *data_file, int bmp){

  int filedes_in = open(filepath_in, O_RDONLY);
  Error("open", "read_bmp_reg", filename, filedes_in);
  
  int return_value = 0; //use to check if it's an error
  if(bmp){
    return_value = lseek(filedes_in, SEEK_BYTES, SEEK_CUR);//set the currsor
    Error("lseek", "read_bmp_reg", filename, return_value);
    
    return_value = read(filedes_in, &w_h[0], sizeof(w_h[0]));
    Error("read width", "read_bmp_reg", filename, return_value);
    
    return_value = read(filedes_in, &w_h[1], sizeof(w_h[1]));
    Error("read hight", "read_bmp_reg", filename, return_value);
  }
  
  return_value = stat(filepath_in, data_file);
  Error("stat", "read_bmp_reg", filename, return_value);

  return_value = close(filedes_in);
  Error("close", "read_bmp_reg", filename, return_value);
}


//function to write bmp data
int write_bmp_reg(char* dirpath_out,char* filename, uint32_t w_h[2], struct stat *data_file, int bmp){
  int nr_lines = 0;
  char result[MAX_LENGTH];
  int return_value = 0; //used to check 'close'
  char *rights = drepturi(data_file);
  char file_name[MAX_LENGTH/2];
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
  Error("open", "write_bmp_reg", filename, filedes_out);

  //the time of the last modification of the content
  //if the name or data had to be modified: st->ctime
  struct tm *dt = gmtime(&data_file->st_mtime);
  if (dt == NULL) {
    perror("Error in gmtime function");
    exit(-1);
  }

  //PRINT FILE NAME--------------------------------
  sprintf(result, "nume fisier: %s\n", file_name);
  writeOutput(filedes_out, result, " write: nume", "write_bmp_reg", filename);
  nr_lines++;

  //if the input file is bmp file write height and width
  if(bmp){
    sprintf(result, "inaltime: %u\n", w_h[0]);
    writeOutput(filedes_out, result, "write: width", "write_bmp_reg", filename);
    nr_lines++;
    
    sprintf(result, "lungime: %u\n", w_h[1]);
    writeOutput(filedes_out, result, "write: height", "write_bmp_reg", filename);
    nr_lines++;
  }
  
  sprintf(result, "dimensiune: %lu\n", data_file->st_size);
  writeOutput(filedes_out, result, "write: size", "write_bmp_reg", filename);
  nr_lines++;

  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "write: uid", "write_bmp_reg", filename);
  nr_lines++;

  sprintf(result, "timpul ultimei modificari: %d.%d.%d\n", dt->tm_mday + 1, dt->tm_mon + 1, dt->tm_year + 1900);
  writeOutput(filedes_out, result, "write: time", "write_bmp_reg", filename);
  nr_lines++;

  sprintf(result, "contorul de legaturi: %ld\n", data_file->st_nlink);
  writeOutput(filedes_out, result, "Error write: nr links", "write_bmp_reg", filename);
  nr_lines++;

  writeOutput(filedes_out, rights, "Error write: rigths", "write_bmp_reg", filename);
  nr_lines += 3;

  return_value = close(filedes_out);
  Error("close", "write_bmp_reg", filename, return_value);

  return nr_lines;
}


int read_write_link(char* dirpath_out, char* filename_link, char* filePath_input, struct stat *data_file){
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
  Error("stat","write_link", filename_link , return_value);
  
  if(S_ISREG(target_info.st_mode)){
    
    int filedes_out = open(out_file ,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    Error("open", "write_link", filename_link, filedes_out);
    
    return_value = lstat(filePath_input, data_file);
    Error("lstat", "write_link", filename_link, return_value);
    
    //check if the target is a regular file
    sprintf(result, "nume legatura: %s\n", filename_link);
    writeOutput(filedes_out, result, "write: nume", "write_link", filename_link);
    nr_lines++;
    
    sprintf(result, "dimensiune: %ld\n", data_file->st_size);
    writeOutput(filedes_out, result, "write: dimensiune", "write_link", filename_link);
    nr_lines++;
    
    sprintf(result, "dimensiune fisier: %lu\n", target_info.st_size);
    writeOutput(filedes_out, result, "write: dimensiune fisier", "write_link", filename_link);
    nr_lines++;
    
    char* rez = drepturi(data_file);
    return_value = write(filedes_out, rez, strlen(rez));
    Error("write drepturi", "write_link", filename_link, return_value);
    nr_lines += 3;
    
    free(rez);
    
    return_value = close(filedes_out);
    Error("close", "write_link", filename_link, return_value);
  }
  return nr_lines;
}


int read_write_directory(char* dirpath_out, char *filename_in, char* filepath, struct stat *data_file){
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
  Error("open", "write_directory", filename_in, filedes_out);

  
  sprintf(result, "nume director: %s\n", filename_in);
  writeOutput(filedes_out, result, "write: nume director", "write_diretory", filename_in);
  nr_lines += 1;
  
  sprintf(result, "identificatorul utilizatorului: %u\n", data_file->st_uid);
  writeOutput(filedes_out, result, "write: uid", "write_diretory", filename_in);
  nr_lines += 1;
  
  char* rez = drepturi(data_file);
  return_value = write(filedes_out, rez, strlen(rez));
  Error("write: drepturi", "write_directory", filename_in, return_value);
  nr_lines += 3;
  
  free(rez);
  
  return_value = close(filedes_out);
  Error("close", "write_diretory", filename_in, return_value);

  return nr_lines;
}


void convertToGray(char* filepath, char* filename){
  int return_value = 0;
  int filedes_in; //file descriptor for input file
  uint32_t width, height;//width and height of the image, 4 bytes each
  uint8_t red, green, blue;
  uint8_t gray;
  uint16_t BitCount; //number of Bits per Pixel, 2 bytes
  int NumColors;
  int size; //size of the image (size = width* height)

  filedes_in = open(filepath, O_RDWR);
  Error("open", "convertToGray", filename, filedes_in);

  //setare cursorul la pozitia 18 pentru a putea citi width si height
  return_value = lseek(filedes_in, 18, SEEK_CUR);
  Error("lseek 18", "convertToGray", filename, return_value);//verificare daca lseek=1

  return_value = read(filedes_in, &width, sizeof(width));//citire width
  Error("read: width", "convertToGray", filename, return_value);//verificare valoare returnata de read

  return_value = read(filedes_in, &height, sizeof(height));//citire height
  Error("read: height", "convertToGray",filename, return_value);//verificare valoare returnata de read
  
  //setare cursor pentru a citi BitCount 
  return_value = lseek(filedes_in, 2, SEEK_CUR);
  Error("lseek 2", "convertToGray", filename, return_value);//verificare valoare returnata de lseek

  return_value = read(filedes_in, &BitCount, sizeof(BitCount));
  Error("read: BitCount", "convertToGray", filename, return_value);

  //setare cursor la pozitia pixelilor(byte-ul 54), 24 bytes fata de pozitia curenta
  return_value = lseek(filedes_in, 24, SEEK_CUR);
  Error("lseek 24", "convertToGray", filename, return_value);

  //verificare daca numarul de bits per pixel <= 8
  if(BitCount <= 8){//daca BitCount = 8 se verifica valoarea exacta pentru a seta Numarul de Culori
    switch(BitCount){
    case 1:
      NumColors = 1;
      break;
    case 4:
      NumColors = 16;
      break;
    case 8:
      NumColors = 256;
      break;
    }
    size = NumColors; //size = NumColors * 4, dar citim cate 4 bytes din tabel,deci avem 256 de citiri din tabel, 256 ori parcurgere de for
  }

  if(BitCount > 8){//daca BitCount > 8 setam sizeul pentru imagine
    size = width*height;
  }
   
  for(int i = 0; i < size; ++i){//conversie din rgb in grayscale pentru fiecare bit
    return_value = read(filedes_in, &red, sizeof(red));//citire valoare red
    Error("read red ", "convertToGray", filename, return_value);

    return_value = read(filedes_in, &green, sizeof(green));//citire valoare green
    Error("read green ", "convertToGray", filename, return_value);

    return_value = read(filedes_in, &blue, sizeof(blue));//citire valoare blue
    Error("read blue ", "convertToGray", filename, return_value);
    
    return_value = lseek(filedes_in, -3, SEEK_CUR);//setare cursor pentru a suprascrie valorile pentru red,green,blue
    Error("lseek ", "convertToGray", filename, return_value);
    
    gray = (0.299 * red + 0.587 * green + 0.114 * blue);

    //suprascriere valori red,green,blue cu valoarea gray
    return_value = write(filedes_in, &gray, sizeof(gray));
    Error("write red ", "convertToGray", filename, return_value);

    return_value = write(filedes_in, &gray, sizeof(gray));
    Error("write red ", "convertToGray", filename, return_value);

    return_value = write(filedes_in, &gray, sizeof(gray));
    Error("write red ", "convertToGray", filename, return_value);

    //daca BitCount <= 8, trebuie sa sarim peste byte-ul de padding din tabel
    if(BitCount <= 8){
      uint8_t padding; //utilizat pentru a sari byteul nefolosit in tabel( = 0)
      return_value = read(filedes_in, &padding, sizeof(padding));
      Error("read padding", "convertToGray", filename, return_value);
    }
  }
  
  //inchidere fisier de intrare
  return_value = close(filedes_in);
  Error("close", "convertToGray", filename, return_value);
} 

  
void processDirectory(char *dirpath_in, char *dirpath_out, char c_in) {
  int return_value = 0;
  uint32_t w_h[2];    //4 bytes width, 4 bytes height
  struct stat data_file;
  struct dirent *entry;
  char entry_name[MAX_LENGTH];
  int nr_lines = 0;
  char aux[MAX_LENGTH];//used to copy path for input file
  int sum=0;
  int pfd[2];

  if(pipe(pfd) < 0){
    perror("Error pid");
    exit(1);
  }

  DIR *dir = opendir(dirpath_in); // open directory
  
  if(dir == NULL) { // check if the directory exists
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
      Error("lstat", "processDirectory", dirpath_in, return_value);
      
      pid_t pid1, pid2;
      pid_t w ;
      int wstatus;
      int pfd1[2], pfd2[2];

      if(pipe(pfd1) == -1){
	perror("Pipe 1");
	exit(1);
      }

      if(pipe(pfd2) == -1){
	perror("Pipe 2");
	exit(1);
      }
      
      
      if((pid1 = fork()) == -1){//error code
	perror("Error in fork");
	exit(EXIT_FAILURE);
      }
      
      
      else if(pid1 == 0){//child 1

	if(S_ISREG(file_type.st_mode)){
	  //function to write data for bmp file or a regular file
	  read_bmp_reg(filePath, entry->d_name, w_h, &data_file, checkFileExtension(entry_name));
	  strcpy(entry_name, entry->d_name); //copy entry->d_name in entry_name because entry_name will be modificate after checkFileExtension function
	  nr_lines = write_bmp_reg(dirpath_out,entry_name, w_h, &data_file, checkFileExtension(entry_name));
	  strcpy(entry_name, entry->d_name);
	  
	  if(checkFileExtension(entry_name) == 0){//daca e fisier obisnuit , dar nu bmp trimite continutul fisierului la al doilea copil
	    strcpy(entry_name, entry->d_name);
	    
	    return_value = close(pfd2[0]);
	    Error("close pfd2[0] for child1", "processDirectory", dirpath_in, return_value);

	    return_value = close(pfd2[1]);
	    Error("close pfd2[1] for child1", "processDirectory", dirpath_in, return_value);

	    
	    return_value = close(pfd1[0]);
	    Error("close pfd1[0] for child1", "processDirectory", dirpath_in, return_value);

	    
	    return_value = dup2(pfd1[1], 1);
	    Error("close dup2 for child1", "processDirectory", dirpath_in, return_value);

	    
	    execlp("cat", "cat", aux, NULL);
	    perror("Execlp 1");
	    exit(EXIT_FAILURE);
	  }
	}

	if(S_ISDIR(file_type.st_mode)){//if input file is directory read and write data in statistica.txt
	  nr_lines = read_write_directory(dirpath_out, entry_name, filePath, &data_file);
	}

	if(S_ISLNK(file_type.st_mode)){//if input file is Link read and write data in statistica.txt
	  nr_lines = read_write_link(dirpath_out, entry_name, filePath, &data_file);
	}

	exit(nr_lines);//send nr_lines from child to parent process
      }
      
      //child2 code
      if(S_ISREG(file_type.st_mode)){
	if((pid2=fork()) == -1){//make child 2
	  perror("Error fork 2");
	  exit(EXIT_FAILURE);
	}
	
	else if(pid2 == 0){ //child 2 code
	  if(checkFileExtension(entry_name)){//if the file have bmp extension convert img to grayscale
	    strcpy(entry_name, entry->d_name);
	    convertToGray(aux, entry_name);
	    exit(0);
	  }
	  else{//if the file is regular, but don't have 'bmp' extension
	    return_value = close(pfd1[1]);
	    Error("close pfd1[1] for child2", "processDirectory", dirpath_in, return_value);

	    return_value = close(pfd2[0]);
	    Error("close pfd2[0] for child2", "processDirectory", dirpath_in, return_value);

	    return_value = dup2(pfd1[0], 0);//citim de pe pipe1
	    Error("dup2 pfd1[0] for parent", "processDirectory", dirpath_in, return_value);
      
	    return_value = dup2(pfd2[1], 1);//iesirea standard sa fie pipe2
	    Error("dup2 pfd2[1] for parent", "processDirectory", dirpath_in, return_value);
      
	    execlp("bash", "bash", script, &c_in, NULL);
	    perror("Execlp 2");
	    exit(EXIT_FAILURE);
	  }//else checkFileExtension
	}//else if(pid2==0)
      }//if(S_ISREG)

      
      //cod proces parinte
      
      return_value = close(pfd1[0]);
      Error("close pfd1[0] for parent", "processDirectory", dirpath_in, return_value);

      return_value = close(pfd1[1]);
      Error("close pfd1[1] for parent", "processDirectory", dirpath_in, return_value);

      return_value = close(pfd2[1]);
      Error("close pfd2[1] for parent", "processDirectory", dirpath_in, return_value);
      
      return_value = dup2(pfd2[0], 0);//redirectare intrare,datele o sa fie luate din pipe2=  pdf2[0]
      Error(" dup2 pfd2 for parent", "processDirectory", dirpath_in, return_value);
      
      
      FILE* pipe = fdopen(pfd2[0], "r");

      if(pipe == NULL){
	perror("Error for open pfd2[0] for parent");
	exit(EXIT_FAILURE);
      }
      
      int c;
      while (fscanf(pipe, "%d", &c) != EOF) {
	sum += c;
      }

      //inchide capatul utilizat
      return_value = close(pfd2[0]);
      Error("close pfd2[0] for parent", "processDirectory", dirpath_in, return_value);

      if(S_ISREG(file_type.st_mode)){//se putea si cu pid2>0, dar ar fi ramas pid2 > 0 de la un copil2 atunci ramane acelasi si intra in wait(pid2), dar nu exista pid2 => afiseaza "waited fpr second child"
	w = waitpid(pid2, &wstatus, WUNTRACED | WCONTINUED);//waitpid for child2
	if (w == -1) {
	  perror("waitpid for second child");
	  exit(EXIT_FAILURE);
	}
	if (WIFEXITED(wstatus)) {
	  printf("S-a incheiat procesul cu PID-ul %i si codul %d\n",pid2, WEXITSTATUS(wstatus));
	} else if (WIFSIGNALED(wstatus)) {
	  printf("second child killed by signal %d\n", WTERMSIG(wstatus));
	} else if (WIFSTOPPED(wstatus)) {
	  printf("second child stopped by signal %d\n", WSTOPSIG(wstatus));
	} else if (WIFCONTINUED(wstatus)) {
	  printf("second child continued\n");
	}
      }
      
      
      w = waitpid(pid1, &wstatus, WUNTRACED | WCONTINUED);//waitpid for child1(all file)
      if (w == -1) {
	perror("waitpid");
	exit(EXIT_FAILURE);
	}
      if (WIFEXITED(wstatus)) {
	printf("S-a incheiat procesul cu PID-ul %i si codul %d\n",pid1, WEXITSTATUS(wstatus));
      } else if (WIFSIGNALED(wstatus)) {
	printf("killed by signal %d\n", WTERMSIG(wstatus));
      } else if (WIFSTOPPED(wstatus)) {
	printf("stopped by signal %d\n", WSTOPSIG(wstatus));
      } else if (WIFCONTINUED(wstatus)) {
	printf("continued\n");
      }
    }//if(strcmp)
  }//while()
  
  printf("S-au identificat in total %d propozitii corecte care contin caracterul %c\n",sum, c_in);
  
  return_value = closedir(dir);
  Error("close input directory", "main", dirpath_in, return_value);

  
  exit(EXIT_SUCCESS);//folosit pentru a incheia procesul parinte cu un cod de iesire corespunzator, daca s-a ajusn la final inseamna ca procesul a avut suces
}


int main(int argc, char* argv[]){
  checkArguments(argc, argv);
    
  char* dirpath_in = argv[1];
  char* dirpath_out = argv[2];
  char c = *argv[3];
  
  processDirectory(dirpath_in, dirpath_out, c);
  
  return 0;
}
