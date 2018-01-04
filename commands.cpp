// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"

bool trigger_dot = false;
bool supress_cout = false;

inode_ptr tree_iterator(inode_state& state, 
                        const wordvec& words, file_type what_type) 
{
   inode_ptr ptr_to_insert = state.get_cwd();
   string delimiter = "/";
   wordvec parsed_words = split(words[1], delimiter);
   if(words[1][0] == '/') ptr_to_insert = state.get_root();
   for(wordvec::iterator it = parsed_words.begin(); 
       it != parsed_words.end(); ++it) //--
   {
       if(it+1 != parsed_words.end())
       {
          if(ptr_to_insert->get_dir_contents()->
             directory_exists(*it) == true) //--
            ptr_to_insert = ptr_to_insert->get_dir_contents()->
                            get_inode(*it); //--
          else 
          {
             cerr<<words[0]<< ": " << *it << 
                  " not such a file or directory" << endl; //--
             return nullptr;
          }
       }
       else
       {
           if(what_type == file_type::DIRECTORY_TYPE)
            {
               if(ptr_to_insert->get_dir_contents()->
                  directory_exists(*it) == true) //--
                  ptr_to_insert = ptr_to_insert->
                                  get_dir_contents()->
                                  get_inode(*it); //---
               else 
                {
                    if(supress_cout == false)
                        cerr<<words[0]<< ": " << *it << 
                            " not such a file or directory" 
                             << endl; //---
                  supress_cout = false;
                  return nullptr;
                }
             }
            else if(what_type == file_type::PLAIN_TYPE)
             {
                 if(ptr_to_insert->get_dir_contents()->
                    file_exists(*it) == true) //--
                     ptr_to_insert = ptr_to_insert->
                                     get_dir_contents()->
                                     get_inode(*it); //---
                 else 
                 {
                    if(supress_cout == false)
                         cerr<<words[0]<< ": " << *it << 
                         ": Not such a file or directory" 
                         << endl; //---

                    supress_cout = false;
                    return nullptr;
                 }
             }
       }
   }
   return ptr_to_insert; 
}

string vector_to_string(const wordvec vec)
{
    string container;
    for(wordvec::const_iterator it = vec.begin(); it != vec.end(); ++it)
        container = container + "/" + *it;
    
    return container;
}

void display_contents(inode_ptr dad,string filename)
{
    inode_ptr backup{nullptr};
    if(dad->get_type() == file_type::DIRECTORY_TYPE)
        dad->get_dir_contents()->print_map();
    else if(dad->get_type() == file_type::PLAIN_TYPE)
        cout << filename << endl << endl; 
}

wordvec trimm_vector(int at , wordvec vec)
{
    wordvec result;

    for(wordvec::iterator it = vec.begin() + at; it != vec.end(); ++it)
        result.push_back(*it);

    return result;
}

void bridge_mkdir_mkfile(inode_state& state, 
                        const wordvec& words, file_type what_type) //--
{
   string delimiter = "/";
   
   if(words.size() > 1)
   {
    wordvec split_vec = split(words[1],delimiter);
    string backup_name = split_vec.back();
    split_vec.pop_back();
    string removal_string = vector_to_string(split_vec);
    wordvec new_words{words[0],removal_string};
    inode_ptr tree_it = tree_iterator(state,new_words,
                                      file_type::DIRECTORY_TYPE); //--

    if(tree_it != nullptr)
    {
       bool possible_file_duplicate = tree_it->get_dir_contents()->
                                      file_exists(backup_name); //--
       bool possible_directory_duplicate = tree_it->get_dir_contents()->
                             directory_exists(backup_name); //---
  
       if(possible_file_duplicate == false &&
           possible_directory_duplicate == false)
       {
           if(what_type == file_type::DIRECTORY_TYPE)
              tree_it->get_dir_contents()->mkdir(backup_name);
           else
           {
              wordvec input_data{};
              if(words.size() > 2)
                 input_data = trimm_vector(2,words);

              inode_ptr new_file = tree_it->
                                   get_dir_contents_base()->
                                   mkfile(backup_name); //---

              new_file->get_dir_contents_base()->writefile(input_data);
           }
       }
       else
       {
          cerr << ((what_type == file_type::DIRECTORY_TYPE) ? 
                                           "mkdir" : "make")  //--
             << "cannot create directory"<<"'"<<backup_name
             << "' : file exists" << endl; //--
       }
     }
   }


   else
       cerr<<"error: missing operand"<< endl;
}

void empty_file(inode_state& state)
{
    inode_ptr current_ptr{nullptr};
    inode_ptr cwd = state.get_cwd();
    if(cwd->get_dir_contents()->size() == 2)
       return;
    else
    {
        while((current_ptr = cwd->get_dir_contents()->
               get_next_file()) != nullptr) //--
        {
             string filename = cwd->get_dir_contents()->
                               get_name(current_ptr);  //--
             wordvec util_vec{"rm",filename};
             if(current_ptr->get_type() == file_type::PLAIN_TYPE)
                 fn_rm(state,util_vec);
             else if(current_ptr->get_type() == 
                     file_type::DIRECTORY_TYPE) //--
             {
                 state.set_current_directory(current_ptr);
                 empty_file(state);
                 state.set_current_directory(cwd);
                 fn_rm(state,util_vec);
             }

        }
    }
}



command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
   {"#"    ,  fn_ignore   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
  DEBUGF ('c', state);
  DEBUGF ('c', words);
 
 for(wordvec::const_iterator it = words.begin() + 1; 
     it != words.end(); ++it )
 {
    wordvec paths{"cat",*it};
   inode_ptr get_file = tree_iterator(state,paths,
                                     file_type::PLAIN_TYPE); //--

   if(get_file != nullptr)
   {
      wordvec file_to_print = get_file->
                             get_dir_contents_base()->readfile(); //--

      for(wordvec::iterator it =  file_to_print.begin(); 
          it!= file_to_print.end(); ++it)//--
       {
         cout<<*it<<" ";
       }
      cout<<endl;
    }
 }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if(words.size() > 1)
   {
       
       inode_ptr tree_it = tree_iterator(state,words,
                           file_type::DIRECTORY_TYPE); //--
       if(tree_it != nullptr)
           state.set_current_directory(tree_it);   
   }
   else
      state.set_current_directory(state.get_root());
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec root_path{"rmr","/"};
   fn_rmr(state,root_path);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
    
   DEBUGF ('c', state);
   DEBUGF ('c', words);
    
   inode_ptr current{nullptr};
   wordvec words_cut; 
   wordvec for_last_word;
   string delimiter = "/";
   string last_word{};
   if(words.size() > 1)
   {
      for(wordvec::const_iterator it = words.begin() + 1; 
          it != words.end(); ++it) //---
       {
          words_cut = {words[0], *it};
          for_last_word = split(*it,delimiter);
          if(for_last_word.size() != 0)
               last_word = for_last_word.back();
          supress_cout = true;
          current = tree_iterator(state, words_cut,
                    file_type::DIRECTORY_TYPE); //--

          if(current != nullptr)
          {
              cout << *it <<":"<<endl;
              display_contents(current,last_word);
          }
          else
          {
              current = tree_iterator(state, words_cut,
                        file_type::PLAIN_TYPE); //--

              if(current != nullptr)
              {
                cout << *it <<":"<<endl;
                display_contents(current,last_word);
              }else 
                cerr << "ls: cannot access"<< *it 
                     <<": No such file or directory" 
                     << endl << endl; //---

          }
            
       }    
   }
   else
     {
       string mock = "..";
       trigger_dot = true;
       fn_pwd(state,words);
       trigger_dot = false;
       display_contents(state.get_cwd(),mock);
     }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   inode_ptr cwd_backup = state.get_cwd();
   inode_ptr cwd = state.get_cwd();
   inode_ptr current_dir{nullptr};
   wordvec empty_vec{};
   if(words.size() > 1) cwd = tree_iterator(state,words,
                              file_type::DIRECTORY_TYPE); //--

   if(cwd != nullptr)
   {
      fn_ls(state,words);

      while((current_dir = cwd->get_dir_contents()->
             get_next_directory() ) != nullptr) //--
       {
          state.set_current_directory(current_dir);
          fn_lsr(state,empty_vec);
       }
   }

   state.set_current_directory(cwd_backup);

   return;
   
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   bridge_mkdir_mkfile(state,words,file_type::PLAIN_TYPE);


}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
 
   bridge_mkdir_mkfile(state,words,file_type::DIRECTORY_TYPE);
   
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string container{};

   for(wordvec::const_iterator it  = words.begin() + 1; 
       it != words.end(); ++it) //--
      container = container + *it + " ";

    state.set_prompt(container);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string pwd_container;
   inode_ptr _cwd = state.get_cwd();
   inode_ptr backup{nullptr};

   if(_cwd == state.get_root())
       pwd_container = "/" + pwd_container;
   else{
     while(_cwd != state.get_root())
     {
         backup = _cwd->get_dir_contents()->get_inode(".");
         _cwd = _cwd->get_dir_contents()->get_inode("..");
         pwd_container = "/" +  
                         _cwd->get_dir_contents()->get_name(backup) + 
                         pwd_container; //--
     }
   }
   
   cout<<pwd_container << ( (trigger_dot == true) ? ":" : "" )<< endl;

   
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() > 1)
   {
      supress_cout = true;
      inode_ptr file_to_remove = tree_iterator(state,words,
                                file_type::DIRECTORY_TYPE); //---
      inode_ptr file_parent{nullptr};
      
      string remove_filename{};
      string delimiter = "/";
      wordvec for_last_word = split(words[1],delimiter);

      if(for_last_word.size() != 0)
      {
         remove_filename = for_last_word.back();
         for_last_word.pop_back();
      }

      string words_cut = vector_to_string(for_last_word); //
      
      if(file_to_remove != nullptr)
      {
           file_parent = file_to_remove->
                         get_dir_contents()->get_inode(".."); //--
           if(file_parent != file_to_remove)  //
               file_parent->get_dir_contents()->remove(remove_filename);
            else
                file_parent->get_dir_contents()->remove_root();
      }
      else
      {
           file_to_remove = tree_iterator(state,words,
                               file_type::PLAIN_TYPE);  //--
           if(file_to_remove != nullptr)
           {
              wordvec path_cut = {words[0],words_cut};
              file_parent = tree_iterator(state,path_cut,
                            file_type::DIRECTORY_TYPE); //--
              file_parent->get_dir_contents()->remove(remove_filename);
           }
      }
   }
   else cerr<<"missing operand"<<endl;     
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   inode_ptr file{nullptr};
   inode_ptr dir{nullptr};

   if(words.size() > 1)
   {
       supress_cout = true;
       file = tree_iterator(state,words,file_type::PLAIN_TYPE);
       dir = tree_iterator(state,words,file_type::DIRECTORY_TYPE);

       if(file != nullptr && file != state.get_root())
         fn_rm(state,words);
       if(dir != nullptr)
       {
          state.set_current_directory(dir);
          empty_file(state);
          inode_ptr parent = state.get_cwd()->
                             get_dir_contents()->get_inode(".."); //--
          state.set_current_directory(parent);
          fn_rm(state,words);
       }
   }
   else cerr << "missing operands" << endl;

   return;
   
}

void fn_ignore(inode_state& state, const wordvec& words)
{
    return;
}






