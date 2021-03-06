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

#include "database.hxx"
#include <asm/byteorder.h>
#include "types.hxx"

Environment::Environment(std::string path):opcount(0){
   struct stat st;
   if(stat(path.c_str(),&st) &&  errno==ENOENT)
      mkdir(path.c_str(),0777);

   dbenv=new DbEnv(0);
   dbenv->set_flags(DB_TXN_NOSYNC,1);

   dbenv->open(path.c_str(),DB_INIT_LOCK|DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_CREATE|DB_RECOVER|DB_REGISTER|DB_THREAD,0);

#if  DB_VERSION_MAJOR==4 && DB_VERSION_MINOR>=7
   dbenv->log_set_config(DB_LOG_AUTO_REMOVE,1);
#else
   dbenv->set_flags(DB_LOG_AUTOREMOVE,1);
#endif
   dbenv->set_lk_detect(DB_LOCK_RANDOM);

   cleanlogs();
}

void Environment::cleanlogs(){
   if(!opcount--){
      opcount=100000;
      dbenv->txn_checkpoint(0,0,0);
   }
}
Environment::~Environment(){
   if(dbenv){
      dbenv->close(0);
      delete dbenv;
   }
}

void Buffer::clean(){
   if(data){
      delete[] data;
      data=NULL;
      size=0;
   }
}

void Buffer::copy(const char* data, size_t size){
   this->data=new char[size];
   this->size=size;
   memcpy(this->data,data,size);
}

// Endianness conversion functions
template<>
uint8_t host_to_db<uint8_t>(uint8_t v){ return v; }

template<>
uint8_t db_to_host<uint8_t>(uint8_t v){ return v; }

template<>
uint16_t host_to_db<uint16_t>(uint16_t v){ return __cpu_to_be16(v); }

template<>
uint16_t db_to_host<uint16_t>(uint16_t v){ return __be16_to_cpu(v); }

template<>
uint32_t host_to_db<uint32_t>(uint32_t v){ return __cpu_to_be32(v); }

template<>
uint32_t db_to_host<uint32_t>(uint32_t v){ return __be32_to_cpu(v); }

template<>
uint64_t host_to_db<uint64_t>(uint64_t v){ return __cpu_to_be64(v); }

template<>
uint64_t db_to_host<uint64_t>(uint64_t v){ return __be64_to_cpu(v); }

