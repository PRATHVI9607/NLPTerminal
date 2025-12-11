#ifndef COMMANDS_H
#define COMMANDS_H

void do_ls(char **args);
void do_pwd(char **args);
void do_mkdir(char **args);
void do_rmdir(char **args);
void do_rm(char **args);
void do_touch(char **args);
void do_cat(char **args);
void do_cp(char **args);
void do_mv(char **args);
void do_echo(char **args);

// Custom commands
void do_tree(char **args);
void do_search(char **args);
void do_backup(char **args);
void do_compare(char **args);
void do_stats(char **args);
void do_sysmon(char **args);
void do_bookmark(char **args);
void do_recent(char **args);
void do_bulk_rename(char **args);

#endif

