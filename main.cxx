#include "common.hxx"
#include "fs.hxx"

void* init_(fuse_conn_info* conn){
   return new FS();
}

void destroy_(void *userdata){
   delete (FS*)fuse_get_context()->private_data;
}

int getattr_(const char* path, struct stat* st){
  return ((FS*)fuse_get_context()->private_data)->getattr(path,st);
}

int readdir_(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi){
  return ((FS*)fuse_get_context()->private_data)->readdir(path,buf,filler,offset,fi);
}

int readlink_(const char* path, char* buf, size_t bufsiz){
   return ((FS*)fuse_get_context()->private_data)->readlink(path,buf,bufsiz);
}

int mknod_(const char* path , mode_t mode, dev_t dev){
   return ((FS*)fuse_get_context()->private_data)->mknod(path,mode,dev);
}

int mkdir_(const char* path, mode_t mode){
   return ((FS*)fuse_get_context()->private_data)->mkdir(path,mode);
}

int unlink_(const char* path){
   return ((FS*)fuse_get_context()->private_data)->unlink(path);
}

int rmdir_(const char* path){
   return ((FS*)fuse_get_context()->private_data)->rmdir(path);
}

int symlink_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->symlink(oldpath,newpath);
}
 
int rename_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->rename(oldpath,newpath);
}

int link_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->link(oldpath,newpath);
}

int chmod_(const char* path, mode_t mode){
   return ((FS*)fuse_get_context()->private_data)->chmod(path,mode);
}

int chown_(const char* path, uid_t owner, gid_t group){
   return ((FS*)fuse_get_context()->private_data)->chown(path,owner,group);
}

int truncate_(const char* path, off_t length){
   return ((FS*)fuse_get_context()->private_data)->truncate(path,length);
}

int open_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->open(path,fi);
}

int read_(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->read(path,buf,nbyte,offset,fi);
}

int write_(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->write(path,buf,nbyte,offset,fi);
}

int statfs_(const char* path, struct statvfs* buf){
   return ((FS*)fuse_get_context()->private_data)->statfs(path,buf);
}

int flush_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->flush(path,fi);
}

int release_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->release(path,fi);
}

int fsync_(const char* path, int datasync, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->fsync(path,datasync,fi);
}

int setxattr_(const char* path, const char* name, const char* value, size_t size, int flags){
   return ((FS*)fuse_get_context()->private_data)->setxattr(path,name,value,size,flags);
}

int getxattr_(const char* path, const char* name, char* value, size_t size){
   return ((FS*)fuse_get_context()->private_data)->getxattr(path,name,value,size);
}

int listxattr_(const char* path , char* list, size_t size){
   return ((FS*)fuse_get_context()->private_data)->listxattr(path,list,size);
}

int removexattr_(const char* path, const char* name){
   return ((FS*)fuse_get_context()->private_data)->removexattr(path,name);
}

int utimens_(const char* path, const struct timespec tv[2]){
   return ((FS*)fuse_get_context()->private_data)->utimens(path,tv);
}

int access_(const char* path, int mode){
   return ((FS*)fuse_get_context()->private_data)->access(path,mode);
}

int main(int argc, char** argv){
  fuse_operations ops;

  memset(&ops,0,sizeof(fuse_operations));
  ops.init=init_;
  ops.destroy=destroy_;
  ops.getattr=getattr_;
  ops.readdir=readdir_;
  ops.readlink=readlink_;
  ops.mknod=mknod_;
  ops.mkdir=mkdir_;
  ops.unlink=unlink_;
  ops.rmdir=rmdir_;
  ops.symlink=symlink_;
  ops.rename=rename_;
  ops.link=link_;
  ops.chmod=chmod_;
  ops.chown=chown_;
  ops.truncate=truncate_;
  ops.open=open_;
  ops.read=read_;
  ops.write=write_;
  ops.statfs=statfs_;
  ops.flush=flush_;
  ops.release=release_;
  ops.fsync=fsync_;
  ops.setxattr=setxattr_;
  ops.getxattr=getxattr_;
  ops.listxattr=listxattr_;
  ops.removexattr=removexattr_;
  ops.utimens=utimens_;
  ops.access=access_;

  return fuse_main(argc, argv,&ops,NULL);
}
