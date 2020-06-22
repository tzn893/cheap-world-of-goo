#pragma once
#pragma warning(disable : 4996)
#include <cstdio>
#include <vector>
#include <string>
#include "IRuntimeModule.hpp"
#include "Buffer.h"

namespace Game {

	enum LOAD_FILE_MODE {
		READ_CHARACTERS = 0,
		READ_BINARY = 1,
		WRITE_BINARY = 2,
		WRITE_CHARACTERS = 3
	};

	enum FILE_SEEK_BASE {
		SEEK_BASE_SET = 0,
		SEEK_BASE_CURRENT = 1,
		SEEK_BASE_END = 2
	};

	class FileLoader : public IRuntimeModule {
	public:
		virtual bool initialize() override;
		virtual void tick() override;
		virtual void finalize() override;

		struct FilePtr { 
			FILE* ptr;
			LOAD_FILE_MODE mode;
		};

		FilePtr OpenFile(const char* filename, LOAD_FILE_MODE mode);

		//Test if a file is on the search path.If you want to open it while it exits,
		//you can pass a pointer to file paramter to get it.
		inline bool Exists(const char* filename, FilePtr* file = nullptr,
			LOAD_FILE_MODE mode = READ_BINARY) {
			FilePtr temp = OpenFile(filename,mode);
			if (file)  *file = temp;
			return temp.ptr != nullptr;
		}

		void AddFileSearchPath(const char* filepath);
		bool RemoveFileSearchPath(const char* filepath);

		size_t FileSize(FilePtr file);
		int FileSeek(FilePtr file, int offset, FILE_SEEK_BASE base);

		size_t FileRead(FilePtr file,Buffer& buffer);
		size_t FileWrite(FilePtr file,Buffer& buffer);

		inline void FileClose(FilePtr& file) {
			fclose(file.ptr);
			file.ptr = nullptr;
		}

		//open a file ,read in data according to the buffer size(0 means read in everything) from the begining and close it.
		inline bool FileReadAndClose(const char* filename, Buffer& buffer, LOAD_FILE_MODE mode = READ_CHARACTERS) {
			FilePtr file;
			if (!Exists(filename,&file,mode)) {
				return false;
			}
			size_t retVal = FileRead(file,buffer);
			FileClose(file);
			return retVal != 0;
		}

		//open a file,write all the data in the buffer to the file, and close it
		inline bool FileWriteAndClose(const char* filename, Buffer& buffer, LOAD_FILE_MODE mode = WRITE_BINARY) {
			FilePtr file;
			if (!Exists(filename,&file,mode)) {
				return false;
			}


			size_t retVal = FileWrite(file,buffer);
			FileClose(file);
			return retVal != 0;
		}

	private:
		std::vector<std::string>  mSearchPaths;
	};
}