#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
//for fork
#include <sys/types.h>
#include <unistd.h>


#include "path_dt.h"
#include "dir_tracker.h"

void generate_traking_process(char *dir_path, char *CACHE_DIR, char *path_to_sh, char *ISOLATED_SPACE_DIR)
{
    bool exists;
    Path_DT father = make_path(dir_path, &exists);
    if(exists == false)
    {
        printf("%s is not a file and was skipped\n", father.fullPath);
        return;
    }

    pid_t pid_process = fork();
    if(pid_process == 0)//sunt in copil
    {
        track(father, CACHE_DIR, path_to_sh, ISOLATED_SPACE_DIR);
        exit(EXIT_SUCCESS);
    }
    else //sunt in parinte
    {
        return;
    }
}

int execute_shell_script(char *path_file_with_no_rights, char *name_file, char *path_to_sh_script, char *ISOLATED_SPACE_DIR)
{

    int pfd[2];
	if(pipe(pfd)<0)
	{
	  perror("Eroare la crearea pipe-ului\n");
	  exit(1);
	}

    pid_t pid_process = fork();
    if(pid_process == 0)//sunt in copil
    {
        close(pfd[0]);/* inchide capatul de citire; */
        //write(pfd[1],buff,len);
        dup2(pfd[1],1);
        execl(path_to_sh_script, "FS", path_file_with_no_rights, ISOLATED_SPACE_DIR,(char *)NULL);
        
        close(pfd[1]);
        perror("Exec didn t overwrite\n");
        exit(EXIT_FAILURE);
    }
    else //sunt in parinte
    {
        close(pfd[1]); /* inchide capatul de scriere; */
        
        char response[FULL_PATH_LENGHT];
        for(int i = 0; i < FULL_PATH_LENGHT; i ++)//fara asta citeste garbage din read
            response[i] = '\0';
        
        read(pfd[0], response, FULL_PATH_LENGHT);
        //printf("RESPONSE: [%s]\nresp nel=%ld\n", response, strlen(response));

        if(strcmp(response, "SAFE\n") != 0)
        {
            // printf("NSFW %s\n", name_file);
            char dest[FULL_PATH_LENGHT];
            strcpy(dest, ISOLATED_SPACE_DIR);
            strcat(dest, name_file);

            if (rename(path_file_with_no_rights, dest) != 0) {
                if (remove(path_file_with_no_rights) != 0) {
                    perror("Error deleting original file");
                    exit(EXIT_FAILURE);
                }
                perror("Error moving file");
                exit(EXIT_FAILURE);
            }
            return 1;
        }
        else
        {
            return 0;
            //printf("e ok\n");
        }
        
        close(pfd[0]);
    }
}