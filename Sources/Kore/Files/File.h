#pragma once

namespace Kore {
	class FileInputStreamData {

	};

	class InputStream {
	public:
		int read(void** data, int size);
		void* readAll();
		int size() const;
		int pos() const;
		void seek(int pos);
	};

	class FileInputStream : public InputStream {
	public:
		FileInputStreamData data;
	};

	class GenFile {
	protected:
		uint size;
	public:
		void* obj;
		
		virtual ~GenFile() { }
		virtual uint write(const void *data, uint psize) = 0;
		virtual uint read (void *data, uint psize) = 0;
		virtual void* readAll() = 0; //eventually returned data can no longer be used, when the GenFile is destroyed (when memory-mapped-file-io is used)
		virtual void flush() = 0;
		virtual void seek (uint ppos) = 0;
		virtual uint getPos() = 0;
		inline uint getSize() const { return size; }
		inline bool isOpened() const { return obj != nullptr; }

		void writeString8(const char* c);
		float readFloat();
		u32 readU32();
		s32 readS32();
		s32 readS32BE();
		u16 readU16();
		s16 readS16();
		u8 readU8();
		s8 readS8();

		static float readFloatFromLittleEndian(u8* data);
		static u32 readU32FromLittleEndian(u8* data);
		static s32 readS32FromLittleEndian(u8* data);
	};
	
	class DiskFile : public GenFile {
	public:
		enum FileMode {
			ReadMode, WriteMode, AppendMode
		};
		
		DiskFile();
		~DiskFile();
		bool open(const char* filename, FileMode mode);
		void close();
		virtual uint write(const void* data, uint psize);
		virtual uint read(void* data, uint psize);
		virtual void* readAll() override;
		void flush() override;
		virtual void seek(uint ppos);
		virtual uint getPos();

		u64 getLastWriteTimeStamp64() const; //time and date
	protected:
		FileMode mode;
#ifdef SYS_WINDOWS
		uint  pos;
		void* file;
		void* mappedFile;
#endif
#ifdef SYS_ANDROID
		uint pos;
#endif
	};

	class MemReadFile : public GenFile {
	protected:
		uint  pos;
		void* own_mem;
		
	public:
		MemReadFile(void* mem, uint size);
		MemReadFile(uint size);
		~MemReadFile();
		virtual uint write(const void* data, uint size);
		virtual uint read(void* data, uint psize);
		virtual void* readAll() override;
		void flush() override;
		virtual void seek(uint ppos);
		virtual uint getPos();
		inline void* getMemory() const { return obj; }
	};

	class MemWriteFile : public GenFile {
	protected:
		uint pos;
		uint alloc_size;
		uint block_size_;
	public:
		MemWriteFile(uint min_size, uint pblock_size);
		~MemWriteFile();
		virtual uint write(const void* data, uint psize);
		virtual uint read(void* data, uint psize);
		virtual void* readAll() override;
		void flush() override;
		virtual void seek(uint ppos);
		virtual uint getPos();
		void* getMemory() const { return obj; }
	};

#ifdef SYS_WINDOWS
	class WindowsResourceFile : public MemReadFile {
	public:
		WindowsResourceFile(unsigned int resource);
	};
#endif
}
