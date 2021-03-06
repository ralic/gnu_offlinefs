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

#include "directory.hxx"
#include "chunks/file.hxx"

using std::auto_ptr;
using std::string;

auto_ptr<Medium_directory> Medium_directory::create(FsTxn& txns){
   auto_ptr<Medium_directory> m(new Medium_directory(txns,txns.dbs.media.createregister(txns.media)));
   m->setattrv("label",Buffer(""));
   m->setattrv("type",Buffer("directory"));
   m->setattrv("directory",Buffer(""));
   m->setattrv("unlink_files",Buffer("false"));
   return m;
}

auto_ptr<Chunk> Medium_directory::getchunk(string phid,int mode){
   return auto_ptr<Chunk>(new Chunk_file(string(getattrv("directory")) + "/" + phid,mode));
}

Medium::Stats Medium_directory::getstats(){
   struct statvfs st;
   string directory = getattrv("directory"); 
   if(statvfs(directory.c_str(),&st))
      throw std::runtime_error("Medium_directory::getstats: error calling statvfs.");
   Stats st_;
   st_.total=st.f_blocks*st.f_frsize;
   st_.free=st.f_bfree*st.f_frsize;
   return st_;
}

void Medium_directory::addfile(string phid){}

void Medium_directory::delfile(string phid){
   if(string(getattrv("unlink_files")) == "true"){
      string base_dir = getattrv("directory");
      string path = base_dir + "/" + phid;

      unlink(path.c_str());

      // Remove each empty parent directory
      string::size_type pos = string::npos;
      struct stat st;

      while(true){
	 // Get next token
	 if((pos = phid.find_last_of("/",pos)) == string::npos)
	    break;

	 if((pos = phid.find_last_not_of("/", pos)) == string::npos)
	    break;

	 string dir = base_dir + "/" + phid.substr(0,pos + 1);

	 // Check whether it is empty
	 if(stat(dir.c_str(), &st))
	    throw std::runtime_error("Medium_directory::Medium_directory: stat() failed: " +
				     string(strerror(errno)));

	 if(st.st_nlink > 2)
	    break;

	 // Remove it
	 if(rmdir(dir.c_str()))
	    throw std::runtime_error("Medium_directory::Medium_directory: rmdir() failed: " +
				     string(strerror(errno)));
      }
   }
}
