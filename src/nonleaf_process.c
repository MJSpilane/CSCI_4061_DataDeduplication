#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
        return 1;
    }

    // Get <file_path> <pipe_write_end> from argv[]
    const char *file_path = argv[1];
    int write_end = atoi(argv[2]);

    // Initialize a buffer to store data from child processes
    char data[4098];
    memset(data, 0, sizeof(data));
    data[0] = '\0';

    // Open directory
    DIR *dir = opendir(file_path);
    if(dir == NULL){
        perror("Failed to open directory\n");
        exit(-1);
    }

    // Traverse directory and fork child process
    struct dirent *entry;
    char *entryName;
    int readEnd[10]; // Total num files <= 10
    int pipeCount = 0;
    pid_t pid;
    int emptyDir = 1; //Empty directory flag

    // Iterate through the entries of dir
    while((entry = readdir(dir)) != NULL){
        // Skip . and .. dirs
        entryName = entry->d_name;
        if(!strcmp(entryName, ".") || !strcmp(entryName, "..")){
            continue; // Jump to next iteration
        }
        
        emptyDir = 0; //Set flag if not empty
        // Set up pipe
        int fd[2];
        int ret = pipe(fd);
        if (ret == -1){
            perror("Error creating pipe\n");
            exit(-1);
        }

        // Store the read end of the pipe in an array
        readEnd[pipeCount] = fd[0];
        pipeCount += 1;

        // Fork for each dirent
        pid = fork();
        if (pid < 0){ // Fork failed
            perror("Error forking child process\n");
            exit(-1);
        }
        else if (pid == 0){ // Child Process
            // Close read end
            close(fd[0]);

            // Set up new path and pipe end
            char new_path[1024];
            char pipeEnd[10];
            snprintf(new_path, sizeof(new_path), "%s/%s", file_path, entry->d_name);
            sprintf(pipeEnd, "%d", fd[1]);

            // Execute the appropriate proces with respect to the entry type
            if(entry->d_type == DT_DIR){ // Entry is a directory
                char *nonLeafArgs[] = {"./nonleaf_process", new_path, pipeEnd, NULL};
                if (execvp(nonLeafArgs[0], nonLeafArgs) < 0){
                    perror("Error execing to nonleaf_process\n");
                    exit(-1);
                }
            }
            else{ // Entry is a leaf
                char *leafArgs[] = {"./leaf_process", new_path, pipeEnd, NULL};
                if (execvp(leafArgs[0], leafArgs) < 0){
                    perror("Error execing to leaf_process\n");
                    exit(-1);
                }
            }
        }
    }

    // Read data from child processes' pipes and write to the parent process's pipe if not empty
    if (emptyDir == 0){
        waitpid(pid, NULL, 0);
        char buffer[1024];
        int bytesRead = 0;
        for (int i = 0; i < pipeCount; i++){
            memset(buffer, '\0', sizeof(buffer));
            bytesRead = read(readEnd[i], buffer, sizeof(buffer));
            if (bytesRead < 0){
                perror("Error reading from child process's pipe\n");
                exit(-1);
            }
            if (buffer[0] != '\0'){
                strncat(data, buffer, bytesRead);
            }
            close(readEnd[i]);
        }
    }

    closedir(dir);

    // Write collected data to the parent process's pipe if not empty
    if (emptyDir == 0){
        write(write_end, data, strlen(data));
    }
    else{
        write(write_end, "", 1);
    }
    close(write_end);
    
    // Exit successfully
    exit(0);
}
