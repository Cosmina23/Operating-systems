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

#define MAX_R 100
#define SIZE_D 256
#define NAME_SIZE 60

typedef struct fileInfo {
    char filename[NAME_SIZE];
    uint32_t width;
    uint32_t height;
    struct stat data;
} fileInfo;

void checkArguments(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage %s <director_intrare>\n", argv[0]);
        exit(-1);
    }
}

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

void Error(const char* function, int ret) {
    if (ret == -1) {
        printf("Error in %s function\n", function);
        exit(-1);
    }
}

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


void writeOutput(int file_des, char* output, const char* error_message) {
    int return_value = 0;

    return_value = write(file_des, output, strlen(output));
    if (return_value == -1) {
        Error(error_message, return_value);
    }
}


void writeFileInfo(int output_file, fileInfo* fileInfo, int bmp) {
    char result[MAX_R] = "";

    sprintf(result, "nume fisier: %s\n", fileInfo->filename);
    writeOutput(output_file, result, "Error write: nume");

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

void readDataFromFile(int input_file, struct fileInfo* fileInfo, int bmp) {
    int return_value = 0;

    if(bmp){
      return_value = lseek(input_file, 18, SEEK_CUR);
      Error("lseek", return_value);
      
      return_value = read(input_file, &(fileInfo->width), sizeof(fileInfo->width));
      Error("read(width) ", return_value);
      
      return_value = read(input_file, &(fileInfo->height), sizeof(fileInfo->height));
      Error("read(height)", return_value);
    }
    
    return_value = lstat(fileInfo->filename, &(fileInfo->data));
    Error("stat", return_value);
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
    
    readDataFromFile(open(filePath, O_RDONLY), &data, bmp);
    

    if (S_ISREG(data.data.st_mode)) {
      writeFileInfo(output_file, &data, bmp);
    } else if (S_ISLNK(data.data.st_mode)) {
        writeLinkInfo(output_file, &data);
    } else if (S_ISDIR(data.data.st_mode)) {
        writeDirectoryInfo(output_file, &data);
    }
}

void processDirectory(const char* dirPath, int output_file) {
    DIR* dir = opendir(dirPath);
    struct dirent* entry;

    if (dir == NULL) {
        perror("Error opening directory");
        exit(-1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char filePath[PATH_MAX];
            strcpy(filePath, dirPath);
            strcat(filePath, "/");
            strcat(filePath, entry->d_name);
            processFile(output_file, filePath, entry->d_name);
	    
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    checkArguments(argc, argv);

    char* dirPath = argv[1];
    struct stat st;
    stat(dirPath, &st);

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
