///

#include <cstdint>
#include <cstring>
#include <wchar.h>
#include <vector>
#include "rm.h"

inline bool IsArg(const wchar_t *candidate, const wchar_t *longname)
{
  if (wcscmp(candidate, longname) == 0)
    return true;
  return false;
}

inline bool IsArg(const wchar_t *candidate, const wchar_t *shortname,
				  const wchar_t *longname)
{
  if (wcscmp(candidate, shortname) == 0 ||
      (longname != nullptr && wcscmp(candidate, longname) == 0))
    return true;
  return false;
}

inline bool IsArg(const wchar_t *candidate, const wchar_t *longname, size_t n,
				  const wchar_t **off)
{
  auto l = wcslen(candidate);
  if (l < n)
    return false;
  if (wcsncmp(candidate, longname, n) == 0) {
    if (l > n && candidate[n] == '=') {
      *off = candidate + n + 1;
    } else {
      *off = nullptr;
    }
    return true;
  }
  return false;
}

void AppUsage()
{
	const char *kUsage =
		R"(OVERVIEW: UNC Removeable Utility
Usage: remove <options> [pathspec]
OPTIONS:
  -h [--help]      print usage and exit
)";
	printf("%s\n", kUsage);
}

bool ParseArgs(int Argc, wchar_t **Argv,std::vector<const wchar_t *> &files)
{
	const wchar_t *va{ nullptr };
	for (int i = 1; i < Argc; i++) {
		auto Arg = Argv[i];
		if (IsArg(Arg, L"-h", L"--help")) {
			AppUsage();
			exit(0);
		} else if (Arg[0]==L'-') {
			///
		} else {
			files.push_back(Arg);
		}
	}
	return true;
}

int wmain(int argc,wchar_t **argv){
    ///
	std::vector<const wchar_t *> files;
	if (ParseArgs(argc, argv, files)) {
		for (auto &f : files) {
			RemoveFileEx(f);
		}
	}
    return 0;
}