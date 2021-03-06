#include <argp.h>
#include <err.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <cmath>
#include "buse/buse.h"

#include "Storage.hpp"

const int BASE_BLK_SIZE = 1024;

static uint64_t parseBytesStr(const char *str) {
	return std::stoull(str); // TODO: suffix parse
}

struct Arguments {
	const char *nbdDevice;
	const char *storageDir;
	const char *chunkPrefix;
	uint32_t chunkSize;
	uint64_t size;
	bool verbose;
};

struct CallbackData {
	Arguments *args;
	Storage *storage;
};

int main(int argc, char **argv) {
	argp_option options[] = {
		{"chunk-prefix", 'p', "prefix", 0, "Set chunk file name prefix ('blk' by default)", 0},
		{"chunk-size", 'c', "size", 0, "Set chunk file size ('1m' by default)", 0},
		{"size", 's', "size", 0, "Available space for data store. If not multiplier of chunk-size or 1024 it will be shrinked ('10m' by default)", 0},
		{"verbose", 'v', nullptr, 0, "Verbose output about all operations", 0},
		{0},
	};

	argp argConfig = {
		.options = options,
		.args_doc = "nbd-device storagedir",
		.doc = "Chunked virtual block device (requires nbd kernel module enabled).\n"
				"Represents set of files in storagedir (ex. blk0, blk1, ...) as single block device.\n"
	};

	argConfig.parser = [] (int key, char *arg, struct argp_state *state) -> error_t {
		auto args = (Arguments *) state->input;

		switch (key) {
			case 'p': args->chunkPrefix = arg; break;
			case 'c': args->chunkSize = parseBytesStr(arg); break;
			case 's': args->size = parseBytesStr(arg); break;
			case 'v': args->verbose = true; break;

			case ARGP_KEY_ARG:
				switch (state->arg_num) {
					case 0: args->nbdDevice = arg; break;
					case 1: args->storageDir = arg; break;

					default:
						/* Too many arguments. */
						return ARGP_ERR_UNKNOWN;
				}
				break;

			case ARGP_KEY_END:
				if (state->arg_num < 2) {
					warnx("not enough arguments");
					argp_usage(state);
				}
				break;

			default:
				return ARGP_ERR_UNKNOWN;
		}
		return 0;
	};

	Arguments args = {
		.chunkPrefix = "blk",
		.chunkSize = 1024 * 1024,
		.size = 10 * 1024 * 1024,
		.verbose = false,
	};
	argp_parse(&argConfig, argc, argv, 0, 0, &args);

	uint32_t blocksCount = args.size / BASE_BLK_SIZE;
	uint64_t chunksCount = blocksCount * BASE_BLK_SIZE / args.chunkSize;

	Storage storage(args.storageDir, args.chunkPrefix, args.chunkSize, chunksCount);

	buse_operations aop = {
		.read = [] (void *buf, uint32_t len, uint64_t offset, void *userdata) {
			auto &cbData = *(CallbackData *) userdata;

			if (cbData.args->verbose) {
				std::cout << "Read " << offset << ", len " << len;
			}

			uint64_t rlen = cbData.storage->read((char *) buf, offset, len);

			if (cbData.args->verbose) {
				std::cout << " => " << rlen << std::endl;
			}

			return rlen == len ? 0 : EINVAL;
		},

		.write = [] (const void *buf, uint32_t len, uint64_t offset, void *userdata) {
			auto &cbData = *(CallbackData *) userdata;

			if (cbData.args->verbose) {
				std::cout << "Write " << offset << ", len " << len;
			}

			uint64_t wlen = cbData.storage->write((const char *) buf, offset, len);

			if (cbData.args->verbose) {
				std::cout << " => " << wlen << std::endl;
			}

			return wlen == len ? 0 : ENOSPC;
		},

		.disc = [] (void *userdata) {
			auto &cbData = *(CallbackData *) userdata;
			cbData.storage->flush();
		},

		.flush = [] (void *userdata) {
			auto &cbData = *(CallbackData *) userdata;
			int result = cbData.storage->flush();
			return result == 0 ? 0 : EINVAL;
		},

		.size = 0,
		.blksize = BASE_BLK_SIZE,
		.size_blocks = blocksCount,
	};

	std::cout << "Initialize device " << args.nbdDevice << " with 1K-blocks count " << blocksCount << std::endl;

	CallbackData cbData = {
		.args = &args,
		.storage = &storage,
	};

	int result = buse_main(args.nbdDevice, &aop, &cbData);

	std::cout << "Clean up" << std::endl;

	return result;
}
