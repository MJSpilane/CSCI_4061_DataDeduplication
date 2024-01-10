#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/hash.h"
#include "../include/utils.h"

char *output_file_folder = "output/inter_submission/";

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: Inter Submission --> ./leaf_process <file_path> 0\n");
        printf("Usage: Final Submission --> ./leaf_process <file_path> <pipe_write_end>\n");
        return -1;
    }

    // Get <file_path> <pipe_write_end> from argv[]
    char *file_path = argv[1];
    int write_end = atoi(argv[2]);

    // Create the hash of given file
    char hash_value[SHA256_BLOCK_SIZE * 2 + 1];
    struct stat fileStat;
    lstat(file_path, &fileStat); 
    if (S_ISLNK(fileStat.st_mode)){ //Exit if symbolic link is found
        printf("Error, symbolic links already present in input directory\n");
        exit(-1);
    }
    hash_data_block(hash_value, file_path);

    // Construct string write to pipe. The format is "<file_path>|<hash_value>"
    char pipeStr[256];
    pipeStr[0] = '\0';
    snprintf(pipeStr, sizeof(pipeStr), "%s|%s|", file_path, hash_value);

    if(write_end == 0){
        // Extract the file_name from file_path using extract_filename() in utils.c
        char *file_name = extract_filename(file_path);

        // Extract the root directory(e.g. root1 or root2 or root3) from file_path using extract_root_directory() in utils.c
        char *root_dir = extract_root_directory(file_path);

        // Get the location of the new file (e.g. "output/inter_submission/root1" or "output/inter_submission/root2" or "output/inter_submission/root3")
        int len = strlen(output_file_folder) + strlen(root_dir) + strlen(file_name) + 3;
        char *output = (char *)malloc(sizeof(char) * len);
        memset(output, 0, len);

        strcat(output, output_file_folder);
        strcat(output, root_dir);
        strcat(output, file_name);

        // Create and write to file, and then close file
        FILE *fp = fopen(output, "w");
        if (!fp){
            perror("fopen");
            exit(-1);
        }
        if (fp == NULL){
            printf("Failed to create file %s\n", output);
            exit(-1);
        }
        else{
            fwrite(pipeStr, sizeof(char), strlen(pipeStr), fp);
        }
        fclose(fp);

        // Free any arrays that are allocated using malloc!! Free the string returned from extract_root_directory()!! It is allocated using malloc in extract_root_directory()
        free(output);
        free(root_dir);

    }else{
        // Write the string to pipe
        write(write_end, pipeStr, strlen(pipeStr));
        close(write_end);
    }
    exit(0);
}
