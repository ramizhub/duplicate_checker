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
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <openssl/sha.h>
#include "duplicate_checker_library.h"

int main(int argc, char * argv[])
{
    if(handle_low_arguments(argc, argv) == -1)
        exit(EXIT_FAILURE);
    
    int process_count;         
    int task_per_worker;
    char folder_name[PATH_MAX];

    if(accept_arguments(argc, argv, &process_count, &task_per_worker, folder_name) == -1) {
        show_debug_info("accept_arguments", "ended program", "", "", -1);
        exit(EXIT_FAILURE);
    }

    show_debug_info("accept_arguments", "succesfully accepted arguments", "", "", -1);
    show_debug_info("accept_arguments", "FOLDER_NAME", folder_name, "", -1);
    show_debug_info("accept_arguments", "PROCESSES_COUNT", "", "", process_count);
    show_debug_info("accept_arguments", "TASK_FOR_EACH_WORKER_COUNT", "", "", task_per_worker);

    /* Take away the main process from this number and get the number of child processes. */
    process_count -= 1;

    /* 
    *   GHashTable g_hash_table_new (GHashFunc hash_func, GEqualFunc key_equal_func). Source: https://docs.gtk.org/glib/index.html.
    *   Hash values returned by hash_func are used to determine where keys are stored within the GHashTable data structure. 
    *   key_equal_func is used when looking up keys in the GHashTable.
    */    
    GHashTable * report_storage = g_hash_table_new(g_str_hash, g_str_equal);
    
    /* 
    *   This data structure accumulates file names. 
    *   Each child process gets its own set of file names. 'task_per_worker' variable defines the size of the set.
    */
    char TT[process_count][task_per_worker][PATH_MAX];
    
    /*  This function is called to prepare the TT data structure and avoid errors. */
    clean_TT(process_count, task_per_worker, TT);
    
    /*  Two-way communication between two processes requires two pipes for each pair. */
    int pipes[process_count*2][2];
    pid_t pid;
    
    /*  This data structure stores the pid of each child process for further access and deletion. */
    pid_t pids[process_count];
    
    for(int i = 0; i < (process_count * 2); i++)
        if(pipe(pipes[i]) == -1) {
            show_debug_info("pipe", "can't create pipe with index", "", "", i);
            return -1;
        }
    show_debug_info("main", "created pipes", "count of pipes", "", (process_count*2));


    /* 
    *   Child processes are created in the major loop, which are infinite loops, each of which is waiting for data to be processed.
    *   'mwcr' - master writes child reads ; 'cwmr' - child writes master reads. These are indexes for self-identification of pipes.
    */
    for(int pi = 0, mwcr = 0, cwmr = 1; pi < process_count; pi++, mwcr = mwcr + 2, cwmr = cwmr + 2) {
        pid = fork();
        if(pid == -1) {
            show_debug_info("fork", "can't create a child process", "", "", -1);
            return -1;
        }
        show_debug_info("main", "successfully created child process", "pid of child", "", pid);
        if(pid == 0)
            while(1)
                for(int ti = 0; ti < task_per_worker; ti++) {
                    int processed_bytes;
                    int sum_bytes = 0;
                    unsigned int task_length;
                    int left_count;
                    int hash_length = 32;
                    
                    char task[PATH_MAX];
                    unsigned char sha256_hash[SHA256_DIGEST_LENGTH];

                    /*  Waiting for data to be received from the parent process. */
                    left_count = sizeof(int);
                    do {
                        processed_bytes = read(pipes[mwcr][0], &task_length, left_count);
                        if(processed_bytes == -1) {
                            show_debug_info("send_tasks_fill_report_storage", "error in reading from pipe by file descriptor", "", "", -1);
                            return -1;
                        }
                        sum_bytes += processed_bytes;
                        left_count -= processed_bytes;
                    } while(sum_bytes != sizeof(int));
                    show_debug_info("child process", "successfully read bytes from parent in 'task_length' variable", "count of read bytes", "", sum_bytes);
                    sum_bytes = 0;
                    

                    left_count = sizeof(char) * task_length;
                    do {
                        processed_bytes = read(pipes[mwcr][0], task, left_count);
                        if(processed_bytes == -1) {
                            show_debug_info("send_tasks_fill_report_storage", "error in reading from pipe by file descriptor", "", "", -1);
                            return -1;
                        }
                        sum_bytes += processed_bytes;
                        left_count -= processed_bytes;
                    } while(sum_bytes != sizeof(char) * task_length);
                    show_debug_info("child process", "successfully read bytes from parent in 'task' variable", "count of read bytes", "", sum_bytes);
                    show_debug_info("child process", "successfully read bytes from parent in 'task' variable", "variable value", task, -1);
                    sum_bytes = 0;        
                    
                    if(make_sha256_hash(task, sha256_hash) == -1) {
                        show_debug_info("child process", "make_sha256_hash returned -1", "ended program", "", -1);
                        exit(EXIT_FAILURE);
                    }
            

                    /*  Sending processed data to parent. */
                    left_count = sizeof(int);
                    do {
                        processed_bytes = write(pipes[cwmr][1], &task_length, left_count);
                        if(processed_bytes == -1) {
                            show_debug_info("send_tasks_fill_report_storage", "error in writing in pipe by file descriptor", "", "", -1);
                            return -1;
                        }
                        sum_bytes += processed_bytes;
                        left_count -= processed_bytes;
                    } while(sum_bytes != sizeof(int));
                    show_debug_info("child process", "successfully send bytes to master of 'task_length' variable", "count of sended bytes", "", sum_bytes);
                    sum_bytes = 0;
                    

                    left_count = sizeof(char) * task_length;
                    do {
                        processed_bytes = write(pipes[cwmr][1], task, left_count);
                        if(processed_bytes == -1) {
                            show_debug_info("send_tasks_fill_report_storage", "error in writing in pipe by file descriptor", "", "", -1);
                            return -1;
                        }
                        sum_bytes += processed_bytes;
                        left_count -= processed_bytes;
                    } while(sum_bytes != sizeof(char) * task_length);
                    show_debug_info("child process", "successfully send bytes to master of 'task' variable", "count of sended bytes", "", sum_bytes);
                    show_debug_info("child process", "successfully send bytes to master of 'task' variable", "variable value", task, -1);
                    sum_bytes = 0;


                    left_count = sizeof(unsigned char) * hash_length;
                    do {
                        processed_bytes = write(pipes[cwmr][1], sha256_hash, left_count);
                        if(processed_bytes == -1) {
                            show_debug_info("send_tasks_fill_report_storage", "error in writing in pipe by file descriptor", "", "", -1);
                            return -1;
                        }
                        sum_bytes += processed_bytes;
                        left_count -= processed_bytes;
                    } while(sum_bytes != sizeof(unsigned char) * hash_length);
                    show_debug_info("child process", "successfully send bytes to master of 'hash_length' variable", "count of sended bytes", "", sum_bytes);
                    sum_bytes = 0;
                }
        /*  Parent process saves pid of child. */
        else if(pid > 0)
            pids[pi] = pid; 
    }

    if(process_directory_recursively(folder_name, report_storage, process_count, task_per_worker, TT, pipes) == -1) {
        show_debug_info("process_directory_recursively", "ended program", "", "", -1);
        return -1;
    }

    if(process_remaining_tasks(report_storage, process_count, task_per_worker, TT, pipes) == -1) {
        show_debug_info("process_remaining_tasks", "ended program", "", "", -1);
        return -1;
    }

    if(kill_workers(pids, process_count) == -1) {
        show_debug_info("kill_workers", "ended program", "", "", -1);
        return -1;
    }
    output_data(report_storage);
}