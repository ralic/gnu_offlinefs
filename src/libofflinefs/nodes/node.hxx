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

#ifndef NODES_NODE_HXX
#define NODES_NODE_HXX

#include <common.hxx>
#include <fsdb.hxx>
#include <scontext.hxx>
#include <fsdb.hxx>

class Node:public Database<uint64_t>::Register{
   public:
      template<typename T>
      class EBadCast:public std::runtime_error{
	 public:
	    EBadCast():runtime_error(std::string("Node::EBadCast<")+typeid(T).name()+">") {}
      };
      class ENotFound:public std::runtime_error{
	 public:
	    ENotFound();
      };
      class EAccess:public std::runtime_error{
	 public:
	    EAccess();
      };

      FsTxn& txns;

      Node(FsTxn& txns,uint64_t id);
      virtual ~Node();

      //Initialize a "neutral" node (with its mode attribute = 0)
      static std::auto_ptr<Node> create(FsTxn& txns);

      // Instance a Node derived object, depending on the type stored in the database
      // It throws ENotFound if the node doesn't exist
      static std::auto_ptr<Node> getnode(FsTxn& txns, uint64_t id);

      // Clone a node.
      static std::auto_ptr<Node> getnode(const Node* n){
	 return getnode(n->txns,n->getid());
      }

      template<typename T>
      static std::auto_ptr<T> cast(std::auto_ptr<Node> n){
	 if(typeid(*n)!=typeid(T))
	    throw EBadCast<T>();
	 return std::auto_ptr<T>((T*)n.release());
      }

      // Increment link count
      void link();
      // Decrement link count and remove it if it has reached zero
      void unlink();

      // Throw EAccess if the node doesn't have all the specified permissions
      // for the caller
      void access(const SContext& sctx, int mode);
};

#endif
