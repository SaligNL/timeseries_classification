
#include "file_operations.h"

using namespace std;

int File_Operations::SearchDirectory(std::vector<wstring> &refvecFiles,
	const std::wstring        &refcstrRootDirectory,
	const std::wstring        &refcstrExtension,
	bool                     bSearchSubdirectories)
{
	std::wstring     strFilePath;             // Filepath
	std::wstring     strPattern;              // Pattern
	std::wstring     strExtension;            // Extension
	HANDLE          hFile;                   // Handle to file
	WIN32_FIND_DATA FileInformation;         // File information


	strPattern = refcstrRootDirectory + L"\\*.*";

	hFile = ::FindFirstFile((LPCWSTR)strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
						// Search subdirectory
						int iRC = SearchDirectory(refvecFiles,
							strFilePath,
							refcstrExtension,
							bSearchSubdirectories);
						if(iRC)
							return iRC;
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					strExtension = strExtension.substr(strExtension.rfind(L".") + 1);

					if(strExtension == refcstrExtension)
					{
						// Save filename
						refvecFiles.push_back(strFilePath);
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
			return dwError;
	}

	//for(int i=0; i<10; i++)
	//	std::wcout << refvecFiles[i]<<std::endl;

	return 0;
}