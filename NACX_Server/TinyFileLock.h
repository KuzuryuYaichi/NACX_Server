#ifndef TINY_FILELOCK
#define TINY_FILELOCK

#include <filesystem>

class TinyFileLock
{
public:
	TinyFileLock();
	static bool running();
private:
	std::filesystem::path FilePath;
};

#endif
