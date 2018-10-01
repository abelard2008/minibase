/* Misc. global variables and functions used by the optimizer */

#ifndef _OPT_GLOBALS_H
#define _OPT_GLOBALS_H

/* nextNodeID stores the value of the next ID to be assigned to 
 * a node.  These IDs are arbitrary and are used only for display
 * purposes.
 */
extern int nextNodeID;

/* pageSize, compCost, hashCost, moveCost, swapCost, and IOCost are 
 * essentially constants for use by join cost estimating functions
 */
extern int pageSize;

extern double compCost; // cost of compareing keys in main memory

extern double hashCost; // cost of hashing a key that is in main memory

extern double moveCost; // cost of moving a tuple in main memory

extern double swapCost; // cost of swapping two tuples that are in main memory

extern double IOCost; // cost of reading or writing a page between disk and
                      // main memory

extern bool Optimizer_IndexOnly; // include index-only plans?

class Plan_AccessMethod;
extern Plan_AccessMethod DefaultAM;

/* ResetNextNodeID sets nextNodeID to a value one higher than the
 * number of available access methods.  This is so that on each
 * query execution with the same catalog the IDs do not become
 * eccessively high.
 */
void ResetNextNodeID();

void ResetNodeIDs();

#endif
