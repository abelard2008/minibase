#include "opt_globals.h"
#include "query.h"

bool Optimizer_IndexOnly = true;

int nextNodeID = 1;

int pageSize = 1024;

/* The following values are somewhat arbitrary.  If you know better what they
 * should be, please change them.
 */
double compCost = 0.5;

double swapCost = 0.5;

double hashCost = 1;

double moveCost = 1;

double IOCost = (double) pageSize / 55;  // this was chosen simply to try to
                                         // get the cost estimates for reading
                                         // or writing a relation to work out
                                         // to be approximately the same as
                                         // the cardinality, this is to try to
                                         // roughly correspond to the rest of
                                         // system

Plan_AccessMethod DefaultAM;

static int baseNextNodeID = 0;

void ResetNodeIDs()
{
  baseNextNodeID = 0;
  nextNodeID = 1;
}

void ResetNextNodeID()
{
  if (baseNextNodeID == 0)
    baseNextNodeID = nextNodeID;
  else
    nextNodeID = baseNextNodeID;
}
