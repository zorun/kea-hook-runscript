#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
// TODO: Not nescessary
#include <stdio.h>

extern "C" {

/* Runs the given script with the given argument and environment
 * variables.  Returns -1 upon failure, or the exit code of the script
 * upon success. */
int run_script(const char *scriptpath, const char *arg0, char *const *envp)
{
    int ret, wstatus, exitcode;
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        // TODO: logging, errno is usable
        fprintf(stderr, "Error during fork()\n");
        return -1;
    }
    if (pid == 0) {
        /* Child process */
        const char *scriptname = strrchr(scriptpath, '/');
        if (scriptname == NULL) {
            scriptname = scriptpath + strlen(scriptpath);
        }
        ret = execle(scriptpath, scriptname, arg0, (char *)NULL, envp);
        // TODO: logging, errno is usable
        fprintf(stderr, "Error during execle() in child\n");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        fprintf(stderr, "Waiting for script to return...\n");
        ret = wait(&wstatus);
        if (ret == -1) {
            // TODO: logging, errno is usable
            fprintf(stderr, "waitpid() failure\n");
            return -1;
        }
        /* Get exit code */
        if (WIFEXITED(wstatus))
            exitcode = WEXITSTATUS(wstatus);
        else
            /* By default, assume everything worked well */
            exitcode = 0;
        return exitcode;
    }
}

} // end extern "C"
