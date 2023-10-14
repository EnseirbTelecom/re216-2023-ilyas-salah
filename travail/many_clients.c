#include <assert.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define nbre_client 3

int main(int argc, char const *argv[])
{
    for (int i = 0; i < nbre_client; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("./ex","ex","test",NULL);
            break;
        }
        
    }
    
    exit(EXIT_SUCCESS);
    return 0;
}
