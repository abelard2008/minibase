#include <ostream>
#include <errno.h>
#include "error.h"


extern "C" void perror(const char *);


void Error::print(Status status)
{
  std::cerr << "Error: ";
  switch(status) {

    // no error

    case OK:           std::cerr << "no error"; break;

    // Tuple errors

    case TUPLE_TOO_BIG:
		std::cerr << "tuple too big, increase Tuple::max_size";
		break;

	case TUPLE_TYPE:
		std::cerr << "invalid type encountered";

    // File and DB errors

    case BADFILEPTR:   std::cerr << "bad file pointer"; break;
    case BADFILE:      std::cerr << "bad filename"; break;
    case FILETABFULL:  std::cerr << "open file table full"; break;
    case FILEOPEN:     std::cerr << "file open"; break;
    case UNIXERR:      perror("Unix error"); break;
    case BADPAGEPTR:   std::cerr << "bad page pointer"; break;
    case BADPAGENO:    std::cerr << "bad page number"; break;
    case FILEEXISTS:   std::cerr << "file exists already"; break;

    // BufMgr and HashTable errors

    case HASHTBLERROR: std::cerr << "hash table error"; break;
    case HASHNOTFOUND: std::cerr << "hash entry not found"; break;
    case BUFFEREXCEEDED: std::cerr << "buffer pool full"; break;
    case PAGENOTPINNED: std::cerr << "page not pinned"; break;
    case BADBUFFER: std::cerr << "buffer pool corrupted"; break;
    case PAGEPINNED: std::cerr << "page still pinned"; break;

    // Page class errors

    case NOSPACE: std::cerr << "no space on page for record"; break;
    case NORECORDS: std::cerr << "page is empty - no records"; break;
    case ENDOFPAGE: std::cerr << "last record on page"; break;
    case INVALIDSLOTNO: std::cerr << "invalid slot number"; break;

    // Heap file errors

    case BADRID:       std::cerr << "bad record id"; break;
    case BADRECPTR:    std::cerr << "bad record pointer"; break;
    case BADSCANPARM:  std::cerr << "bad scan parameter"; break;
    case BADSCANID:    std::cerr << "bad scan id"; break;
    case SCANTABFULL:  std::cerr << "scan table full"; break;
    case FILEEOF:      std::cerr << "end of file encountered"; break;

    // Index errors

    case BADINDEXPARM: std::cerr << "bad index parameter"; break;
    case RECNOTFOUND:  std::cerr << "no such record"; break;
    case BUCKETFULL:   std::cerr << "bucket full"; break;
    case DIROVERFLOW:  std::cerr << "directory is full"; break;
    case NONUNIQUEENTRY: std::cerr << "nonunique entry"; break;
    case NOMORERECS:   std::cerr << "no more records"; break;

    // Sorted file errors

    case BADSORTPARM:  std::cerr << "bad sort parameter"; break;
    case INSUFMEM:     std::cerr << "insufficient memory"; break;

    // Catalog errors

    case BADCATPARM:   std::cerr << "bad catalog parameter"; break;
    case RELNOTFOUND:  std::cerr << "relation not in catalog"; break;
    case ATTRNOTFOUND: std::cerr << "attribute not in catalog"; break;
    case NAMETOOLONG:  std::cerr << "name too long"; break;
    case ATTRTOOLONG:  std::cerr << "attributes too long"; break;
    case DUPLATTR:     std::cerr << "duplicate attribute names"; break;
    case RELEXISTS:    std::cerr << "relation exists already"; break;
    case NOINDEX:      std::cerr << "no index exists"; break;
    case INDEXEXISTS:  std::cerr << "index exists already"; break;

    default:           std::cerr << "undefined error status: " << status;
  }
  std::cerr << endl;
}

