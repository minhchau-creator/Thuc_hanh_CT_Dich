#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
 
int main(int argc, char *argv[]) {
    char file1_path[256];
    char file2_path[256];
    
    if (argc < 2) {
        strcpy(file1_path, "example1");
        strcpy(file2_path, "./tests/example1");
    } else {
        strcpy(file1_path, argv[1]);
        snprintf(file2_path, sizeof(file2_path), "./tests/%s", argv[1]);
    }
    
    printf("Comparing: %s vs %s\n", file1_path, file2_path);
    
    FILE *file1 = fopen(file1_path, "rb");
    if (file1 == NULL) {
        printf("Error opening file 1: %s\n", file1_path);
        return 1;
    }
 
    FILE *file2 = fopen(file2_path, "rb");
    if (file2 == NULL) {
        printf("Error opening file 2: %s\n", file2_path);
        fclose(file1);
        return 1;
    }

    // Get file sizes
    struct stat stat1, stat2;
    fstat(fileno(file1), &stat1);
    fstat(fileno(file2), &stat2);
    
    long size1 = stat1.st_size;
    long size2 = stat2.st_size;
    
    if (size1 != size2) {
        printf("Files differ in size\n");
        printf("File 1 size: %ld bytes\n", size1);
        printf("File 2 size: %ld bytes\n", size2);
        fclose(file1);
        fclose(file2);
        return 1;
    }
 
    int byte_count = 0;
    while (!feof(file2) && !feof(file1)) {
        unsigned char byte1 = fgetc(file1);
        unsigned char byte2 = fgetc(file2);
        byte_count++;
 
        if (byte1 != byte2) {
            printf("Files differ\n");
            fclose(file1);
            fclose(file2);
            return 1;
        }
    }
 
    printf("Files are identical with %d bytes compared\n", byte_count);
    fclose(file1);
    fclose(file2);
    return 0;
}