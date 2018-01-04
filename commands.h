// $Id: commands.h,v 1.11 2016-01-14 14:45:21-08 - - $

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <unordered_map>
using namespace std;

#include "file_sys.h"
#include "util.h"

// A couple of convenient usings to avoid verbosity.

using command_fn = void (*)(inode_state& state, const wordvec& words);
using command_hash = unordered_map<string,command_fn>;

// command_error -
//    Extend runtime_error for throwing exceptions related to this 
//    program.

class command_error: public runtime_error {
   public: 
      explicit command_error (const string& what);
};

// execution functions -

void fn_cat    (inode_state& state, const wordvec& words);
void fn_cd     (inode_state& state, const wordvec& words);
void fn_echo   (inode_state& state, const wordvec& words);
void fn_exit   (inode_state& state, const wordvec& words);
void fn_ls     (inode_state& state, const wordvec& words);
void fn_lsr    (inode_state& state, const wordvec& words);
void fn_make   (inode_state& state, const wordvec& words);
void fn_mkdir  (inode_state& state, const wordvec& words);
void fn_prompt (inode_state& state, const wordvec& words);
void fn_pwd    (inode_state& state, const wordvec& words);
void fn_rm     (inode_state& state, const wordvec& words);
void fn_rmr    (inode_state& state, const wordvec& words);
void fn_ignore (inode_state& state, const wordvec& words);


command_fn find_command_fn (const string& command);

// exit_status_message -
//    Prints an exit message and returns the exit status, as recorded
//    by any of the functions.

int exit_status_message();
class ysh_exit: public exception {};

inode_ptr tree_iterator(inode_state& state, const wordvec& words);
string vector_to_string(const wordvec vec);
void display_contents(inode_ptr, string filename);
void bridge_mkdir_mkfile(inode_state& state, 
                        const wordvec& words, 
                        file_type what_type);
                        
wordvec trimm_vector(int at , wordvec vec);
void empty_file(inode_state& state);

#endif

