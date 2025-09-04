#include <stdlib.h>
#include "utils.h"

int generate_random_value() {
    return rand() % (1 << 16);
}
