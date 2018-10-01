set minibase_cost_help {
Optimizer Costs

Variable Definitions

 Variable Name                         Description
    NTuples   Tuples
     NKeys    Number of Distinct Key
    IHeight   height of btree,
     ICost    # of pages required for hash look up a tuple

    INPages   Number of Index Leaf Pages (width of btree, # of hash table
              pages)

      BPR     Buffer Pool Pages Required; how many pages are required in
              the buffer pool in order for this query to run.
    NPages    Number of Relation Pages
    NTuples   Number of Tuples
     PSize    Page Size
  RF(Primary) Primary Predicate(s) Reduction Factor
    NPages    Result Number of Pages
      RF      Reduction Factors(all predicates for this node)
     TSize    Result Tuple Sizes
Primary Predicates are the ones used to access the data.
Composite indexes have more than one primary predicate and in that case:

Primary = primary1 AND primary2 AND ... primaryN
RF(Primary) = RF(primary1) * RF(primary2) * ... RF(primaryN)

Reduction Factors

With only minor alterations, this comes right from "Access Path Selection in
a Relational Database Management System" by Selinger et al.
                   RF = 1 / NKeys if index on column
 column = value
                   RF = 1/10 otherwise
                   RF = 1 / MAX (1.NKeys, 2.NKeys) if index on columns 1
                   and 2
 column1 = column2
                   RF = 1 / I.NKeys if only one index on column i.
                   RF = 1/10 otherwise
                   RF = 1 - 1 / NKeys if index on column
 column != value
                   RF = 1 - 1/10 otherwise
                   RF = 1 - 1 / MAX (1.NKeys, 2.NKeys) if index on columns
 column1 !=        1 and 2
 column2           RF = 1 - 1 / I.NKeys if only one index on column i.
                   RF = 1 - 1/10 otherwise
                   (note: for our purposes, no real point in distinguishing
                   between < and <= with regard to RF)
 value1 < col <
 value2            RF = (value2 - value1) / (high key value - low key
                   value) if int or real
                   1/4 otherwise
                   (or any other open-ended comparison -- the formula is
                   similar)
 value < column    RF = (high key value - value) / (high key value - low
                   key value) if numeric
                   RF = .3
 column1 < column2 RF = .3
 A AND B           RF = RF(A) * RF(B)
 A OR B            RF = RF(A) + RF(B) - RF(A) * RF(B)

Index Only

If the only attribute that is being used in either the selection or
projection in a plan node can be supplied by the index, that node will be
marked as being an index-only plan.

Access Method Costs

   Access Method                         Description
     File Scan     NPages
                   log(NPages) + (NPages * RF(Primary))
  File Scan, file  We can use a binary search to find the correct first
    is sorted      page, and then we only need to read in as many pages
                   that match the condition based on the sorted attribute.
                   INPages
  Hash Index-Only
       Scan        We can read through all of the pages of the hash index
                   instead of going to the base relation.
                   ICost + (NPages * RF(Primary))
  Clustered Hash   The cost to get to the appropriate pages (ICost), plus
       Scan        the number of pages that contain records (since
                   clustered, we know they are all together).
                   ICost + NTuples(Fetched)
 Unclustered Hash  The cost to get to the appropriate pages (ICost), plus
       Scan        one page IO for each matching tuple (since each could be
                   on a different page).
                   IHeight + (INPages * RF(Primary))
 Index-Only BTree  The depth of the btree, plus the number of index leaves
      Index        we have to look at. We don't have to go to the base
                   relation.
                   IHeight + (INPages * RF(Primary)) + (NPages *
                   RF(Primary))
  Clustered BTree  The depth of the btree, plus the number of index leaves
      Index        we have to look at, plus the number of pages that
                   contain records (since clustered, we know they are all
                   together)

 Unclustered BTree IHeight + (INPages * RF(Primary)) + NTuples(Fetched)
      Index        In this case, each record could be on a different page.

Joins

In general, we assume all joins are pipelined; that is, the results of one
join are directly fed into the next.

Index Nested Loops

The optimizer never considers whether the left plan was already ordered or
not. If we could make a guess on how many keys are entering in the left hand
side, we could then use the L.NKeys instead of L.NTuples in the formulas,
which could be cheaper.

The cost estimate is computed using the following formula for joining plan R
and plan L, where R is the right subplan and L is the left:

      L.TotalCost + (L.Cardinality * R.Access)

Where R.Access is the cost to get one tuple. Note, that RF(Primary) is the
selectivity of the join, so the selectivity for just one probe should be
(RF(Primary) / L.NTuples) if we have L.NTuples probes. So, what I did was
grab the cost for R.Access from the access-method level lookups & then did
some algebra to simplify it up a little bit. This would be a high estimate
if the left plan was sorted & the index is clustered. (Note, that these will
work in non-equijoin joins if the index supports equijoins)
                        (not strictly an index, but close) L.Total +
 Right Relation Sorted  L.NTuples * ( log(R.NPages) + R.NPages *
                        RF(Primary) / L.NTuples )
     (Clustered or
      Unclustered)      L.Total + L.NTuples * R.ICost
 Index-Only Hash Index

  Clustered Hash Index  L.Total + L.NTuples * ( R.ICost + R.NPages *
                        RF(Primary) / L.NTuples)
                        R.ICost is cost to use index once
                        L.Total + L.NTuples * ( R.ICost + NTuples(Fetched)
 Unclustered Hash Index / L.NTuples + R.ICost)
                        Note that NTuples(Fetched) = NTuples(Input) *
                        RF(Primary), or NTuples(Fetched) = L.NTuples *
                        R.NTuples * RF(Primary)

 Index Only BTree Index L.Total + L.NTuples * ( R.IHeight + R.INPages *
                        RF(Primary)/L.NTuples)
                        L.Total + L.NTuples * ( R.IHeight + R.INPages *
 Clustered BTree Index  RF(Primary)/L.NTuples + R.NPages *
                        RF(Primary)/L.NTuples)
                        L.Total + L.NTuples * ( R.IHeight + R.INPages *
   Unclustered BTree    RF(Primary)/L.NTuples + NTuples(Fetched)/L.NTuples)
         Index          (note that NTuples(Fetched) = NTuples(Input) *
                        RF(Primary) = L.NTuples * R.NTuples * RF(Primary))

Sort Merge

Hash indexes are not allowed.

In general, the cost is the cost to sort one or both incoming relations, if
necessary, plus the cost to access each one once. (If there is an index, it
will include the cost to use the index)

We also assume we can always sort in two runs (not always reasonable)

Note; we assume that sort-merge is done in two distinct steps -- a sort
step, and a merge step. It assumes (like the current version of minibase)
that merging is not done at the same time as the last sort run.

          Both relations unsorted          4L.NPages + 4R.NPages +
                                           L.TotalCost + R.NPages
 Left relation sorted, right unsorted after4R.NPages + L.TotalCost +
  access method, no b-tree index on right  R.NPages
 Left relation unsorted, right sorted after4L.NPages + L.TotalCost +
  access method, no b-tree index on right  R.NPages
 Left relation unsorted, right sorted after
  access method, clustered b-tree index on 4L.NPages + L.TotalCost +
                   right                   R.IHeight + R.INPages + R.NPages
 Left relation unsorted, right sorted after4L.NPages + L.TotalCost +
 access method, unclustered b-tree index onR.IHeight + R.INPages +
                   right                   R.NTuples
 Both relations sorted after access method,
         no b-tree index on right          L.TotalCost + R.NPages
 Both relations sorted after access method,L.TotalCost + R.IHeight +
      clustered b-tree index on right      R.INPages + R.NPages
 Both relations sorted after access method,L.TotalCost + R.IHeight +
     unclustered b-tree index on right     R.INPages + R.NTuples

Other Join Methods

 Tuple Nested Loops (L.NTuples * R.NPages) + L.Total
 Page Nested Loops  (L.NPages * R.NPages) + L.Total
     Hash Join      L.TotalCost + 2L.NPages + 2R.NPages + R.NPages

Order by, group by, distinct, and aggregates

Aggregates are free. If all aggregates can be computed using some index, and
index-only plan is produced. (However, all the aggregates must be computable
from the same index. It could be cheaper to instead scan two different
indexes and join them together, but the optimizer isn't smart enough to
consider this currently.)

Order by and group by: Group by and distinct are done by sorting the
relation and then stepping through the different groups; after this point
they are not significantly different as far as the optimizer is concerned
from a similar order by clause.

If the plan is already in the order desired, there is no additional cost.
Otherwise, 4 * NP is added to the cost of the plan to sort the relation.
}
