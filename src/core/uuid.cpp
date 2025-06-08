#include "uuid.h"
#include <uuid4.h>

static bool uuid4_inited = false;
std::string uuid4_generate()
{
    if (!uuid4_inited)
    {
        uuid4_init();
        uuid4_inited = true;
    }

    char buf[UUID4_LEN];
    uuid4_generate(buf);
    return buf;
}