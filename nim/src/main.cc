#include <nim/nim.h>
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[])
{
    ::srand(static_cast<unsigned>(::time(nullptr)));
    nim::Application app(argc, const_cast<const char**>(argv));
    return app.Run();
}