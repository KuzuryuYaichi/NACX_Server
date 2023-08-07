#include "TinyInstance.h"
#include "TinyFileLock.h"

TinyFileLock FileLock;

int main(int argc, char **argv)
{
    if (TinyFileLock::running())
    {
        std::cout << "The application is already running.\nAllowed to run only one instance of the application." << std::endl;
        return 0;
    }
    TinyInstance tinyInstance;
    tinyInstance.join();
}
