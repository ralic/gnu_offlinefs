#ifndef FSDB_HXX
#define FSDB_HXX

#include "common.hxx"
#include "database.hxx"

class FsDb:public Environment{
   public:
      FsDb(std::string dbroot);
      Database<uint64_t> nodes;
      Database<uint64_t> directories;
      Database<uint32_t> media;
};
#endif
