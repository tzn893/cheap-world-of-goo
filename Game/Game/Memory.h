#pragma once
#include <new.h>
#include <stdint.h>
#include <vector>
#include "IRuntimeModule.hpp"

using byte = uint8_t;

namespace Game {
	class PageAllocator {
		union Block {
			Block* next;
			uint8_t raw_ptr[1];
		};

	public:
		static const size_t page_size = 0xffff;

		PageAllocator(size_t size) :block_size(size), curr_block(nullptr) { }

		void* allocate();
		void deallocate(void* mem);
	private:
		const size_t block_size;

		Block* curr_block;
	};

	//we don't do anything for oom.
	//Let the OS solve the problem!!
	class MemoryModule : public IRuntimeModule{

	public:
		template<typename T, typename ...Args>
		T* New(Args... params) {
			return new (allocate(sizeof(T)))T(params...);
		}

		template<typename T, typename ...Args>
		T* NewArray(size_t arraySize, Args... params) {
			uint8_t* target = (uint8_t*)allocate(sizeof(T) * arraySize);
			for (size_t i = 0; i != arraySize; i++) {
				new (target + i * sizeof(T))T(params...);
			}
			return reinterpret_cast<T*>(target);
		}

		template<typename T>
		void Delete(T* data) {
			data->~T();
			deallocate(sizeof(T), data);
		}

		template<typename T>
		void DeleteArray(size_t arraySize, T* data) {
			for (int i = 0; i != arraySize; i++) {
				(data + i)->~T();
			}
			deallocate(sizeof(T) * arraySize, data);
		}

		void* allocate(size_t size);
		void deallocate(size_t size, void* mem);

		virtual bool initialize() override;
		virtual void tick() override;
		virtual void finalize() override;

	private:
		size_t get_block_size(size_t index);
		size_t get_block_index(size_t size);

		std::vector<PageAllocator> allocators;
		static const size_t blockSize[];
		static const size_t max_block_size = 1024;
	};

}