#pragma once

#include "core/module.h"
#include <string>
#include <vector>

class Platform : public Module<Platform>
{
public:
    void setup();

    void cleanup();

    double get_time();

    bool poll_events();
};