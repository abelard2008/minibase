#ifndef PARSE_ROUTINES_H
#define PARSE_ROUTINES_H

#include "parser.h"

extern int pp_makenode(pp_NODPTR *,pp_NODE_TYPE);
extern void freenode( /*pp_NODPTR*/ void* node );
extern void pp_link_to_left_child(pp_NODPTR,pp_NODPTR);
extern void pp_link_to_right_child(pp_NODPTR,pp_NODPTR);
extern void pp_link_to_parent(pp_NODPTR,pp_NODPTR);
extern void pp_link_siblings(pp_NODPTR,pp_NODPTR);
extern void pp_link_all_to_parent(pp_NODPTR,pp_NODPTR,pp_NODPTR *);

#endif /* PARSE_ROUTINES_H */
