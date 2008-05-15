#include "fs.hxx"
#include "util.hxx"

using std::auto_ptr;
using std::string;
using std::exception;
using std::list;

FS::FS(string dbroot):dbs(dbroot){
   memset(openFiles,0,sizeof(openFiles));
}

FS::~FS(){
   for(int i=0;i<MAX_OPEN_FILES;i++)
      if(openFiles[i]!=NULL)
	 delete openFiles[i];
}

int errcode(exception& e){
   if(typeid(e)==typeid(Node::ENotFound))
      return -ENOENT;
   else if(typeid(e)==typeid(std::bad_cast))
      return -ENOTDIR;
   else if(typeid(e)==typeid(Node::EAttrNotFound))
      return -EIO;
   else if(typeid(e)==typeid(Node::ENotDir))
      return -ENOTDIR;
   else if(typeid(e)==typeid(Directory::EExists))
      return -EEXIST;
   else if(typeid(e)==typeid(Node::EAccess))
      return -EACCES;
   else{
      std::cout << e.what() << "!!" <<std::endl;
      return -EIO;
   }
}

int FS::getattr(const char* path, struct stat* st){
   memset(st, 0, sizeof(struct stat));
   try{
      checkparentaccess(dbs,path,X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,string(path));
      st->st_ino=(ino_t)n->getid();
      st->st_dev=n->getattr<dev_t>("offlinefs.dev");
      st->st_nlink=n->getattr<nlink_t>("offlinefs.nlink");
      st->st_mode=n->getattr<mode_t>("offlinefs.mode");
      st->st_uid=n->getattr<uid_t>("offlinefs.uid");
      st->st_gid=n->getattr<gid_t>("offlinefs.gid");
      st->st_size=n->getattr<off_t>("offlinefs.size");
      st->st_atime=n->getattr<time_t>("offlinefs.atime");
      st->st_mtime=n->getattr<time_t>("offlinefs.mtime");
      st->st_ctime=n->getattr<time_t>("offlinefs.ctime");
   }catch(exception& e){
      return errcode(e);
   }

  return 0;
}

int FS::readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi){ 
   try{
      checkaccess(dbs,path,X_OK|R_OK);
      auto_ptr<Node> n=Node::getnode(dbs,string(path));
      n->setattr<time_t>("offlinefs.atime",time(NULL));
      Directory& d=dynamic_cast<Directory&>(*n);
      list<string> l=d.getchildren();
      for(list<string>::iterator it=l.begin();it!=l.end();it++)
	 filler(buf,it->c_str(),NULL,0);
   }catch(exception& e){
      return errcode(e);
   }
  return 0;
}

int FS::readlink(const char* path, char* buf, size_t bufsiz){
   try{
      checkaccess(dbs,path,0);
      auto_ptr<Node> n=Node::getnode(dbs,string(path));
      Symlink& sl=dynamic_cast<Symlink&>(*n);
      Buffer target=sl.getattrv("offlinefs.symlink");
      memcpy(buf,target.data,MIN(bufsiz,target.size));
      if(target.size<bufsiz)
	 buf[target.size]=0;
      return 0;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::mknod(const char* path , mode_t mode, dev_t dev){
   try{
      checkparentaccess(dbs,path,X_OK|W_OK);
      auto_ptr<Node> n=Node::create(dbs,string(path));
      n->setattr<time_t>("offlinefs.atime",time(NULL));
      n->setattr<time_t>("offlinefs.mtime",time(NULL));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->setattr<uid_t>("offlinefs.uid",fuse_get_context()->uid);
      n->setattr<gid_t>("offlinefs.gid",fuse_get_context()->gid);
      n->setattr<mode_t>("offlinefs.mode",mode);
      if(S_ISCHR(mode)||S_ISBLK(mode))
	 n->setattr<dev_t>("offlinefs.dev",dev);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::mkdir(const char* path, mode_t mode){
   try{
      checkparentaccess(dbs,path,X_OK|W_OK);
      auto_ptr<Directory> n=Directory::create(dbs,string(path));
      n->setattr<time_t>("offlinefs.atime",time(NULL));
      n->setattr<time_t>("offlinefs.mtime",time(NULL));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->setattr<uid_t>("offlinefs.uid",fuse_get_context()->uid);
      n->setattr<gid_t>("offlinefs.gid",fuse_get_context()->gid);
      n->setattr<mode_t>("offlinefs.mode",S_IFDIR|mode);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::unlink(const char* path){
   try{
      checkparentaccess(dbs,path,X_OK|W_OK);
      auto_ptr<Node> n=Node::getnode(dbs,string(path));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      Directory::Path p(dbs,path);
      p.parent->delchild(p.leaf);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::rmdir(const char* path){
   try{
      checkparentaccess(dbs,path,X_OK|W_OK);
      Directory::Path p(dbs,path);
      auto_ptr<Node> n=p.parent->getchild(p.leaf);
      Directory& d=dynamic_cast<Directory&>(*n);
      if(d.getchildren().size()>2)
	 return -ENOTEMPTY;
      p.parent->delchild(p.leaf);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::symlink(const char* oldpath, const char* newpath){
   try{
      checkparentaccess(dbs,newpath,X_OK|W_OK);
      auto_ptr<Symlink> sl=Symlink::create(dbs,newpath);
      sl->setattr<time_t>("offlinefs.atime",time(NULL));
      sl->setattr<time_t>("offlinefs.mtime",time(NULL));
      sl->setattr<time_t>("offlinefs.ctime",time(NULL));
      sl->setattr<uid_t>("offlinefs.uid",fuse_get_context()->uid);
      sl->setattr<gid_t>("offlinefs.gid",fuse_get_context()->gid);
      sl->setattrv("offlinefs.symlink",Buffer(oldpath,strlen(oldpath)));
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}
 
int FS::rename(const char* oldpath, const char* newpath){
   int err;
   if((err=link(oldpath,newpath)))
      return err;
   if((err=unlink(oldpath)))
      return err;
   return 0;
}

int FS::link(const char* oldpath, const char* newpath){
   try{
      checkparentaccess(dbs,oldpath,R_OK|X_OK);
      checkparentaccess(dbs,newpath,W_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,oldpath);
      Directory::Path p(dbs,newpath);
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      p.parent->addchild(p.leaf,*n);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::chmod(const char* path, mode_t mode){
   try{
      checkparentaccess(dbs,path,W_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      if(fuse_get_context()->uid!=0){
	 gid_t fgid=n->getattr<gid_t>("offlinefs.gid");
	 uid_t fuid=n->getattr<uid_t>("offlinefs.uid");
	 if(fuse_get_context()->uid!=fuid)
	    return -EACCES;
	 if(fuse_get_context()->gid!=fgid && !ingroup(fuse_get_context()->uid,fgid))
	    mode&=~S_ISGID;
      }
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->setattr<mode_t>("offlinefs.mode",mode&(~S_IFMT)|n->getattr<mode_t>("offlinefs.mode")&S_IFMT);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::chown(const char* path, uid_t owner, gid_t group){
   try{
      checkparentaccess(dbs,path,W_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      if(owner!=(uid_t)-1 && owner!=n->getattr<uid_t>("offlinefs.uid")){
	 if(fuse_get_context()->uid!=0)
	    return -EACCES;
	 n->setattr<uid_t>("offlinefs.uid",owner);
      }
      if(group!=(gid_t)-1)
	 if(fuse_get_context()->uid!=0){
	    if(getgid() != group && !ingroup(getuid(),group))
	       return -EACCES;
	    else{
	       //Clear setgid bit
	       mode_t m=n->getattr<mode_t>("offlinefs.mode")&(~S_ISUID);
	       if(m&S_IXGRP)
		  m&=~S_ISGID;
	       n->setattr<mode_t>("offlinefs.mode",m);
	    }
	 }
	 n->setattr<gid_t>("offlinefs.gid",group);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::truncate(const char* path, off_t length){
   try{
      checkaccess(dbs,path,W_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      n->setattr<time_t>("offlinefs.mtime",time(NULL));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      File& f=dynamic_cast<File&>(*n);
      return f.getmedium(path)->truncate(f,length);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::open(const char* path, struct fuse_file_info* fi){
//FIXME: race condition possible
   try{
      if((fi->flags&O_ACCMODE)==O_RDONLY||(fi->flags&O_ACCMODE)==O_RDWR)
	 checkaccess(dbs,path,R_OK);
      if((fi->flags&O_ACCMODE)==O_WRONLY||(fi->flags&O_ACCMODE)==O_RDWR)
	 checkaccess(dbs,path,W_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      try{
	 File& f=dynamic_cast<File&>(*n);
	 auto_ptr<Source> s=f.getmedium(path)->getsource(f,fi->flags);
	 for(int i=0;i<MAX_OPEN_FILES;i++)
	    if(openFiles[i]==NULL){
	       openFiles[i]=s.release();
	       fi->fh=i;
	       return 0;
	    }
	 return -ENFILE;
      }catch(std::bad_cast& e){
	 return -EISDIR;
      }
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::read(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   if(nbyte>0)
      openFiles[fi->fh]->getfile().setattr<time_t>("offlinefs.atime",time(NULL));

   return openFiles[fi->fh]->read(buf,nbyte,offset);
}

int FS::write(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   if(nbyte>0)
      openFiles[fi->fh]->getfile().setattr<time_t>("offlinefs.mtime",time(NULL));
   return openFiles[fi->fh]->write(buf,nbyte,offset);
}

int FS::statfs(const char* path, struct statvfs* buf){
   try{
      Medium::Stats st=Medium::collectstats(dbs);
      memset(buf,0,sizeof(struct statvfs));
      buf->f_bsize=4096;
      buf->f_frsize=4096;
      buf->f_blocks=st.blocks;
      buf->f_bfree=st.freeblocks;
      buf->f_bavail=st.freeblocks;
      buf->f_namemax=0;
      return 0;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::flush(const char* path, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   return openFiles[fi->fh]->flush();
}

int FS::release(const char* path, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   delete openFiles[fi->fh];
   openFiles[fi->fh]=NULL;
   return 0;
}

int FS::fsync(const char* path,int datasync, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   return openFiles[fi->fh]->fsync(datasync);
}

int FS::setxattr(const char* path, const char* name, const char* value, size_t size, int flags){
   try{
      checkparentaccess(dbs,path,W_OK|X_OK);
      if(fuse_get_context()->uid!=0 && fuse_get_context()->uid!=getuid() && string(path).find("offlinefs.")==0)
	 return -EACCES;
      auto_ptr<Node> n=Node::getnode(dbs,path);
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      if(flags){
	 if(flags==XATTR_CREATE){
	    try{
	       n->getattrv(name);
	       return -EEXIST;
	    }catch(Node::EAttrNotFound& e){}
	 }else if(flags==XATTR_REPLACE){
	    n->getattrv(name);
	 }else{
	    return -EINVAL;
	 }
      }
      n->setattrv(name,Buffer(value,size));
   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;   
}

int FS::getxattr(const char* path, const char* name, char* value, size_t size){
   try{
      checkparentaccess(dbs,path,R_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      Buffer b=n->getattrv(name);
      memcpy(value,b.data,MIN(size,b.size));
      return b.size;
   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::listxattr(const char* path , char* list, size_t size){
   try{
      checkparentaccess(dbs,path,R_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      std::list<string> l=n->getattrs();
      size_t count=0;
      for(std::list<string>::iterator it=l.begin();it!=l.end();it++){
	 int namesize=it->size()+1;
	 count+=namesize;
	 if(count<=size){
	    memcpy(list,it->c_str(),namesize);
	    list+=namesize;
	 }
      }
      return count;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::removexattr(const char* path, const char* name){
   try{
      checkparentaccess(dbs,path,W_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->delattr(name);
   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::utimens(const char* path, const struct timespec tv[2]){
   try{
      checkparentaccess(dbs,path,W_OK|X_OK);
      auto_ptr<Node> n=Node::getnode(dbs,path);
      n->setattr<time_t>("offlinefs.atime",tv[0].tv_sec);
      n->setattr<time_t>("offlinefs.mtime",tv[1].tv_sec);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;   
}

int FS::access(const char* path, int mode){
   try{
      checkaccess(dbs,path,mode);
   }catch(Node::ENotFound& e){
      return -EACCES;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}
