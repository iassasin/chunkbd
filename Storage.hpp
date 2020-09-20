#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include <string>
#include <cstdint>

class Storage {
private:
	std::string basePath;
	std::string chunkPrefix;
	uint32_t chunkSize;
	uint64_t chunksCount;
public:
	Storage(std::string path, std::string _chunkPrefix, uint32_t chunkSize, uint64_t chunksCount);
	Storage(const Storage &) = delete;
	~Storage();

	uint64_t read(char *buf, uint64_t offset, uint64_t length);
	uint64_t write(const char *buf, uint64_t offset, uint64_t length);
	int flush();
};

#endif