#include "FileLoader.h"
#include "Common.h"

using namespace Game;

size_t FileLoader::FileRead(FileLoader::FilePtr file,Buffer& buffer) {
	if (!file.ptr) {
		Log("FileLoader::FileRead : fail to load file,invaild file handle\n");
		return 0;
	}

	size_t retVal;
	switch (file.mode) {
	case READ_BINARY:

		if(buffer.size == 0) buffer.resize(FileSize(file));
		retVal = fread(buffer.data, buffer.size, 1, file.ptr);
		return retVal;

	case READ_CHARACTERS:

		buffer.resize(FileSize(file) + 1);
		retVal = fread(buffer.data,buffer.size - 1,1,file.ptr);
		buffer.data[buffer.size - 1] = '\0';
		return retVal;

	default:
		Log("FileLoader::FileRead : fail to read file because the file is a write only file\n");
		return 0;
	}
}

size_t FileLoader::FileWrite(FileLoader::FilePtr file,Buffer& buffer) {
	if (!file.ptr) {
		Log("FileLoader::FileWrite : fail to write to the file invaild file handle\n");
		return 0;
	}

	size_t retVal;
	switch (file.mode) {
	case WRITE_BINARY:
		return fwrite(buffer.data,buffer.size,1,file.ptr);
	case WRITE_CHARACTERS:
		//Write everything except the tail of the string
		return fwrite(buffer.data,buffer.size - 1,1,file.ptr);
	default:
		Log("FileLoader::FileWrite : fail to read file because the file is a read only file\n");
		return 0;
	}
}

size_t FileLoader::FileSize(FileLoader::FilePtr file) {
	size_t offset = ftell(file.ptr);

	fseek(file.ptr,0,SEEK_END);
	size_t size = ftell(file.ptr);
	fseek(file.ptr,offset,SEEK_SET);

	return size;
}

int FileLoader::FileSeek(FileLoader::FilePtr file,int offset,FILE_SEEK_BASE base) {
	return fseek(file.ptr,offset,base);
}

void FileLoader::AddFileSearchPath(const char* filepath) {
	for (std::string& path : mSearchPaths) {
		if (path == filepath) {
			return;
		}
	}

	mSearchPaths.push_back(std::string(filepath));
}

bool FileLoader::RemoveFileSearchPath(const char* filepath) {

	for (auto iter = mSearchPaths.begin();
		iter != mSearchPaths.end(); iter++) {
		if (*iter == filepath) {
			mSearchPaths.erase(iter);
			return true;
		}
	}

	return false;
}

FileLoader::FilePtr FileLoader::OpenFile(const char* filename,LOAD_FILE_MODE mode) {
	FilePtr file;
	file.ptr = nullptr;

	auto open_file = [](const char* fullname,LOAD_FILE_MODE mode,FilePtr* file) {
		switch (mode) {
		case READ_BINARY:
			file->ptr = fopen(fullname,"rb");
			break;
		case READ_CHARACTERS:
			file->ptr = fopen(fullname,"rb");
			break;

		case WRITE_BINARY:
			file->ptr = fopen(fullname,"wb");
			break;
		case WRITE_CHARACTERS:
			file->ptr = fopen(fullname,"wb");
			break;
		}
		file->mode = mode;
	};

	if (mode <= READ_BINARY) {
		for (std::string pathname : mSearchPaths) {
			pathname.append(filename);

			open_file(pathname.c_str(), mode, &file);
			if (file.ptr) {
				return file;
			}
		}
	}

	open_file(filename,mode,&file);

	return file;
}

bool FileLoader::initialize() {
	return true;
}

void FileLoader::tick() {
}

void FileLoader::finalize() {
	mSearchPaths.clear();
}