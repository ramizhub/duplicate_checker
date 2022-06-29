/*
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                @@@@@@@       @@@@   @@@   @@&      @@@*  ,@@@@@@   @@@@     @@@@@@    @@@         @         @@@@@@@
                @@@@@@@   @@    @@   @@@   @@&   @    @*  ,@@@@@@   @@   %@@   @@@      @@         @   (@@@@@@@@@@@@
                @@@@@@@   @@@   @@   @@@   @@&        @*  ,@@@@@@   @@   @@@@@@@@@  @@  @@@@@   @@@@        @@@@@@@@
                @@@@@@@   @@@   @@   @@@   @@&   @@@@@@*  ,@@@@@@   @@   @@@   @@        @@@@   @@@@   (@@@@@@@@@@@@
                @@@@@@@        @@@@       @@@&   @@@@@@*       @@   @@@       @@   @@@@   @@@   @@@@         @@@@@@@
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                @@@@@@@@@@@@@@@@   .@@@@   @@@%  .@@       .@@@@    @@@@   @@@@   @@       .@@     .@@@@@@@@@@@@@@@@
                @@@@@@@@@@@@@@        @@   @@@&   @@        @@        @@   @@    @@@        @@         @@@@@@@@@@@@@
                @@@@@@@@@@@@@   @@@@@@@@          @@       @@   @@@@@@@@       @@@@@       @@@        @@@@@@@@@@@@@@
                @@@@@@@@@@@@@   @@@@  @@   @@@&   @@   @@@@@@   @@@@   @        @@@@   @@@@@@@   @   @@@@@@@@@@@@@@@
                @@@@@@@@@@@@@@        @@   @@@&   @@        @@        @@   @@@    @@        @@   @@   @@@@@@@@@@@@@@
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                shorturl.at/bsAGR
*/

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glib.h>
#include <openssl/sha.h>

#define __MIN__PROC__NUMBER 4
#define __MIN__TASK__PER__WORKER__NUMBER 1
#define BUFFSIZE 256

/*  The variable is declared external for convenient function calls that repeatedly access it. */
bool is_debug = false;





void show_debug_info(char section_name[], char definition[], char str_data1[], char str_data2[], int numeric_data)
{   
    if(is_debug) {
        fprintf(stderr, "\n%*s()  (process_id = %d)\n", 50, section_name, getpid());
        if(numeric_data == -1)
            fprintf(stderr, "%*s\t%*s %*s\n", 50, definition, 20, str_data1, 20, str_data2);
        else
            fprintf(stderr, "%*s\t%*d %*s %*s\n", 50, definition, 20, numeric_data, 20, str_data1, 20, str_data2);
    }
}




/* A function that copies one line to another. It was created in order not to load other parts of the code with error checks. */
int copy_in_string(char * dest, char * src, int src_length)
{   
    if(dest == NULL || src == NULL) {
        show_debug_info("copy_in_string", "destination and source strings can not be NULL", dest, src, -1);
        return -1;
    }

    if(src_length + 1 <= PATH_MAX)
        strncpy(dest, src, src_length + 1);                                 
    else {
        show_debug_info("copy_in_string", "not enough memory to copy string in other", src, dest, -1);
        return -1;
    }
    show_debug_info("copy_in_string", "succesfully copied string in other", src, dest, -1);        
}





/* 
*   A function concatenating the name of the current directory, 
*   the '/' sign and the name of the bare element to create a full path.
*/
int make_file_name(char * dest, char * src, int dest_length, int src_length)
{
    if(dest == NULL || src == NULL)
        return -1;
    
    if(dest_length + src_length + 2 <= PATH_MAX) {
        strcat(dest, "/");
        strcat(dest, src);                            
    }
    else {
        show_debug_info("make_file_name", "not enough memory to concatenate string to", src, dest, -1);
        return -1;
    }
    show_debug_info("make_file_name", "succesfully made file name", src, dest, -1);
}





void show_help(void)
{
    fprintf(stdout, "USAGE: -f FOLDER_NAME -p PROCESSES_COUNT -t TASK_FOR_EACH_WORKER_COUNT \n\t [-h] [-d] \n\n");
    fprintf(stdout, "-f\t\tSet the directory for the search. \n\n");
    fprintf(stdout, "-p\t\tSet the number of running\n\t\trunning processes(minimum %d) \n\n", __MIN__PROC__NUMBER);
    fprintf(stdout, "-t\t\tSet the number of tasks for each process (minimum %d) \n\n", __MIN__TASK__PER__WORKER__NUMBER);
    fprintf(stdout, "-d\t\tDisplay debugging information \n\n");
    fprintf(stdout, "-h\t\tDisplay help\n\t\tand exit \n");
}





int handle_low_arguments(int argc, char * const argv[])
{   
    if(argc < 6)
        if(argc == 1) {
                fprintf(stderr, "missing operands \ntry '%s -h' for more information \n", argv[0]);
                return -1;
            }                
        else 
            if(strncmp(argv[1], "-h", strlen("-h")) == 0) {
                show_help();
                exit(EXIT_SUCCESS);
            }
            else {
                fprintf(stderr, "missing operands \ntry '%s -h' for more information \n", argv[0]);
                return -1;
            }
}





int accept_arguments(int argc, char * const argv[], int * process_count, int * task_per_worker, char * folder_name)
{   
    char option_character;
    while((option_character = getopt(argc, argv, "f:p:t:hd")) != -1)
        switch(option_character) {
            case 'f':
                if(optarg != NULL)
                    if(copy_in_string(folder_name, optarg, strlen(optarg)) == -1)
                        return -1;
                break;
            case 'p':
                if(optarg != NULL)
                    *process_count = atoi(optarg);
                break;
            case 'h':
                show_help();
                break;
            case 'd':
                is_debug = true;
                break;
            case 't':
                *task_per_worker = atoi(optarg);
                break;
            default:
                fprintf(stderr, "invalid option -- \ntry '%s -h' for more information\n", argv[0]);
                return -1;
        }
    if(*process_count < __MIN__PROC__NUMBER || *task_per_worker < __MIN__TASK__PER__WORKER__NUMBER) {
        fprintf(stderr, "invalid numeric values \ntry '%s -h' for more information \n", argv[0]);
        return -1;
    }
}





/*  Clearing occurs by replacing the first character with a NULL byte. Functions interacting with. */
void clean_TT(int rows, int columns, char TT[][columns][PATH_MAX])
{
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < columns; c++)
            TT[r][c][0] = '\0';
}





_Bool is_TT_full(int rows, int columns, char TT[][columns][PATH_MAX])
{   
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < columns; c++)
            if(TT[r][c][0] == '\0')
                return false;
    return true;
}





_Bool is_TT_empty(int rows, int columns, char TT[][columns][PATH_MAX])
{   
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < columns; c++)
            if(TT[r][c][0] != '\0')
                return false;
    return true;
}





int enter_task_in_TT(int rows, int columns, char TT[][columns][PATH_MAX], char * task)
{
    for(int r = 0; r < rows; r++)
        for(int c = 0; c < columns; c++)
            if(TT[r][c][0] == '\0') {
                if(copy_in_string(TT[r][c], task, strlen(task)) == -1)
                    return -1;
                return 0;
            }
}





void output_data(GHashTable * report_storage) {
    GHashTableIter iterator;
    gpointer key, value;
    fprintf(stdout, "%*s\n\n", 50, "DUPLICATES");

    g_hash_table_iter_init(&iterator, report_storage);
    while(g_hash_table_iter_next(&iterator, &key, &value)) {
        if(strchr((char*)value, ',' ) != NULL)
            fprintf(stdout, "%s\n", (char *)value);
    }
}





int kill_workers(int pids[], int limit)
{
    for(int i = 0; i < limit; i++)
        if(kill(pids[i], SIGKILL) == -1) {
            show_debug_info("kill_workers", "error in process killing", "", "", pids[i]);
            return -1;
        }
    show_debug_info("kill_workers", "succesfully killed all child processes", "", "", -1);
}





/* 
*   Using OpenSSL cryptographic hash function that generates SHA256 especially. Source: https://www.openssl.org/.
*   int SHA256_Init(SHA256_CTX *c);
*   int SHA256_Update(SHA256_CTX *c, const void *data, size_t len);
*   int SHA256_Final(unsigned char *md, SHA256_CTX *c);
*
*   This function opens file, reads by file descriptor, generates hash basing on file content and closes file.
*/
int make_sha256_hash(char * path, unsigned char * hash_target)
{   
    int fd;
    int i;
    char buffer[BUFFSIZE];

    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);

    if((fd = open(path, O_RDONLY, 0)) == -1) {
        show_debug_info("make_sha256_hash", "can't open file by file_descriptor", path, "", -1);
        return -1;
    }
    show_debug_info("make_sha256_hash", "succesfully opened file by file descriptor", "file", path, -1);

    do {
        if((i = read(fd, buffer, BUFFSIZE)) == -1) {
            show_debug_info("make_sha256_hash", "can't read file by file_descriptor", path, "", -1);
            return -1;
        }
        SHA256_Update(&sha256_ctx, buffer, i);
    } while(i > 0);
    show_debug_info("make_sha256_hash", "succesfully read bytes from file and made hash", "file", path, -1);

    if(close(fd) == -1) {
       show_debug_info("make_sha256_hash", "can't close file by file_descriptor", path, "", -1);
       return -1;
    }
    
    show_debug_info("make_sha256_hash", "succesfully closed file by file_descriptor", "file", path, -1);
    SHA256_Final(hash_target, &sha256_ctx);    
}





/* 
*   Before placing the value in report storage, function will check the key.
*   If there is already a value under such a key, it will stretch, connect to the new one and return back to the same cell.
*   If the cell is not occupied by this key, the new value is immediately placed there.
*/
int fill_report_storage(GHashTable * report_storage, unsigned char sha256_hash[], char file_name[])
{   
    char new_task[PATH_MAX];
    if(g_hash_table_lookup(report_storage, sha256_hash) != NULL) {
        strcpy(new_task, (char *)g_hash_table_lookup(report_storage, sha256_hash));

        if(strlen(file_name) + strlen(new_task) + 2 <= PATH_MAX) {
            strcat(new_task, ",");
            strcat(new_task, file_name);
        }
        
        else {
            show_debug_info("fill_report_storage", "not enough memory to concatenate string to", file_name, "", -1);
            return -1;
        }
        show_debug_info("fill_report_storage", "succesfully made new value for report storage", "new value", new_task, -1);
        g_hash_table_insert(report_storage, g_strdup(sha256_hash), g_strdup(new_task));
        show_debug_info("fill_report_storage", "inserted value in report storage", "new value", new_task, -1);
    }
    else {
        g_hash_table_insert(report_storage, g_strdup(sha256_hash), g_strdup(file_name));
        show_debug_info("fill_report_storage", "inserted value in report storage", "new value", file_name, -1);
    }
}





/* 
*   This function sends and receives data in major loop. 
*   'mwcr' - master writes child reads ; 'cwmr' - child writes master reads. These are indexes for self-identification of pipes.
*/
int send_tasks_fill_report_storage(GHashTable * report_storage, int rows, int columns, char TT[][columns][PATH_MAX], int pipes[][2])
{   
    for(int pi = 0, mwcr = 0, cwmr = 1; pi < rows; pi++, mwcr = mwcr + 2, cwmr = cwmr + 2)
        for(int ti = 0; ti < columns; ti++) {
            /* Avoiding incomplete TT. */
            if(TT[pi][ti][0] == '\0')
                return 0;
            
            int processed_bytes;
            int sum_bytes = 0;

            int task_length = strlen(TT[pi][ti]) + 1;
            unsigned char sha256_hash[SHA256_DIGEST_LENGTH];
            int hash_length = 32;
            int left_count;
            char task[PATH_MAX];


            /* Sending data to child processes. */
            left_count = sizeof(int);
            do {
                processed_bytes = write(pipes[mwcr][1], &task_length, left_count);
                if(processed_bytes == -1) {
                    show_debug_info("send_tasks_fill_report_storage", "error in writing in pipe by file descriptor", "", "", -1);
                    return -1;
                }
                sum_bytes += processed_bytes;
                left_count -= processed_bytes;
            } while(sum_bytes != sizeof(int));
            show_debug_info("send_tasks_fill_report_storage", "successfully send bytes from 'hash_length' variable in child", "count of sended bytes", "", sum_bytes);
            sum_bytes = 0;


            left_count = sizeof(char) * task_length;
            do {
                processed_bytes = write(pipes[mwcr][1], TT[pi][ti] + sum_bytes, left_count);
                if(processed_bytes == -1) {
                    show_debug_info("send_tasks_fill_report_storage", "error in writing in pipe by file descriptor", "", "", -1);
                    return -1;
                }
                sum_bytes += processed_bytes;
                left_count -= processed_bytes;
            } while(sum_bytes != sizeof(char) * task_length);
            show_debug_info("send_tasks_fill_report_storage", "successfully send bytes from 'task' variable in child", "count of sended bytes", "", sum_bytes);
            sum_bytes = 0;


            /* Receiving processed data from child processes. */
            left_count = sizeof(int);
            do {
                processed_bytes = read(pipes[cwmr][0], &task_length, left_count);
                if(processed_bytes == -1) {
                    show_debug_info("send_tasks_fill_report_storage", "error in reading from pipe by file descriptor", "", "", -1);
                    return -1;
                }
                sum_bytes += processed_bytes;
                left_count -= processed_bytes;
            } while(sum_bytes != sizeof(int));
            show_debug_info("send_tasks_fill_report_storage", "successfully read bytes in 'hash_length' variable from child", "count of read bytes", "", sum_bytes);
            sum_bytes = 0;


            left_count = sizeof(char) * task_length;
            do {
                processed_bytes = read(pipes[cwmr][0], task + sum_bytes, left_count);
                if(processed_bytes == -1) {
                    show_debug_info("send_tasks_fill_report_storage", "error in reading from pipe by file descriptor", "", "", -1);
                    return -1;
                }
                sum_bytes += processed_bytes;
                left_count -= processed_bytes;
            } while(sum_bytes != sizeof(char) * task_length);
            show_debug_info("send_tasks_fill_report_storage", "successfully read bytes in 'task' variable from child", "count of read bytes", "", sum_bytes);
            sum_bytes = 0;


            left_count = sizeof(unsigned char) * hash_length;
            do {
                processed_bytes = read(pipes[cwmr][0], sha256_hash + sum_bytes, left_count);
                if(processed_bytes == -1) {
                    show_debug_info("send_tasks_fill_report_storage", "error in reading from pipe by file descriptor", "", "", -1);
                    return -1;
                }
                sum_bytes += processed_bytes;
                left_count -= processed_bytes;
            } while(sum_bytes != sizeof(unsigned char) * hash_length);
            show_debug_info("send_tasks_fill_report_storage", "successfully read bytes in 'sha256_hash' variable from child", "count of read bytes", "", sum_bytes);
            
            /* The key and value are placed in report storage. */
            if(fill_report_storage(report_storage, sha256_hash, task) == -1)
                return -1;
        }
}





/*
*   Any data in TT is unprocessed data. Data is sent only when the table is filled in.
*   This function does nothing but processes remaining data in TT.
*/
int process_remaining_tasks(GHashTable * report_storage, int rows, int columns, char TT[][columns][PATH_MAX], int pipes[][2])
{
    if(is_TT_empty(rows, columns, TT)) {
        show_debug_info("process_reamining_tasks", "task table is empty there are no remaining tasks", "returning 0", "", -1);
        return 0;     
    }
    
    show_debug_info("process_reamining_tasks", "there are tasks left in the table", "sending them to child processes", "", -1);
    
    if(send_tasks_fill_report_storage(report_storage, rows, columns, TT, pipes) == -1) {
        show_debug_info("process_reamining_tasks", "function send_tasks_fill_report_storage() returned -1", "returning -1", "", -1);
        return -1;
    }

    show_debug_info("process_reamining_tasks", "successfully processed remaining tasks", "", "", -1);
}





/* 
*   The function processes the directory recursively. When a file is found, it directs it to TT.
*   But if the table is full, then all its contents are sent to child processes.
*/
int process_directory_recursively(char * folder, GHashTable * report_storage, int rows, int columns, char TT[][columns][PATH_MAX], int pipes[][2])
{   
    struct dirent * directory_stat;
    struct stat src_stat;

    char object[PATH_MAX];
    char object_copy[PATH_MAX];

    if(copy_in_string(object, folder, strlen(folder)) == -1) {
        show_debug_info("process_directory_recursively", "function copy_in_string() returned -1", "returning -1 to main()", "", -1);    
        return -1;
    }

    if(copy_in_string(object_copy, object, strlen(object)) == -1) {
        show_debug_info("process_directory_recursively", "function copy_in_string() returned -1", "returning -1 to main()", "", -1);
        return -1;
    }

    DIR * dp = opendir(object);
    if(dp == NULL) {
        show_debug_info("process_directory_recursively", "can't open directory for processing", object, "", -1);
        return -1;
    }
    show_debug_info("process_directory_recursively", "succesfully opened directory by file descriptor", object, "", -1);

    while((directory_stat = readdir(dp))  != NULL ) {
        if(directory_stat->d_name[0] =='.')
            continue;

        if(copy_in_string(object, object_copy, strlen(object_copy)) == -1) {
            show_debug_info("process_directory_recursively", "function copy_in_string() returned -1", "returning -1 to main()", "", -1);
            return -1;
        }
        
        if(make_file_name(object, directory_stat->d_name, strlen(object), strlen(directory_stat->d_name)) == -1) {
            show_debug_info("process_directory_recursively", "function make_file_name() returned -1", "returning -1 to main()", "", -1);
            return -1;
        }
        
        if(lstat(object, &src_stat) != 0) {
            show_debug_info("process_directory_recursively", "can't get stat about folder element", object, "", -1);
            return -1;
        }
        show_debug_info("process_directory_recursively", "successfully got stat about object", object, "", -1);
        
        if( (src_stat.st_mode & S_IFMT) == S_IFREG ) {
            if(enter_task_in_TT(rows, columns, TT, object) == -1) {
                show_debug_info("process_directory_recursively", "function enter_task_in_TT() returned -1", "returining -1 to main()", "", -1);
                return -1;
            }
            
            if(is_TT_full(rows, columns, TT)) {
                if(send_tasks_fill_report_storage(report_storage, rows, columns, TT, pipes) == -1) {
                    show_debug_info("process_directory_recursively", "function send_tasks_fill_report_storage() returned -1", "returning -1 to main()", "", -1);
                    return -1;    
                }     
                clean_TT(rows, columns, TT);
            }
        }
        else if( (src_stat.st_mode & S_IFMT) == S_IFDIR )
            if(process_directory_recursively(object, report_storage, rows, columns, TT, pipes) == -1) {
                show_debug_info("process_directory_recursively", "another call of this function returned -1", "returning -1 to main()", "", -1);
                return -1;
            }
    }
}