#include <magick/api.h>

__attribute__((constructor))
static void initialize_magick()
{
    InitializeMagick(NULL);
}
