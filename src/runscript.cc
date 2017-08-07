#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
// TODO: Not nescessary
#include <stdio.h>

#include <string>
#include <vector>

#include "common.h"

extern "C" {

int run_script(std::string arg0, std::vector<std::string> env)
{
    /* Convert the vector containing environment variables to the format
     * expected by execle(). */
    char const* envp[env.size() + 1];
    for (int i = 0; i < env.size(); ++i) {
        envp[i] = env[i].data();
    }
    envp[env.size()] = (char const*) NULL;

    /* fork() & execle() */
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
        ret = execle(script_path.data(), script_name.data(), arg0.data(), (char *)NULL, envp);
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
