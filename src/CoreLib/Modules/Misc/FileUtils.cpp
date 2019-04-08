#include <stdio.h>
#include <Windows.h>

#include <iostream>

#include "FileUtils.h"


std::vector<std::string> FileUtils::getAllFilesInFolder(const std::string folder)
{
	std::vector<std::string> names;
	std::string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

std::string FileUtils::getFullPath(const std::string path)
{
	const int buffsize = 4096;
	TCHAR  buff[buffsize] = TEXT("");
	GetFullPathName(path.c_str(), buffsize, buff, NULL);
	return std::string(buff);
}

std::string FileUtils::getFilenameFromPath(const std::string path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

// Does not recurse yet beyond getting the subfolders in first level, will fix later
std::vector<std::string> FileUtils::getFoldersInPath(const std::string path)
{
	std::vector<std::string> names;
	std::string search_path = getFullPath(path) + "*.*";

	names.push_back(path);

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::string dir = fd.cFileName;
				if (dir != "." && dir != "..") {
					dir = path + dir;
					addPathSlashSeparator(dir);
					names.push_back(dir);
				}
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

void FileUtils::addPathSlashSeparator(std::string &path) {
	if (!(path.find_last_of("/\\") == path.length() - 1)) {
		path += '/';
	}
}

std::string FileUtils::getFileExtension(const std::string filename)
{
	std::string extension;
	auto pos = filename.find_last_of('.');
	if (pos != std::string::npos) {
		extension = filename.substr(pos + 1);
	}
	return extension;
}

// could provide vector of extensions searching in the future
extensionFilesMap FileUtils::findFilesOfExtensionsInFolder(std::string rootFolder) {
	extensionFilesMap extensionToFilesMap;

	std::vector<std::string> folders = FileUtils::getFoldersInPath(rootFolder);
	for (auto folder : folders) {
		auto allFiles = FileUtils::getAllFilesInFolder(folder);
		for (auto file : allFiles) {
			extensionToFilesMap[FileUtils::getFileExtension(file)].emplace_back(folder + file);
		}
	}
	return extensionToFilesMap;
}
