#include "Storage.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>

Storage::Storage(std::string path, std::string _chunkPrefix, uint32_t _chunkSize, uint64_t _chunksCount)
	: basePath(path), chunkPrefix(_chunkPrefix), chunkSize(_chunkSize), chunksCount(_chunksCount)
{}

Storage::~Storage() {
	flush();
}

uint64_t Storage::read(char *buf, uint64_t offset, uint64_t length) {
	if (offset + length > chunkSize * chunksCount) {
		return 0;
	}

	uint32_t bufPos = 0;
	uint64_t pos = offset;
	uint64_t len = length;

	while (len > 0) {
		uint64_t curChunk = pos / chunkSize;
		uint32_t chunkPos = pos % chunkSize;
		uint32_t readLen = std::min(len, (uint64_t) chunkSize - chunkPos);

		std::ifstream chunk(basePath + "/" + chunkPrefix + std::to_string(curChunk), std::ios::in | std::ios::binary);

		if (!chunk) {
			return bufPos > 0 ? bufPos : length;
		}

		if (!(chunk.seekg(chunkPos) && chunk.read(buf + bufPos, readLen))) {
			return bufPos;
		}

		bufPos += readLen;
		len -= readLen;
		pos += readLen;
	}

	return bufPos;
}

uint64_t Storage::write(const char *buf, uint64_t offset, uint64_t length) {
	if (offset + length > chunkSize * chunksCount) {
		return 0;
	}

	uint32_t bufPos = 0;
	uint64_t pos = offset;
	uint64_t len = length;

	while (len > 0) {
		uint64_t curChunk = pos / chunkSize;
		uint32_t chunkPos = pos % chunkSize;
		uint32_t writeLen = std::min(len, (uint64_t) chunkSize - chunkPos);

		std::string fpath = basePath + "/" + chunkPrefix + std::to_string(curChunk);

		if (!std::filesystem::exists(fpath)) {
			std::ofstream file(fpath);
		}

		std::fstream chunk(fpath, std::ios::in | std::ios::out | std::ios::binary);
		if (!(chunk && chunk.seekp(chunkPos) && chunk.write(buf + bufPos, writeLen))) {
			return bufPos;
		}

		bufPos += writeLen;
		len -= writeLen;
		pos += writeLen;
	}

	return bufPos;
}

int Storage::flush() {
	return 0;
}