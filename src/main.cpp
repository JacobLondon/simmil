#include <cstdio>
#include <cstring>

#include "modules.hpp"

// TODO: Fix rand_range, divides by 0 sometimes?

int main(int argc, char **argv)
{
    auto ctx = pse::context("Simmil", PSE_RESOLUTION_43_1024_768, 60);

    if (arg_check(argc, argv, "--demo")) {
        ctx.run(demo_setup, demo_update, NULL);
    }
    else if (arg_check(argc, argv, "--mil")) {
        ctx.run(simmil_setup, simmil_update, simmil_cleanup);
    }

    return 0;
}