// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16 :52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
   inode new_root(file_type::DIRECTORY_TYPE);
   root = make_shared<inode>(new_root);
   cwd = root;

   shared_ptr<directory> new_directory =
               dynamic_pointer_cast<directory> (new_root.contents);

   new_directory->insert_inode(".",root);
   new_directory->insert_inode("..",root);

  
}

const string& inode_state::prompt() { return prompt_; }

void inode_state::set_current_directory(inode_ptr node){ cwd = node;}

inode_ptr inode_state::get_root(){return root;}

inode_ptr inode_state::get_cwd(){return cwd;}

void inode_state::set_prompt(string& new_prompt){ prompt_ = new_prompt;}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           this_type = file_type::PLAIN_TYPE;
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           this_type = file_type::DIRECTORY_TYPE;
           contents = make_shared<directory>();
           break;
   }

   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}


int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

shared_ptr<directory> inode::get_dir_contents()const
{
  return dynamic_pointer_cast<directory> (contents); 
} //--
shared_ptr<base_file> inode::get_dir_contents_base()const
{
  return dynamic_pointer_cast<base_file> (contents); 
}//--

file_type inode::get_type()const{return this_type;}


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   
   size_t word_length{0};
   string word_container;

   for(wordvec::const_iterator it = data.begin(); 
       it != data.end(); ++it) //--
   {
       word_container = *it;
       word_length += word_container.length();
   }

   size = word_length + data.size()  - 
       ( ( ( data.size() > 0 ) == true) ? 1 : 0 ); //--


   return size;
   
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::clear_vector(){
    data.clear();
}


size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   
   size = dirents.size();
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);

   if(directory_exists(filename) == true && 
       filename != ".." && filename != ".") //--
   {
      inode_ptr this_file = get_inode(filename);
      if(this_file->get_dir_contents()->size() == 2)
      {
         this_file->get_dir_contents()->clear_dirents();  
         dirents.erase(filename);
      }
      else cerr<< "directory not empty"<< endl;      
   }
   else if(file_exists(filename) == true)
       {
          dirents.erase(filename);
       }
   else  cerr<<"not found"<<endl;
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode dir(file_type::DIRECTORY_TYPE);
   auto newDir = make_shared<inode>(dir);
   auto thisDir = dirents.find(".");
   dirents.insert(pair<string,inode_ptr>(dirname,newDir));
   
   shared_ptr<directory> new_directory =
               dynamic_pointer_cast<directory> (newDir->contents);
   
   new_directory->insert_inode(".",newDir);
   new_directory->insert_inode("..",thisDir->second);
   
   return newDir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode dir(file_type::PLAIN_TYPE);
   auto newDir = make_shared<inode>(dir);
   dirents.insert(pair<string,inode_ptr>(filename,newDir));
   return newDir;
}

void directory::insert_inode(string name, inode_ptr node)
{
    dirents.insert(pair<string,inode_ptr>(name,node));
}

inode_ptr directory::get_inode(string name)
{
    return dirents.find(name)->second;
}

bool directory::directory_exists(const string name)const
{
    for(map<string,inode_ptr>::const_iterator it = dirents.begin(); 
        it != dirents.end(); ++it) //--
    {
        if(it->first == name && 
           it->second->this_type == file_type::DIRECTORY_TYPE)  //--
           return true;
            
    }
    return false;
}

bool directory::file_exists(const string name)const
{
    for(map<string,inode_ptr>::const_iterator it = dirents.begin(); 
        it != dirents.end(); ++it)//--
    {
        if(it->first == name && 
          it->second->this_type == file_type::PLAIN_TYPE) //--
           return true;
            
    }
    return false;
}

void directory::print_map()const
{
   int serial_number{0};
   size_t file_size{0};
   string filename;

   for(map<string,inode_ptr>::const_iterator it = dirents.begin(); 
       it != dirents.end(); ++it) //--
   {
       filename = it->first;
       serial_number = it->second->get_inode_nr();
       if(it->second->get_type() == file_type::DIRECTORY_TYPE)
        file_size = it->second->get_dir_contents()->size();
       else
        file_size =  it->second->get_dir_contents_base()->size();
 
       cout << std::setw(6);
       cout << serial_number <<"  " ;
       cout << std::setw(6);
       cout << file_size <<"  ";
       cout << filename;
       if(it->second->get_type() == file_type::DIRECTORY_TYPE
          && it->first != ".."
          && it->first != ".")
          cout <<"/";

       cout<<endl;
   }    
}

string directory::get_name(inode_ptr node)
{
     for(map<string,inode_ptr>::iterator it = dirents.begin(); 
         it!= dirents.end(); ++it) //--
     {
         if(it->second == node)
             return it->first;    
     }

     return "null";
}

inode_ptr directory::get_next_directory()
{
    if(iterator_set == false)
      {
       iterator_set = true;
       map_iterator = dirents.begin();
      }

    while(map_iterator != dirents.end())
     {
        if(map_iterator->second->get_type() != 
           file_type::DIRECTORY_TYPE) //--
           ++map_iterator;
        else if (map_iterator->first == "." || 
                 map_iterator->first == "..") //--
           ++map_iterator;
        else 
        {
           map<string,inode_ptr>::iterator current_iterator = 
                                   map_iterator; //--
           ++map_iterator; 
           return current_iterator->second;
        }
     }
     
     iterator_set = false;
     return nullptr;
}

inode_ptr directory::get_next_file()
{
    if(iterator_set_files == false)
      {
       iterator_set_files = true;
       map_iterator = dirents.begin();
      }

    while(map_iterator != dirents.end())
     { 
        if (map_iterator->first == "." || map_iterator->first == "..")
           ++map_iterator;
        else 
        {
            map<string,inode_ptr>::iterator current_iterator = 
                                   map_iterator; //--
           ++map_iterator; 
           return current_iterator->second;
        }
     }
     
     iterator_set_files = false;
     return nullptr;

}

void directory::clear_dirents(){ dirents.clear(); }

void directory::remove_root()
{ 
    inode_ptr this_root= get_inode("..");
    this_root->get_dir_contents()->clear_dirents(); 
}

