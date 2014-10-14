
#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H


#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

class File_Operations
{
public:
	static 	int SearchDirectory(std::vector<std::wstring> &refvecFiles,
		const std::wstring        &refcstrRootDirectory,
		const std::wstring        &refcstrExtension,
		bool                     bSearchSubdirectories = true);

protected:
private:
};





#endif