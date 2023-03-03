#pragma once

#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sstream>
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif

static std::string GetExecDir()
{
	static std::string ret;

	if (ret.size() > 0)
	{
		return ret;
	}

	unsigned int bufferSize = 512;
	std::vector<char> buffer(bufferSize + 1);

#ifdef _WIN32
	::GetModuleFileName(NULL, &buffer[0], bufferSize);
#elif __linux__
	// Get the process ID.
	int pid = getpid();

	// Construct a path to the symbolic link pointing to the process executable.
	// This is at /proc/<pid>/exe on Linux systems (we hope).
	std::ostringstream oss;
	oss << "/proc/" << pid << "/exe";
	std::string link = oss.str();

	// Read the contents of the link.
	int count = readlink(link.c_str(), &buffer[0], bufferSize);
	if (count == -1) throw std::runtime_error("Could not read symbolic link");
	buffer[count] = '\0';
#elif __APPLE__
	if (_NSGetExecutablePath(&buffer[0], &bufferSize))
	{
		buffer.resize(bufferSize);
		_NSGetExecutablePath(&buffer[0], &bufferSize);
	}
#endif

	std::filesystem::path p = &buffer[0];
	p.remove_filename();
	ret = p.generic_string();

	return ret;
}

static std::string GetResDir()
{
	static std::string ret;
	if (ret.size() > 0)
	{
		return ret;
	}
	auto ProjectDir = GetExecDir();
	ret = ProjectDir.append("res/");
	return ret;
}

static std::string ResFullPath(const std::string& InFilePath)
{
	auto ResDir = GetResDir();
	return ResDir.append(InFilePath);
}

static std::string GetScriptDir()
{
	static std::string ret;
	if (ret.size() > 0)
	{
		return ret;
	}
	auto ProjectDir = GetExecDir();
	ret = ProjectDir.append("res/script/");
	return ret;
}

static std::string ScriptFullPath(const std::string& InFilePath)
{
	auto ResDir = GetScriptDir();
	return ResDir.append(InFilePath);
}
