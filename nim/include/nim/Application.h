#pragma once

#include "nim_stdtypes.h"

namespace nim
{
    class Application final
    {
    public:
        Application(int argc, const char* argv[]);
        Application(const Application&) = delete;
        Application(Application&&) = delete;
        ~Application();

        int Run();

    private:
        struct impl;
        impl* m_impl;
    };
}