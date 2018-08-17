#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string>
#include <vector>
#include <cerrno>

#include "logger.h"
#include "common.h"

extern "C" {

int run_script(std::string arg0, std::vector<std::string> env)
{
    /* Convert the vector containing environment variables to the format
     * expected by execle(). */
    char const* envp[env.size() + 1];
    for (int i = 0; i < env.size(); ++i) {
        envp[i] = env[i].c_str();
    }
    envp[env.size()] = (char const*) NULL;

    /* fork() & execle() */
    int ret, wstatus, exitcode;
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        LOG_ERROR(runscript_logger, RUNSCRIPT_FORK_FAILED).arg(strerror(errno));
        return -1;
    }
    if (pid == 0) {
        /* Child process */
        ret = execle(script_path.c_str(), script_name.c_str(), arg0.c_str(), (char *)NULL, envp);
        LOG_ERROR(runscript_logger, RUNSCRIPT_EXEC_FAILED).arg(strerror(errno));
        /* This only exists the child, not Kea itself. */
        exit(EXIT_FAILURE);
    } else {
        if (script_wait) {
            /* Parent process */
            LOG_DEBUG(runscript_logger, 50, RUNSCRIPT_WAITING_SCRIPT);
            ret = wait(&wstatus);
            if (ret == -1) {
                LOG_ERROR(runscript_logger, RUNSCRIPT_WAITPID_FAILED).arg(strerror(errno));
                return -1;
            }
            /* Get exit code */
            if (WIFEXITED(wstatus))
                exitcode = WEXITSTATUS(wstatus);
            else
                /* By default, assume everything worked well */
                exitcode = 0;
            return exitcode;
        } else {
            return 0;
        }
    }
}

} // end extern "C"
