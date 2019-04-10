#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>
#include <map>

typedef std::map<std::string, std::vector<std::string>> extensionFilesMap;

class FileUtils
{
public:

	static std::vector<std::string> getAllFilesInFolder(const std::string folder);
	static std::string getFullPath(const std::string path);
	static std::string getFilenameFromPath(const std::string path);
	static std::string getFileExtension(const std::string filename);
	static std::vector<std::string> getFoldersInPath(const std::string path);
	static void addPathSlashSeparator(std::string &path);
	static extensionFilesMap findFilesOfExtensionsInFolder(std::string rootFolder);
};

#endif
