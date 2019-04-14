/* Copyright (c) 2017-2019 by Baptiste Jonglez
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <vector>

extern "C" {

/* Runs the configured script with the given (single) argument and
 * environment variables.  Returns -1 upon failure, or the exit code of
 * the script upon success. */
int run_script(std::string arg0, std::vector<std::string> env);

}
