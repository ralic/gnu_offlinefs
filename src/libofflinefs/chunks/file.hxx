//     Copyright (C) 2008 Francisco Jerez
//     This file is part of offlinefs.

//     offlinefs is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     offlinefs is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CHUNK_FILE_HXX
#define CHUNK_FILE_HXX

#include "chunk.hxx"

// Implementation that stores the chunk inside a file of the
// underlying filesystem
class Chunk_file:public Chunk{
      int fd;
   public:
      Chunk_file(std::string path,int mode);
      virtual ~Chunk_file();

      virtual int read(char* buf, size_t nbyte, off_t offset);
      virtual int write(const char* buf, size_t nbyte, off_t offset);
      virtual int flush();
      virtual int fsync(int datasync);
      virtual int ftruncate(off_t length);
};

#endif
