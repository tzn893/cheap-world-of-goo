#pragma once
#include "Memory.h"
namespace Game {
	extern MemoryModule* gMemory;

	struct Buffer {
		Buffer():size(0),data(nullptr) {}
		Buffer(size_t size):size(size),
			data(reinterpret_cast<uint8_t*>(gMemory->allocate(size)))
		{}
		~Buffer() { if (data) release(); }

		void release() { if(data) gMemory->deallocate(size, data); data = nullptr, size = 0; }
		void resize(size_t size) { 
			if (data) release();
			data = reinterpret_cast<uint8_t*>(gMemory->allocate(size));
			this->size = size;
		}

		Buffer& assign(Buffer& buf) {
			std::swap(buf.data,data);
			std::swap(size,buf.size);
		}

		uint8_t* data;
		size_t size;
	};

	/*
	//ObjRef is not a smart pointer.We suggest only one copy of one ObjRef in the whole program
	template<typename T>
	struct ObjRef {

		template<typename ...Args>
		ObjRef(Args ...params):ptr(gMemory->New<T>(params)) {}
		
		~ObjRef() { gMemory->Delete(ptr); }

		T& operator*() { return *ptr; }
		const T& operator*() const { return *ptr; }

		ObjRef(const ObjRef& rhs) = delete;
		ObjRef& operator=(const ObjRef& rhs) = delete;

		T* ptr;
	};
	*/
}