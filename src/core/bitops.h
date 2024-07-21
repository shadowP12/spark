#pragma once

static inline int get_shift_from_power_of_2(unsigned int bits)
{
    for (unsigned int i = 0; i < 32; i++)
    {
        if (bits == (unsigned int)(1 << i))
        {
            return i;
        }
    }

    return -1;
}