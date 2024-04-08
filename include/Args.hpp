#include <argparse/argparse.hpp>

using argparse::ArgumentParser;

class Args : protected ArgumentParser
{
public:
	Args(const int argc, const char *const *const argv);
};