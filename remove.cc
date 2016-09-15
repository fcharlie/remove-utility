/////
#include <cassert>
#include <string>
#include <array>
#include <Windows.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include <PathCch.h>

#pragma comment(lib,"Pathcch")

std::wstring GetFormattedMessage(DWORD result)
{
	LPWSTR errorText = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);
	if (errorText) {
		std::wstring tmp(errorText);
		wprintf(L"%s\n", errorText);
		LocalFree(errorText);
		return tmp;
	}
	return L"";
}


inline bool RemoveFileInternal(const std::wstring &file)
{
	if (!DeleteFileW(file.c_str())) {
		auto e = GetLastError();
		auto es = GetFormattedMessage(e);
		if (!es.empty()) {
			wprintf(L"Delete %s error\n", es.c_str());
		}
		return false;
	}
	wprintf(L"Delete %s success\n", file.c_str());
	return true;
}

bool RemoveAll(const std::wstring &dir)
{
	WIN32_FIND_DATAW find_data;
	std::wstring mdir = dir + L"\\*";
	HANDLE hFind = FindFirstFileW(mdir.c_str(), &find_data);
	if (hFind == INVALID_HANDLE_VALUE) {
		auto e = GetLastError();
		auto es = GetFormattedMessage(e);
		if (!es.empty()) {
			wprintf(L"RemoveAll File: %s\n", es.c_str());
		}
		return false;
	}
	do {
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (wcscmp(find_data.cFileName, L".") != 0 && wcscmp(find_data.cFileName, L"..") != 0) {
				std::wstring xdir = dir;
				xdir.push_back('\\');
				xdir.append(find_data.cFileName);
				RemoveAll(xdir);
				RemoveDirectoryW(xdir.c_str());
			}
		} else {
			std::wstring xfile = dir;
			xfile.push_back('\\');
			xfile.append(find_data.cFileName);
			RemoveFileInternal(xfile);
			/// Remove File
		}
	} while (FindNextFileW(hFind, &find_data));
	FindClose(hFind);
	return true;
}

inline bool PathIsDirectoryInternal(const wchar_t *lpszPath)
{
	DWORD dwAttr;
	if ((dwAttr = GetFileAttributesW(lpszPath)) == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool RemoveFileEx(const wchar_t *file)
{
	std::wstring ufile;
	auto flen = wcslen(file);
	/// UNC Path
	if (flen > 5 && wmemcmp(file, L"\\\\?\\", sizeof("\\\\?\\") - 1) == 0) {
		ufile.assign(file);
	} else if (flen > 4 && file[1] == ':') {
		///
		ufile.assign(L"\\\\?\\");
		ufile.append(file);
	} else {
		ufile.assign(L"\\\\?\\");
		std::array<wchar_t, PATHCCH_MAX_CCH> pwd;
		std::array<wchar_t, PATHCCH_MAX_CCH> tfile;
		auto len = GetCurrentDirectoryW(PATHCCH_MAX_CCH, pwd.data());
		if (len == 0) {
			return false;
		}
		auto ptr = pwd.data() + len;
		auto xlen = PATHCCH_MAX_CCH - len;
		if (xlen <= flen + 2)
			return false;
		wcscpy_s(ptr, 2, L"\\");
		ptr++;
		wcscpy_s(ptr, flen + 2, file);
		PathCchCanonicalizeEx(tfile.data(), PATHCCH_MAX_CCH, pwd.data(), PATHCCH_ALLOW_LONG_PATHS);
		ufile.append(tfile.data());
	}
	if (PathIsDirectoryInternal(ufile.c_str())) {
		if (RemoveAll(ufile)) {
			RemoveDirectoryW(ufile.c_str());
			return true;
		}
		return false;
	}
	return RemoveFileInternal(ufile);
}