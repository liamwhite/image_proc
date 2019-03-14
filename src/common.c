#include <signal.h>
#include <magick/api.h>

__attribute__((constructor))
static void initialize_magick()
{
    // HACK: Prevent GM from wrongly overriding signal handlers
    struct sigaction saved_signals[SIGSYS];

    for (int i = 0; i < SIGSYS; ++i)
        sigaction(i, NULL, &saved_signals[i]);

    InitializeMagick(NULL);

    for (int i = 0; i < SIGSYS; ++i)
        sigaction(i, &saved_signals[i], NULL);
}
