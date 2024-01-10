#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/utils.h"

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
char *output_file_folder = "output/final_submission/";

void redirection(char **dup_list, int size, char* root_dir){
    // Determine the filename based on root_dir
    char *filename = malloc(sizeof(char)*300);
    filename[0] = '\0';

    strcat(filename, "./");
    strcat(filename, output_file_folder);
    strcat(filename, extract_filename(root_dir));
    strcat(filename, ".txt");

    // Redirect standard output to output file (output/final_submission/root*.txt)
    int temp = dup(STDOUT_FILENO);
    int output = open(filename, WRITE, PERM);
    if(dup2(output, STDOUT_FILENO) == -1){
        perror("Failed to redirect output\n");
        exit(-1);
    }

    close(output);

    // Read the content each symbolic link in dup_list
    // Write the path, as well as the content of symbolic link, to output file
    char symLinkBuf[200];
    for (int i = 0; i < size; i++){
        memset(symLinkBuf, 0, sizeof(symLinkBuf));
        if(readlink(dup_list[i], symLinkBuf, 200) == -1){
            perror("Failure Reading Symbolic Link in redirection()");
            exit(-1);
        }
        fflush(stdout);
        printf("[<path of symbolic link> --> <path of retained file>] : [%s --> %s]\n", dup_list[i], symLinkBuf);
        fflush(stdout);
    }

    if(dup2(temp, STDOUT_FILENO) == -1){
        perror("Failed to restore output\n");
        exit(-1);
    }

    close(temp);
}

void create_symlinks(char **dup_list, char **retain_list, int size) {
    // Create symbolic link at the location of deleted duplicate file
    for(int i = 0; i < size; i++){
        if(symlink(retain_list[i], dup_list[i]) == -1){
            perror("Failure Creating Symlink in create_symlinks()");\
            exit(-1);
        }
    }
}

void delete_duplicate_files(char **dup_list, int size) {
    // Delete duplicate files
    for(int i = 0; i < size; i++){
        if(remove(dup_list[i]) == -1){
            perror("Failure Deleting Duplicate File in delete_duplicate_files()");
            exit(-1);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        // dir is the root_directories directory to start with
        // e.g. ./root_directories/root1
        printf("Usage: ./root <dir> \n");
        return 1;
    }

    // Fork the first non_leaf process associated with root directory
    char* root_directory = argv[1];
    char all_filepath_hashvalue[4098]; //buffer for gathering all data transferred from child process
    memset(all_filepath_hashvalue, 0, sizeof(all_filepath_hashvalue)); // clean the buffer

    // Construct pipe
    int fds[2];
    pipe(fds);

    // fork() child process & read data from pipe to all_filepath_hashvalue
    char *writeEnd = malloc(40);
    pid_t pid = fork();
    if (pid == 0){
        close(fds[0]);
        sprintf(writeEnd, "%d", fds[1]);
        char *args[] = {"./nonleaf_process", root_directory, writeEnd, NULL};
        int test = execvp(args[0], args);
        // Check for failed root exec
        if (test == -1){
            printf("Error, exec failed at root process\n");
            return -1;
        }
    }

    close(fds[1]);
    waitpid(pid, NULL, 0);
    read(fds[0], all_filepath_hashvalue, 4098);
    close(fds[0]);


    // Malloc dup_list and retain list & use parse_hash() in utils.c to parse all_filepath_hashvalue
    //     dup_list: list of paths of duplicate files. We need to delete the files and create symbolic links at the location
    //     retain_list: list of paths of unique files. We will create symbolic links for those files
    char **dup_list = malloc(sizeof(char*)*32);
    char **retain_list = malloc(sizeof(char*)*32);
    int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list);

    // Implement the functions
    delete_duplicate_files(dup_list,size);
    create_symlinks(dup_list, retain_list, size);
    redirection(dup_list, size, root_directory);

    // Free any arrays that are allocated using malloc!!
    free(writeEnd);
    free(dup_list);
    free(retain_list);
    dup_list = NULL;
    retain_list = NULL;
}
