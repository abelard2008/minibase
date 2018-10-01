/*
 *  $Id: rawfile.C,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Implementation of class Rawfile methods.
 */

#include <string.h>     //  strcpy
#include <fcntl.h>      //  file operations
#include <unistd.h>     //  SEEK_CUR, ..., unlink
#include <sys/stat.h>   //  fstat

#include "rawfile.h"

/* ---------------------------------------------------------------- */

/*
   PERMS

   Default permissions for creating the file.
*/

#define PERMS   0660

/* ---------------------------------------------------------------- */

/*
   rawfile_msgs

   Messages returned by Rawfile methods on detecting various errors.
*/

const char *rawfile_msgs[] = {
    "Invalid argument",
    "Cannot create raw file",
    "Cannot set raw file size",
    "Open attempted on a non-existent raw file",
    "Cannot open raw file",
    "Cannot find raw file size",
    "Not a raw file",
    "Cannot remove raw file",
    "Raw file not open",
    "Cannot seek into raw file",
    "Cannot read raw file page",
    "Cannot write raw file page",
    "Cannot sync raw file"
};

/* ---------------------------------------------------------------- */

/*
   Create constructor

   Creates the Unix file.  lseek to its end and write a byte to get
   the size right.  Once this is done, further increases/decreases to
   its size are not allowed.
*/

Rawfile::Rawfile(const char *pathname, unsigned int num_pages,
                 Status &error_status)
{
    open_failed  = true;
    error_status = OK;
    
    //
    //  Check arguments.
    //
    if (pathname == NULL  ||  *pathname == '\0'  ||
        strlen(pathname) >= MAX_PATHNAME) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_INVAL]);
        error_status = RAWFILE;
        return;
    }

    //
    //  Create the file.
    //  O_RDWR  = Open the file for reading and writing.
    //  O_CREAT = Create the file if it is doesn't exist.
    //  O_TRUNC = Truncate the file if it exists.
    //
    fd = open(pathname, O_CREAT | O_RDWR | O_TRUNC, PERMS);
    if (fd < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTCREATE]);
        error_status = RAWFILE;
        return;
    }
    
    //
    //  Seek to the end of the file and write a byte.  This fixes
    //  the size of the file.
    //
    if (lseek(fd, (off_t) num_pages * PAGESIZE - 1, SEEK_SET) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTSETNPAGES]);
        error_status = RAWFILE;
        close(fd);
        return;
    }
    char c = 0;
    if (write(fd, &c, 1) < 1) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTSETNPAGES]);
        error_status = RAWFILE;
        close(fd);
        return;
    }

    //
    //  We have succeeded.  Setup data members.
    //
    n_pages     = num_pages;
    strcpy(name, pathname);
    open_failed = false;
}

/* --------------------------------------------------------------- */

/*
   Open constructor

   Open the file.  Do an fstat on it to find its size.  Initialize
   data members.
*/

Rawfile::Rawfile(const char *pathname, Status &error_status)
{
    open_failed  = true;
    error_status = OK;

    //
    //  Check arguments.
    //
    if (pathname == NULL  ||  *pathname == '\0'  ||
        strlen(pathname) >= MAX_PATHNAME) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_INVAL]);
        error_status = RAWFILE;
        return;
    }

    //
    //  Check if the file exists.  Should use the create constructor
    //  if the file doesn't exist.
    //
    if (access(pathname, F_OK) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_NOFILE]);
        error_status = RAWFILE;
        return;
    }
    
    //
    //  Open the file and find its size.
    //
    fd = open(pathname, O_RDWR);
    if (fd < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTOPEN]);
        error_status = RAWFILE;
        return;
    }
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTFINDSIZE]);
        error_status = RAWFILE;
        close(fd);
        return;
    }
    if (statbuf.st_size % PAGESIZE != 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_NOTRAWFILE]);
        error_status = RAWFILE;
        close(fd);
        return;
    }
    n_pages = statbuf.st_size / PAGESIZE;

    strcpy(name, pathname);
    open_failed = false;
}

/* ---------------------------------------------------------------- */

/*
   Destructor

   Close the file if the constructor was able to open it.
*/

Rawfile::~Rawfile()
{
    if (!open_failed)
        close(fd);
}

/* ---------------------------------------------------------------- */

/*
   destroy()

   Close the file and destroy it.
*/

Status Rawfile::destroy()
{
    if (open_failed) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_NOTOPEN]);
        return RAWFILE;
    }

    close(fd);
    open_failed = true;     //  so that the destructor doesn't attempt
                            //  another close.
    if (unlink(name) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTREMOVE]);
        return RAWFILE;
    }
    return OK;
}

/* ---------------------------------------------------------------- */

/*
   destroy(pathname)

   Just destroy the file.
*/

Status Rawfile::destroy(const char *pathname)
{
    if (pathname == NULL  ||  *pathname == '\0') {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_INVAL]);
        return RAWFILE;
    }
    
    if (unlink(pathname) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTREMOVE]);
        return RAWFILE;
    }

    return OK;
}

/* ---------------------------------------------------------------- */

/*
   read_page

   Seek to the appropriate point and read from the file.
*/

Status Rawfile::read_page(PageId pageno, Page *page_ptr)
{
    if (open_failed) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_NOTOPEN]);
        return RAWFILE;
    }       
    
    //
    //  Check arguments.
    //
    if (pageno < 0  ||  pageno >= n_pages  ||  page_ptr == NULL) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_INVAL]);
        return RAWFILE;
    }
    
    //
    //  Read the page.
    //
    if (lseek(fd, (off_t) pageno * PAGESIZE, SEEK_SET) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTSEEK]);
        return RAWFILE;
    }       

    if (read(fd, page_ptr, PAGESIZE) < PAGESIZE) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTREAD]);
        return RAWFILE;
    }

    return OK;
}

/* ---------------------------------------------------------------- */

/*
   write_page

   Seek to the appropriate point and write the page.  Then fsync three
   times to force the update to disk.
*/

Status Rawfile::write_page(PageId pageno, const Page *page_ptr)
{
    if (open_failed) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_NOTOPEN]);
        return RAWFILE;
    }
    
    //
    //  Check arguments.
    //
    if (pageno < 0  ||  pageno >= n_pages  ||  page_ptr == NULL) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_INVAL]);
        return RAWFILE;
    }       

    //
    //  Write the page.
    //
    if (lseek(fd, (off_t) pageno * PAGESIZE, SEEK_SET) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTSEEK]);
        return RAWFILE;
    }       
        
    if (write(fd, page_ptr, PAGESIZE) < PAGESIZE) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTWRITE]);
        return RAWFILE;
    }       

    //
    //  Sync the update.
    //
    if (fsync(fd) < 0  ||  fsync(fd) < 0  ||  fsync(fd) < 0) {
        minirel_error->add_error(RAWFILE, rawfile_msgs[E_RAWFILE_CANTSYNC]);
        return RAWFILE;
    }       

    return OK;
}

