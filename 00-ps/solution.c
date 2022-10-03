#include <solution.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


const char directory_proc[] = "/proc/";
const char stat[] = "/stat";

struct stat_file {
    int pid;
    char name[256];
    char status;
};

void ps(void)
{
    DIR *dir;
    FILE *file_status;
    struct dirent *entry;
    struct stat_file statFile;

    dir = opendir(directory_proc);
    if (!dir) {
	report_error(directory_proc, (int)errno);    
        //perror("diropen");
        return;
    };

    while ((entry = readdir(dir)) != NULL) {
        if (!isdigit(entry->d_name[0]))
            continue;

        char tmp_proc[256];

        snprintf(tmp_proc, sizeof(tmp_proc), "%s%s%s", directory_proc, entry->d_name, stat);
	
        file_status = fopen(tmp_proc, "r");
        if (file_status == NULL) {
		report_error(file_status, (int)errno);
		return;
            //perror("file open:");
           // exit(1);
        }
        fscanf(file_status, "%d %s %c", &statFile.pid, statFile.name, &statFile.status);
        fclose(file_status);
        printf("PID - %s, status - %c, name - %s\n",
               entry->d_name, statFile.status, statFile.name);

    };

    if (errno != 0){
        printf("Error in reading dir proc/\n");
        perror("readdir");
        exit(1);
    }
    closedir(dir);
}
