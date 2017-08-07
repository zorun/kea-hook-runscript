// version.cc
#include <hooks/hooks.h>
extern "C" {
int version() {
    return (KEA_HOOKS_VERSION);
}
}
