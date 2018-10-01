#ifndef SCAN_ROUTINES_H
#define SCAN_ROUTINES_H

extern int pp_newline();
extern int pp_note_token(int);
extern int pp_int_token();
extern int pp_real_token();
extern int pp_str_token();
extern int id_token();
extern int pp_get_int_val(char *);
extern int pp_our_atoi(char *);
extern int pp_is_zero(char *);
extern char *makeupper(char *); 
extern int strcmp_ours (char *, char *);

#endif /* SCAN_ROUTINES_H */
