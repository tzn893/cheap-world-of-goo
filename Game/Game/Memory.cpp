#include "Memory.h"
#include <stdlib.h>
#include <DirectXMath.h>

using namespace std;

class MemPagePool{
public:
	MemPagePool() :curr_offset(0), curr_page(nullptr) {}

	void* allocate_page();
	void release();
private:
	vector<void*> allocateded_pages;
	size_t curr_offset;
	uint8_t* curr_page;

	static const size_t pool_size = 0xffffff;
};

namespace Game {
	MemPagePool mem_page_pool;
};

void* MemPagePool::allocate_page() {

	if (curr_page == nullptr || curr_offset + Game::PageAllocator::page_size >= pool_size) {
		static int counter = 0;
		printf("%d ", counter++);
		void* new_pool = malloc(pool_size);
		curr_page = reinterpret_cast<uint8_t*>(new_pool);
		curr_offset = 0;
		allocateded_pages.push_back(new_pool);
	}
	void* new_page = curr_page + curr_offset;
	curr_offset += Game::PageAllocator::page_size;
	return new_page;
}

void MemPagePool::release() {
	for (int i = 0; i != allocateded_pages.size(); i++)
		free(allocateded_pages[i]);
	allocateded_pages.clear();
	curr_offset = 0;
	curr_page = nullptr;
}

void* Game::PageAllocator::allocate() {
	if (curr_block == nullptr) {
		uint8_t* page = reinterpret_cast<uint8_t*>(Game::mem_page_pool.allocate_page());

		uint8_t* end = page + page_size, * curr;
		Block* front;
		for (curr = page + block_size; curr < end - block_size; curr += block_size) {
			front = reinterpret_cast<Block*>(curr - block_size);
			front->next = reinterpret_cast<Block*>(curr);
		}
		front = reinterpret_cast<Block*>(curr - block_size);
		front->next = nullptr;
		curr_block = reinterpret_cast<Block*>(page);
	}
	void* new_block = curr_block;
	curr_block = curr_block->next;
	return new_block;
}

void Game::PageAllocator::deallocate(void* mem) {
	Block* new_block = reinterpret_cast<Block*>(mem);
	new_block->next = curr_block;
	curr_block = new_block;
}

const size_t Game::MemoryModule::blockSize[] = {
	//round up 8
	8,16,24,32,40,48,56,64,72,80,88,96,
	104,112,120,128,
	//round up 32
	160,192,224,256,288,320,352,384,416,
	448,480,512,
	//round up 64
	576,640,704,768,832,896,960,1024
};

//base must be the power of 2
inline size_t round_up(size_t target, size_t base) {
	return (target + base - 1) & ~(base - 1);
}


size_t Game::MemoryModule::get_block_index(size_t size) {
	if (size <= 128) return round_up(size, 8) / 8 - 1;
	else if (size <= 512) return round_up(size - 128, 32) / 32 + 15;
	else return round_up(size - 512, 64) / 64 + 27;
}

size_t Game::MemoryModule::get_block_size(size_t index) { return blockSize[index]; }

void* Game::MemoryModule::allocate(size_t size) {
	if (size <= max_block_size) {
		size_t index = get_block_index(size);
		//printf("%d ",size);
		return allocators[index].allocate();
	}
	else {
		return malloc(size);
	}
}

void Game::MemoryModule::deallocate(size_t size, void* mem) {
	if (size <= max_block_size) {
		size_t index = get_block_index(size);
		allocators[index].deallocate(mem);
	}
	else
		free(mem);
}

bool Game::MemoryModule::initialize() {
	for (int i = 0; i != _countof(blockSize); i++) {
		allocators.push_back(PageAllocator(blockSize[i]));
	}
	return true;
}

void Game::MemoryModule::tick() {

}

void Game::MemoryModule::finalize() {
	mem_page_pool.release();
}