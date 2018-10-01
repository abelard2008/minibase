/*
 *  $Id: rawfile.h,v 1.3 1996/06/02 03:06:42 luke Exp $
 *
 *  $Log: rawfile.h,v $
 *  Revision 1.3  1996/06/02 03:06:42  luke
 *  Upgraded all errors to new interface
 *
 *  Revision 1.2  1996/03/31 18:27:23  luke
 *  Added MULTIUSER tests and singleuser build code
 *
 *  Revision 1.1  1996/02/11 04:25:08  luke
 *  Initial check-in of minibase
 *
 * Revision 1.5  1995/05/10  13:32:47  cjin
 * ajit checked out, I checked in. Good enough reason.
 *
 * Revision 1.4  1995/05/09  08:48:14  cjin
 * incorporated new error protocol
 *
 * Revision 1.3  1995/05/09  06:56:57  cjin
 * added RAWFILE error mesgs
 * modified rawfile fns to use the new error protocol
 *
 * Revision 1.2  1995/05/07  21:59:36  ajitk
 * error.h contains the enumerated type for errors.
 * Changed #include to reflect this.
 *
 * Revision 1.1  1995/05/07  02:52:32  ajitk
 * Initial revision
 *
 *
 *  Declaration of class Rawfile which has an interface similar to DB
 *  but is much simpler to implement.  Rawfile gives the semantics of
 *  a pagefile with a fixed number of pages.
 */

#ifndef _RAWFILE_H
#define _RAWFILE_H

#include <sys/types.h>  //  off_t

#include "minirel.h"    //  errors
#include "page.h"       //  MINIBASE_PAGESIZE, PageId

/*
 * Declaration of Rawfile errors
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


/* ----------------------------------------------------------------- */

/*
   Rawfile

   A Rawfile is a file which consists of a fixed set of pages.  Reads
   and writes are in units of a page.  The contents of the file are
   completely managed by the user of the Rawfile.
   The interface provided by Rawfile is similar to DB, however, it
   doesn't provide the space map related methods that DB provides,
   because a Rawfile doesn't have a space map.

   Constructors
        The first constructor creates a rawfile with a given number of
        pages.  error_status is returned OK if the create succeeded,
        otherwise appropriate errors are returned through the global
        minibase_errors object.  It also opens the file for reading and writing.
        The second constructor opens the rawfile for reading and
        writing.  error_status is returned ok if the open succeeded,
        otherwise the suitable error is returned through minibase_errors.
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

   name
        Returns the name of the rawfile (the Unix filename).

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
    const int   MAX_PATHNAME = 256;

    Rawfile(const char *name, Status &error_status);
    Rawfile(const char *name, unsigned int num_pages, Status &error_status);
    ~Rawfile();

    Status destroy();
    static Status destroy(const char *name);

    unsigned int    pagesize()  { return MINIBASE_PAGESIZE; }
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
