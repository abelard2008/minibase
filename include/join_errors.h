// this file contains all the error codes that can be returned by
// the join methods

class JoinError
{
public:
    enum {
        LOW_MEMORY,
        ATTR_TYPE_MISMATCH,
        WRONG_R_EST,
        HASH_TBL_ERROR,
        NEWFAILED,
        INTERNALERR,
        BAD_SCAN,
        CANT_SCAN_NONE,
        UNK_INDEX_TYPE,
    };
};
