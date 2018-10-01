/*
 *  $Id: rawfile.h,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Declaration of class Rawfile which has an interface similar to DB
 *  but is much simpler to implement.  Rawfile gives the semantics of
 *  a pagefile with a fixed number of pages.
 */

#ifndef _RAWFILE_H
#define _RAWFILE_H

#include <sys/types.h>  //  off_t

#include "minirel.h"
#include "page.h"     

/* ------------------------------------------------------------------ */

/*
   rawfile_errors

   Index into an array of messages that the Rawfile methods return on error.
   Keep this in sync with rawfile_msgs.
 */

enum rawfile_errors {
    E_RAWFILE_INVAL,
    E_RAWFILE_CANTCREATE,
    E_RAWFILE_CANTSETNPAGES,
    E_RAWFILE_NOFILE,
    E_RAWFILE_CANTOPEN,
    E_RAWFILE_CANTFINDSIZE,
    E_RAWFILE_NOTRAWFILE,
    E_RAWFILE_CANTREMOVE,
    E_RAWFILE_NOTOPEN,
    E_RAWFILE_CANTSEEK,
    E_RAWFILE_CANTREAD,
    E_RAWFILE_CANTWRITE,
    E_RAWFILE_CANTSYNC
};

extern const char *rawfile_msgs[];

/* ----------------------------------------------------------------- */

/*
   Rawfile

   A Rawfile is a file which consists of a fixed set of pages.  Reads
   and writes are in units of a page.  The contents of the file are
   completely managed by the user of the Rawfile.
   The interface provided by Rawfile is similar to DB. However, it
   doesn't provide the space map related methods that DB provides,
   because a Rawfile doesn't have (or need) a space map.

   Constructors
    The first constructor creates a rawfile with a given number of
    pages.  error_status is returned OK if the create succeeded.
    Otherwise, appropriate errors are returned through the global
    minirel_error variable.  It also opens the file for reading and
    writing.  The second constructor opens the rawfile for reading and
    writing.  error_status is returned OK if the open succeeded.
    Otherwise, the suitable error is returned through minirel_error.
    The pathnames given to either constructors cannot exceed
    Rawfile::MAX_PATHNAME characters in length (including \0).

   Destructor
        Closes the rawfile opened by the create or open constructors.
        Closing occurs only if the constructor successfully opened the
        file.

   destroy
        Destroys a rawfile that was created earlier.
        destroy() destroys an open rawfile and implicitly closes it.
        destroy(name) destroys a closed rawfile.  It is given the name
        of the rawfile to destroy.

   pagesize
        Returns the size of each page of the rawfile.

   npages
        Returns the total number of pages in the raw file.

   read_page
        Reads a page from the rawfile given its page number (page
        numbering starts at 0 and increases continuously till
        numpages-1.

   write_page
        Writes the given page to the page whose number is supplied.
        Flushes internal buffers to ensure that the page makes it to
        disk.

   fd
        Unix file descriptor of the file that stores the rawfile.

   name
        Name of the Unix file that stores the rawfile.

   n_pages
        Total number of pages in the file.

   open_failed
        Set to true if the constructor couldn't open the file.
*/

class Rawfile {
public:
    const int   MAX_PATHNAME = MAXPATHNAME;

    Rawfile(const char *name, Status &error_status);
    Rawfile(const char *name, unsigned int num_pages, Status &error_status);
    ~Rawfile();

    Status destroy();
    static Status destroy(const char *name);

    unsigned int    pagesize()  { return PAGESIZE; }
    unsigned int    npages()    { return n_pages;  }
    
    Status read_page(PageId pageno, Page *page_ptr);
    Status write_page(PageId pageno, const Page *page_ptr);

private:
    int             fd;
    char            name[MAX_PATHNAME];
    unsigned int    n_pages;
    bool            open_failed;
};

/* ------------------------------------------------------------------ */

#endif  // _RAWFILE_H
