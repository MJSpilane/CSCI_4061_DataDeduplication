# CSCI 4061 Fall 2023 Programming Assignment 2

Canvas PA Group 60

### Group Members:
- Matthew Breach ([breac001](mailto:breac001@umn.edu))
- Matthew Johnson ([joh18723](mailto:joh18723@umn.edu))
- Harrison Wallander ([walla875](mailto:walla875@umn.edu))

### CSELabs Computer Used for Testing:
- csel-kh1260-14.cselabs.umn.edu

### Makefile Changes:
- None

### Additional Assumptions:
- The root directory must be emptied and refilled with its original contents before running the executable on it for a second time to ensure correct results
- The ordering of outputs to the output file does not matter so long as the output lines themselves are correct

### AI Code:
- None

### Individual Contributions:
- breac001
    - leaf_process.c, nonleaf_process.c, debugging, code linting
- joh18723
    - root_process.c - helper functions, debugging
- walla875
    - README.md, root_process.c - main/helping with redirection, debugging

### Our Data Deduplication Algorithm:
1. In root_process.c
    ```
    int fds[2] 
    pipe(fds)
    int pid = fork()
    if (pid = 0){ //Child case, exec into root
        close(fds[0]) //Close read end
        exec(nonleaf_process in root directory with root path and fds[1])
    }
    close(fds[1]) //Close write end
    wait(NULL)

    read pipe contents into all_filepath_hashvalue
    init and malloc space for dup_list and retain_list
    int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list) //Changed from intermediate, set return value of parse_hash to size varibale for function
    calls since it is the reasonable way to find dup/retain_list's sizes
    delete_duplicate_files(dup_list, size){
        for each item in dup_list{
            remove(item)
        }
    }
    create_symlinks(dup_list, retain_list, size){
        for int i in dup_list{
            symlink(retain_list[i], dup_list[i]) //Changed from intermediate, swapped retain and dup lists so symlink works as intended
        }
    }
    redirection(dup_list, size, argv[1]){
        //filename creation, different from intermediate, this code correctly concats strings to make the output file name
        char *filename = malloc(sizeof(char)*300)
        filename[0] = '\0'
        strcat(filename, "./")
        strcat(filename, output_file_folder)
        strcat(filename, extract_filename(root_dir))
        strcat(filename, ".txt")
        int temp = dup(STDOUT_FILENO) //Changed from intermediate, stores stdout
        int fd = open(output_name, WRITE, PERM)
        dup2(fd, STDOUT_FILENO)
        close(fd) //Changed from intermediate, close fd
        char pointedFile[100];
        for int i in dup_list{ //Print out each item in dup_list and where it points
            memset(pointedFile, '\0', sizeof(pointedFile)) //Changed from intermediate, fixed memset syntax
            readlink(dup_list[i], pointedFile, sizeof(pointedFile))
            fflush(stdout)
            printf("[<path of symbolic link> --> <path of retained file>] : [%s --> %s]\n", dup_list[i], pointedFile) 
            fflush(stdout) //Changed from intermediate, added flushes and edited string output formatting
        }
        dup2(temp, STDOUT_FILENO)
        close(temp) //changed from intermediate, reset stdout and close temp
    }
    free(dup_list)
    free(retain_list)
    ```

2. In nonleaf_process.c
    ```
    char path = argv[1]
    int write_end = (int) argv[2]

    char data[4098] // CHANGE: use this for data from child_process
    memset(data, '\0', sizeof(data)) // Initialize data buffer

    DIR *dir = opendir(path)  //CHANGE: open dir

    struct dirent *entry
    char *entryName
    int readEnd[10] // CHANGE: total num files <= 10
    int readidx
    pid_t pid // CHANGE: store pid in function scope
    int empty = 1; // CHANGE: marks if the directory is empty

    while((entry = readdir(dir)) != NULL){
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) { //Skip current/parent directory items
            continue
        }
        
        empty = 0; //CHANGE: set not empty flag
        int fd[2];
        int ret = pipe()
        readEnds[count] = fd[0]
        count++

        // CHANGE: Moved fork() to higher-level

        pid = fork() // Fork to child process
        if (pid == 0){ // Child process
            close(fd[0])
            char new_path[1024]
            char pipeEnd[10]

            // Write to new_path and pipeEnd
            snprintf(new_path, "{file_path}/{entry->d_name})
            snprintf(pipeEnd, {"fd[1]})

            if (entry->d_type == DT_DIR){ //Directory case
               exec to nonleaf_process
            }
            else{
                exec to leaf_process
            }
        }
    }

    // NEW: Added in pseudocode to handle reading from child processes' pipes and writing to 
    // parent process's pipe
    if (empty == 0){ // CHANGE: only wait and read from child processes if the directory is not empty
        waitpid(child pid)
        char buffer[1024]
        int bytesRead

        for (int i = 0; i < pipeCount; i++){
            memset(buffer, '\0', sizeof(buffer))
            bytesRead = read(readEnd[i] into buffer)
            strncat(buffer onto data)
            close(readEnd[i])
        }
    }
    closedir(dir)
    
    if (empty == 0) // CHANGE: only write data to parent if the directory is not empty
        write(data to write_end)
    }
    else{ // CHANGE: if directory is empty, write an empty string to parent
        write(empty string to write_end)
    }
    close(write_end)        
    ```
3. In leaf_process.c
    (Most of this is already written for the intermediate submission, so only 
    the final else statement is going to be in pseudocode here)
    ```
    else{
        write pipeStr to write end of pipe at argv[2]
        close write end of pipe
    }
    ```
