#include <string>
#include <vector>

extern "C" {

/* Runs the configured script with the given (single) argument and
 * environment variables.  Returns -1 upon failure, or the exit code of
 * the script upon success. */
int run_script(std::string arg0, std::vector<std::string> env);

}
