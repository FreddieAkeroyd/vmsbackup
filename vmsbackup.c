/*
 *
 *  Title:
 * Backup
 *
 *  Decription:
 * Program to read VMS backup tape
 *
 *  Author:
 * John Douglas CAREY.  (version 2.x, I think)
 * Sven-Ove Westberg    (version 3.0)
 *      See ChangeLog file for more recent authors.
 *
 *  Net-addess (as of 1986 or so; it is highly unlikely these still work):
 * john%monu1.oz@seismo.ARPA
 * luthcad!sow@enea.UUCP
 *
 *
 *  Installation (again, as of 1986):
 *
 * Computer Centre
 * Monash University
 * Wellington Road
 * Clayton
 * Victoria 3168
 * AUSTRALIA
 *
 */

/*
// Portions Copyright (c) 2008 HoffmanLabs LLC
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// Except as contained in this notice, the name(s) of the above 
// copyright holders shall not be used in advertising or otherwise
// to promote the sale, use or other dealings in this Software 
// without prior written authorization.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
*/
/*
** Portions Copyright (c) 1983, 1989
**    The Regents of the University of California.  All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. All advertising materials mentioning features or use of this software
**    must display the following acknowledgement:
**      This product includes software developed by the University of
**      California, Berkeley and its contributors.
** 4. Neither the name of the University nor the names of its contributors
**    may be used to endorse or promote products derived from this software
**    without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/



/*
//  Port to Solaris C. Enhance VMS time support. Add extra tape mark
//  skip to get past tape header. Add close statement for extracted files.
//  Time stamp extracted files with original create and mod times. 
//  Add further documentation comments. First attempt to handle indexed
//  files.
//
// 10/ 3/08 K. Clark CSC/STSci
*/

/*
//  05-May-2008, Stephen Hoffman, HoffmanLabs LLC
//  incorporate contributed two gigabyte (longword) display fixes.
//  (Thanks, Weiss.)   Resolved various endian issues from the
//  Solaris port; added BSD code for endian-ness from nameser.h. 
*/


/* begin included (and reworked) BSD code (from nameser.h) */
#ifndef BYTE_ORDER
#if ((__APPLE__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1060))
# include <machine/endian.h>
#endif
#if (BSD >= 199103)
# include <machine/endian.h>
#endif
#ifdef linux
# include <endian.h>
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long (pdp)*/

#if defined(vax) || defined(ns32000) || defined(sun386) || defined(i386) || \
	defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
	defined(__alpha__) || defined(__alpha) || \
    	(defined(__Lynx__) && defined(__x86__))
#define BYTE_ORDER      LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
	defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
	defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
	defined(apollo) || defined(__convex__) || defined(_CRAY) || \
	defined(__hppa) || defined(__hp9000) || \
	defined(__hp9000s300) || defined(__hp9000s700) || \
	defined (BIT_ZERO_ON_LEFT) || defined(m68k)
#define BYTE_ORDER      BIG_ENDIAN
#endif

#if (defined(__Lynx__) && (defined(__68k__) || defined(__sparc__) || defined(__powerpc__)))
#define BYTE_ORDER      BIG_ENDIAN
#endif

#endif /* LITTLE_ENDIAN */
#endif /* BYTE_ORDER */
                               
#if !defined(BYTE_ORDER) || \
    (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN && \
    BYTE_ORDER != PDP_ENDIAN)
        /* you must determine what the correct bit order is for
         * your compiler - the next line is an intentional error
         * which will force your compiles to bomb until you fix
         * the above macros.
         */
   error "Undefined or invalid BYTE_ORDER";
#endif
/*  end included (and reworked) BSD code (from nameser.h)  */


/* Does this system have the magnetic tape ioctls?  The answer is yes for 
   most unices, and I think it is yes for VMS 7.x with DECC 5.2 (needs
   verification), but it is no for VMS 6.2.  mtio.h was removed from
   Mac OS X Snow Leopard (10.6).  */
#if ((__APPLE__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1060))
# define HAVE_MT_IOCTLS 0
#endif
#ifndef HAVE_MT_IOCTLS
#  define HAVE_MT_IOCTLS 1
#endif

#ifdef HAVE_UNIXIO_H
/* Declarations for read, write, etc.  */
#  include <unixio.h>
#else
#  include <unistd.h>
#  include <fcntl.h>
#endif
/*
 * On VMS these are in unistd.h, which in build.com is not included
 * as HAVE_UNIXIO_H is defined.
 */
#ifndef STDIN_FILENO
# define STDIN_FILENO 0 /* Standard input. */
# define STDOUT_FILENO 1 /* Standard output. */
# define STDERR_FILENO 2 /* Standard error output. */
#endif

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

#if HAVE_MT_IOCTLS
#  include <sys/ioctl.h>
#  include <sys/mtio.h>
#endif /* HAVE_MT_IOCTLS */

#ifdef REMOTE
#  include <local/rmt.h>
#  include <sys/stat.h>
#endif /* REMOTE */

#include <sys/file.h>

#include "vmsconstants.h"

#ifdef __VAXC
/* The help claims that mkdir is declared in stdlib.h but it doesn't
   seem to be true.  AXP/VMS 6.2, DECC ?.?.  On the other hand, VAX/VMS 6.2
   seems to declare it in a way which conflicts with this definition.
   This is starting to sound like a bad dream.  */
int mkdir ();
#endif  /* __VAXC */

#include "vmsbackup.h"
// #include "sysdep.h"

// Dump raw data records.
static void debug_dump(const unsigned char* buffer, 
                       unsigned short int dsize, 
                       unsigned short int dtype);


int   match ();
char *strlocase ();

/* 
*   Convert the OpenVMS epoch to the Unix epoch, punting on
*   select special cases.
*/
time_t time_vms_to_unix(unsigned long long vms_time_ll);

/*
 *  Convert the OpenVMS epoch into text, selectively punting.
*/
time_t time_vms_to_unix_asc( unsigned char *vmstime,   int vmstim_len, 
                             char *date_text, int *date_text_len );

#define MAX_DATE_STR 24



#if BYTE_ORDER == LITTLE_ENDIAN

unsigned long long
  getu64 (unsigned char *addr)  {
    return (*(unsigned long long int *) addr);
 }
unsigned int
 getu32 (unsigned char *addr) {
    return *(unsigned int *) addr;
 }
unsigned int
 getu16 (unsigned char *addr) {
    return *(unsigned short int *) addr;
 }
#endif
#if BYTE_ORDER == BIG_ENDIAN

unsigned long long
  getu64 (unsigned char *addr)  {

  long long big_endian;

  unsigned char *dst = (unsigned char *)&big_endian;
  unsigned char *src = (unsigned char *)addr;

  dst [0] = src[7];
  dst [1] = src[6];
  dst [2] = src[5];
  dst [3] = src[4];
  dst [4] = src[3];
  dst [5] = src[2];
  dst [6] = src[1];
  dst [7] = src[0];

  return big_endian;
 }

unsigned 
 getu32 (unsigned char *addr) {
    return (((((unsigned)addr[3] << 8) | addr[2]) << 8)
              | addr[1]) << 8 | addr[0];
 }


unsigned int
 getu16 (unsigned char *addr) {
    return (addr[1] << 8) | addr[0];
 }
#endif



// VMS backup block header.
struct bbhdef {
   unsigned char bbh_dol_w_size[2];      // Size in bytes of block header. 
   unsigned char bbh_dol_w_opsys[2];     // Operating system ID.
   unsigned char bbh_dol_w_subsys[2];    // Subsystem ID.
   unsigned char bbh_dol_w_applic[2];    // Application ID.
   unsigned char bbh_dol_l_number[4];    // Block sequence number.
   char          bbh_dol_t_spare_1[20];  // Reserved spare bits.
   unsigned char bbh_dol_w_struclev[2];  // Block structure level.
   unsigned char bbh_dol_w_volnum[2];    // Media volume number.
   unsigned char bbh_dol_l_crc[4];       // Block CRC.
   unsigned char bbh_dol_l_blocksize[4]; // Block size in bytes.
   unsigned char bbh_dol_l_flags[4];     // Block flags.
   char          bbh_dol_t_ssname[32];   // Save set name (counted ASCII).
   unsigned char bbh_dol_w_fid[3][2];    // Current file ID.
   unsigned char bbh_dol_w_did[3][2];    // Current directory ID.
   char          bbh_dol_t_filename[128];// Current file name.
   char          bbh_dol_b_rtype;        // Record type of current file.
   char          bbh_dol_b_rattrib;      // Record attributes of current file.
   unsigned char bbh_dol_w_rsize[2];     // Record size of current file.
   char          bbh_dol_b_bktsize;      // Bucket size of current file.
   char          bbh_dol_b_vfcsize;      // VFC area size of current file.
   unsigned char bbh_dol_w_maxrec[2];    // Maximum record size of current file.
   unsigned char bbh_dol_l_filesize[4];  // Allocation of current file.
   char          bbh_dol_t_spare_2[22];  // Reserved - spare bytes.
   unsigned char bbh_dol_w_checksum[2];  // Header checksum.
} *ablock_header;

 // VMS backup record header.  This structure prefixes each record
 //  within a data block. It identifies the type and use of the record.
struct brhdef {
   unsigned char brh_dol_w_rsize[2];      // Record size in bytes - 
                                          //  unsigned short int
   unsigned char brh_dol_w_rtype[2];      // Record type. -
                                          //  unsigned short int
   unsigned char brh_dol_l_flags[4];      // Record flags. -
                                          //  unsigned int
   unsigned char brh_dol_l_address[4];    // Address of data (e.g. VBN or LBN).
                                          //  unsigned int
   unsigned char brh_dol_l_blockflags[4]; // Per block error mask & reserved?
                                          //  unsigned short int, unsigned short int

} *arecord_header;


/* Define VMS backup record types. */

#define brh_dol_k_null        0 /* null record */
#define brh_dol_k_summary     1 /* saveset summary record */
#define brh_dol_k_volume      2 /* volume summary */
#define brh_dol_k_file        3 /* file attributes */
#define brh_dol_k_vbn         4 /* file VBN */
#define brh_dol_k_physvol     5 /* disk volume attributes */
#define brh_dol_k_lbn         6 /* disk volume LBN */
#define brh_dol_k_fid         7 /* file identification */
#define brh_dol_k_file_ext    8 /* file extension record */
#define brh_dol_k_lbn_576     9 /* 576-byte LBN record */
#define brh_dol_k_rs_dirattr 10 /* RSTS directory attributes */
#define brh_dol_k_alias      11 /* file alias entry */
#define brh_dol_k_max_rtype  12 /* Maximum number of VMS record types. */

#define MAX_RETRYS           10 /* Maximum of I/O read retrys. */

/* VMS backup file attribute structure. */
struct bsa {
   unsigned char bsa_dol_w_size[2]; // Size of attribute value. 
   unsigned char bsa_dol_w_type[2]; // Atribute code.
   unsigned char bsa_dol_t_text[1]; // Attribute data.
}*data_item;


#ifdef STREAM
  char *def_tapefile = "/dev/rts8";
#else
  // char *def_tapefile = "/dev/rmt8";
  char *def_tapefile = "/dev/rmt/1";
#endif

char filename[128];
char unix_filename[256];

time_t create_time;
time_t last_create_time;
time_t mod_time;
time_t last_mod_time;

unsigned long long int  filesize;
unsigned long long int  allocated_bytes_file_size;

 // Breakout of VMS Record format bytes of a file attribute.
static struct vms_rec_type {
   union {
      unsigned char rec_format; // VMS record type.

      struct {
         unsigned char rtype : 4; // VMS record type subfield.
         unsigned char fileorg : 4; // VMS file organization.
      } rectype_bits;

   } rec_type_word;
} rec_type;

char recatt;  /* record attributes */

FILE *fptr_out = NULL;

/* Number of bytes we have read from the current file so far (or something
   like that; see process_vbn).  */
int  file_byte_count;

short reclen;
short fix;
short recsize;
unsigned short int VFC_ctr_bytes   = 0;
int needToSkipVFC = 0;

/* Number of files we have seen.  */
unsigned int nfiles;

/* Number of blocks in those files.  */
unsigned long nblocks;

#ifdef NEWD
  FILE *lf;
#endif

int fd;  /* tape file descriptor */

/* Entirely weird, but some savesets have several summary records.
*  These seem to entirely replicate the contents of the tape; it
*  might be triggered by a searchlist default when the BACKUP is
*  created?  BACKUP itself appears to ignore these, so we too 
*  ignore everything  starting at the second summary record and
*  onward.   (This smells vaguely of a BACKUP bug.)
*/
/* No bug. This happened, when an XORBLOCK was processed as a DATABLOCK
 * and a record type 1 == summary was found in the XOR data.
 * For now, XORBLOCKS are skipped.
 */
int summary_seen = 0;


/* Command line stuff.  */

/* The save set that we are listing or extracting.  */
char *tapefile;
/* The output file we are extracting into.  */
char *outfile;

/* We're going to say tflag indicates our best effort at the same
   output format as VMS BACKUP/LIST.  Previous versions of this
   program used tflag for a briefer output format; if we want that
   feature we'll have to figure out what option should control it.  */
int tflag; // List files in saveset.
int cflag; // Retain complete filename.
int dflag; // Create subdirectories.
int eflag; // Extract all files.
int sflag; // Read saveset number.
int vflag; // List files as they are processed.
int wflag; // Confirm files before restoring.
int xflag; // Extract files.
int ehint; // Hint for using the eflag when one or more files were ignored
int debugflags;
#define D_BLKREC   0x1
#define D_FILE     0x2
#define D_VBN      0x4
#define D_IO       0x8
#define D_SUM	  0x10
#define D_TIME    0x20
#define D_FILE_X  0x40
#define D_VBN_X   0x80
#define D_SUM_X  0x100

/* Extract files in binary mode.  FIXME: Haven't tried to document
   (here or in the manpage) exactly what that means.  I think it probably
   means (or should mean) to give us raw bits, the same as you would get
   if you read the file in VMS directly using SYS$QIO or some such.  But
   I haven't carefully looked at whether that is what it currently does.  */
int flag_binary;

int flag_full; // Full detail in listing.

/* Which save set are we reading?  */
int selset;

/* These variables describe the files we will be operating on.  GARGV is
   a vector of GARGC elements, and the elements from GOPTIND to the end
   are the names.  */
char **gargv;
int  goptind, gargc;

int setnr;

#define LABEL_SIZE 80

/* Default blocksize, as specified in -b option.  */
int blocksize = 32256;

#if HAVE_MT_IOCTLS
  struct mtop op;
#endif

int typecmp ();



FILE *
openfile(char *req_file_name, int struclev)
 {
    FILE *found_file_ftr = NULL;

    char temp_name_buf[256];
    char *output_unix_filename;
    char *temp_unix_filename;
    char s;
    char *ext;

    char ans[80];
    int  procf;

    procf = 1;

     // Copy the input requested file name to the output file name. 
    output_unix_filename = req_file_name;

     // Allocate enough memory for the temporary unix file name.
    temp_unix_filename = temp_name_buf;

    /* Step through each character of the file name and convert it to
       lower case.
    */
    while (*output_unix_filename) {
/*
       if (isupper(*output_unix_filename))
         *temp_unix_filename = *output_unix_filename - 'A' + 'a';
       else
         *temp_unix_filename = *output_unix_filename;
*/
       *temp_unix_filename = ( struclev == 2) ? 
                             tolower( *output_unix_filename ) : 
                              *output_unix_filename;
       output_unix_filename++;
       temp_unix_filename++;
    }

    // Terminate the lower case file name.
    *temp_unix_filename = '\0';

    // Copy the lower case file name to the output file name.
    output_unix_filename = temp_name_buf;

    temp_unix_filename = ++output_unix_filename;

     // Convert the VMS directory path to Unix format.
    while (*temp_unix_filename) {

       if (*temp_unix_filename == '.' || *temp_unix_filename == ']') {
          s  = *temp_unix_filename;
          *temp_unix_filename = '\0';

	   // Create an output directory if requested.
          if (procf && dflag) {
             mkdir(output_unix_filename, 0777);
          }

          *temp_unix_filename = '/';

          if (s == ']') {
	      // Found the end of the directory path.
             break;
          }
       } 

       temp_unix_filename++;

    } // Convert the VMS directory path to Unix format.

     // Position the pointer at the end of the file path.
    temp_unix_filename++;

    if (!dflag) {
        // Write the output file to the current directory.
       output_unix_filename = temp_unix_filename;
    }

    // Locate the start of the VMS version part of the file name 
    while (*temp_unix_filename && *temp_unix_filename != ';') {

       if( *temp_unix_filename == '.') {
	   // Record the file name extension.
          ext = temp_unix_filename;
       }

       temp_unix_filename++;
    }

    if (cflag) {
        // Retain the version number after a ":" instead of a ";".
       *temp_unix_filename = ':';

    }  else {
       // Strip off the version number.
       *temp_unix_filename = '\0';
    }

    if(!eflag && procf && !(goptind < gargc)) {
       // Check if this file should be ignored, because it matches an entry
       // on the extensions list,
       // but only if there was no explicit FILE list on the command line
       procf = typecmp(++ext);
       if (!procf && vflag) {
          printf("ignoring: %s\n", filename);
       }
       if (!procf && !ehint) {
	       ehint = 1;
	       printf("one or more files matching the extensions list are ignored,\n"
		      "consider using the -e/--extensions option\n");
	       if (!vflag)
		       printf("or the -v/--verbose option to have the ignored files listed\n");
       }
    }

    if(procf && wflag) {
        // Confirm file is to be extracted.
       printf("extract %s [ny]: ",filename);

       fflush(stdout);
       gets(ans);

       if(*ans != 'y') {
          procf = 0;
       }
    }

    if(procf) {
        /* Open the file for writing. */
       if(outfile)
	  if(0==strcmp(outfile,"-"))
	     found_file_ftr = fdopen(STDOUT_FILENO,"w");
	  else
	     found_file_ftr =  fopen(outfile, "w");
       else
          found_file_ftr =  fopen(output_unix_filename, "w");

        // Record the current file name and date.
       strcpy (unix_filename, output_unix_filename);
       last_create_time = create_time;
       last_mod_time    = mod_time;

    } else {
       found_file_ftr = NULL;
    }

    return (found_file_ftr);
 }


/* Close the output file and set the file time stamps. */
int close_file (FILE *out_fptr, char output_filename[256], 
                time_t file_create_date, time_t file_mod_date) {

   int status = 0;

   struct utimbuf utb;

   status = fclose(out_fptr);

   if (debugflags & D_IO) {
      fprintf (stderr, "Closing file: %s status %d\n", output_filename, status);
   }

   // Set the file access time and modification time if known.
//   if ((file_create_date != NULL) & (file_mod_date != NULL)) {
   if ( (file_create_date) && (file_mod_date) ) { 
      utb.actime  = file_create_date;
      utb.modtime = file_mod_date;
      utime (output_filename, &utb);

      if (debugflags & D_IO)  {
         fprintf (stderr, "Setting file %s create time to %d\n",
                 output_filename, (int) file_create_date);
         fprintf (stderr, "Setting file mod time to %d\n",
                 (int) file_mod_date);
       }
   }

   return status;
}

int
typecmp(register char *str)
  /* Compare the filename type in str with our list of file type 
      to be ignored.  

      Return 0 if the file is to be ignored.
      Return 1 if the file is to be included; if the file is not 
                in our list and should not be excluded. 
  */
 {
     /*  SORTED! */
    static char *type[] = {
                "dir",          /* directory file */
                "dmp",          /* dump file */
                "exe",          /* executable image */
                "hlb",          /* help library */
                "lib",          /* vms object library */
                "mlb",          /* macro32 library */
                "obj",          /* object file */
                "odl",          /* rsx overlay description file */
                "olb",          /* rsx object library */
                "pcsi",         /* POLYCENTER Installation Kit */
                "pcsi$compressed",
                "pcsi-dcx_axpexe",
                "pcsi-dcx_vaxexe",
                "pmd",          /* rsx post mortem dump */
                "stb",          /* symbol table */
                "sys",          /* bootable system image */
                "tlb",          /* text library */ 
                "tlo",          /* ? */
                "tsk",          /* rsx executable image */
                "upd",          /* ? */
                ""              /* null string terminates list */
    };

    register int    i;

        i = -1;
        while (*type[++i]) {
                int foo;
                int str_len = strlen( str );
                int typ_len = strlen( type[i] );
                if ( !typ_len )         /* no more types in list? include the file */
                        return 1;
                if ( str_len != typ_len )       /* length mismatch?  skip the comparison */
                        continue;
                foo = strncasecmp(str, type[i],typ_len);
                if ( foo == 0 ) /* zero is a match; ignore the file */
                     return(0);
        }
	return 1;      /* default to include file */
 }


/* Process a VMS backup summary record. */
void
process_summary (unsigned char *buffer, unsigned short rsize)
 {
    unsigned short int  dsize;

    unsigned char      *text;

    int  attr_ptr;
    int  type;

     /* These are the various fields from the summary.  */
    unsigned long  id = 0;
    char           *saveset = "";
    size_t          saveset_size = 0;

    char           *command = "";
    size_t          command_size = 0;

    char           *comment = "";
    size_t          comment_size = 0;

    char           *written_by = "";
    size_t          written_by_size = 0;

    unsigned short  grp = 0377;
    unsigned short  usr = 0377;

    char           *os = "<unknown OS>"; /* '\0' terminated.  */
    char           *osversion = "unknown";
    size_t          osversion_size = 7;

    char           *nodename = "";
    size_t          nodename_size = 0;

    char           *written_on = "";
    size_t          written_on_size = 0;

    char           *backup_version = "unknown";
    size_t          backup_version_size = 7;

    char           *vol_set_name = "";
    size_t          vol_set_name_size = 0;

    char            date[MAX_DATE_STR];

    int             date_length = 0;
    int             num_files   = 0;
    unsigned long   blksz       = 0;
    unsigned int    grpsz       = 0;
    unsigned int    bufcnt      = 0;
    unsigned int    num_vols    = 0;
    unsigned int    backup_size = 0;


    if (!tflag) {
       // No verbose mode requested.
      return;
    }

     /* Check the header word - this should be a saveset summary record. */
    if (buffer[0] != 1 || buffer[1] != 1) {
       printf ("Cannot print summary; invalid data header\n");
       return;
    }

     // Skip over the first two bytes which contain the record type.
    attr_ptr = 2;

    while (attr_ptr < rsize) {

         // Get the size of the attribute value.
       dsize = getu16 (((struct bsa *)&buffer[attr_ptr])->bsa_dol_w_size);

        // Get the attribute type.
       type  = getu16 (((struct bsa *)&buffer[attr_ptr])->bsa_dol_w_type);

        // Get the attribute value.
       text  = ((struct bsa *)&buffer[attr_ptr])->bsa_dol_t_text;

       if ( debugflags & D_SUM ) {
          fprintf (stderr, "\n--process_summary debug_dump\n");
          debug_dump(text, dsize, type);
          fprintf (stderr, "--Processing summary record %d type = %d size = %d\n",
                  attr_ptr, type, dsize);
       }

       if ( !dsize ) {
          if ( debugflags & D_SUM ) {
             fprintf (stderr, "\nnull dsize; slamming the record type code.\n");
 	  }
         type = 0;
	}
       switch (type) {
          case 0:
              /* This seems to be used for padding at the end
                  of the summary. 
              */
	    //goto end_of_summary;
             break;

          case BSA_K_SSNAME:
	      // Store saveset name and its length. 
             saveset      = (char *)text;
             saveset_size = dsize;
             break;

          case BSA_K_COMMAND:
	       // Store the backup command line.
             command      = (char *)text;
             command_size = dsize;
             break;


          case BSA_K_COMMENT:
	      // Store whatever the user included as a comment and its length.
             comment      = (char *)text;
             comment_size = dsize;
             break;

          case BSA_K_USERNAME:
	      // Store the name of the user who created the backup.
             written_by      = (char *)text;
             written_by_size = dsize;
             break;

          case BSA_K_USERUIC:
  	      // Store the user's UIC.
             if (dsize == 4) {
                usr = getu16 (text);
                grp = getu16 (text + 2);
             }
             break;

          case BSA_K_DATE:
	      // Store the date the backup was made.
             text = ((struct bsa *)&buffer[attr_ptr])->bsa_dol_t_text; 

             time_vms_to_unix_asc( text, dsize, date, &date_length );

             break;

          case BSA_K_OPSYS:
	      // Store the operating system name.
             if (dsize == 2) {
                unsigned short oscode;

                oscode = getu16 (text);

	         // Convert the OS code name.
                switch(oscode) {

                   case BACKUP_K_OPSYS_IA64:
                      os = "OpenVMS I64";
                      break;

                   case BACKUP_K_OPSYS_ALPHA:
                      os = "OpenVMS Alpha";
                      break;

                   case BACKUP_K_OPSYS_VAX:
                      os = "OpenVMS VAX";
                      break;

                   default:
		     if ( debugflags & D_SUM ) {
                        fprintf(stderr, "Unknown OS Code = %d\n", oscode );
                     }

                     os = "Unknown OS Code";
                     break;

                }
             }
             break;

          case BSA_K_SYSVER:
	      // Store the operating system version.
             osversion      = (char *)text;
             osversion_size = dsize;
             break;

          case BSA_K_NODENAME:
	      // Store the node name the backup was run on.
             nodename      = (char *)text;
             nodename_size = dsize;
             break;

          case BSA_K_SIR:
	      // Store the CPU systems ID register.
             if (dsize >= 4) {
                id = getu32 (text);
             }
             break;

          case BSA_K_DRIVEID:
	      // Store the name of the drive used for the backup.
             written_on      = (char *)text;
             written_on_size = dsize;
             break;

          case BSA_K_BACKVER:
	      // Store the backup utility's version number.
             backup_version      = (char *)text;
             backup_version_size = dsize;
             break;

          case BSA_K_BLOCKSIZE:
	      // Store the block size of the saveset.
             if (dsize >= 4) {
                blksz = getu32 (text);
             }
             break;

          case BSA_K_XORSIZE:
	      // Store the size of each XOR group.
             if (dsize >= 2) {
                grpsz = getu16 (text);
             }
             break;

          case BSA_K_BUFFERS:
	      // Store the number of buffers used.
             if (dsize >= 2) {
                bufcnt = getu16 (text);
             }
             break;

          case BSA_K_VOLSETNAM:
	      // Store the volume set name.
             vol_set_name      = (char *)text;
             vol_set_name_size = dsize;

	     if (debugflags & D_SUM_X) {
                fprintf(stderr, "BSA$K_VOLSETNAM: %s dsize = %d\n",
                       vol_set_name, dsize);
             }
             break;

          case BSA_K_NVOLS:
	      // Get the number of volumes in the save set.

             if (dsize >= 2) {
                num_vols = getu16 (text);
             }

	     if (debugflags & D_SUM_X) {
                fprintf(stderr, "BSA$K_NVOLS: %d dsize = %d\n", num_vols, dsize);
             }

             break;

          case BSA_K_BACKSIZE:
	      // Get the total file space in the save set.
             if (dsize >= 2) {
                backup_size = getu16 (text);
             }

	     if (debugflags & D_SUM_X) {
                fprintf(stderr, "BSA$K_BACKSIZE: %d dsize = %d\n", backup_size, dsize);
             }
             break;

          case BSA_K_BACKFILES:
             num_files = getu16 (text);

	      // Get the number of files in the save set.
	     if (debugflags & D_SUM_X) {
                fprintf(stderr, "BSA$K_BACKFILES: dsize = %d\n", dsize);
             }
             break;

          default:
              /* Silently ignore any unrecognized attribute codes,
                 allowing for future changes to BACKUP.
              */
	     if (debugflags & D_SUM_X) {
                fprintf(stderr, "BSA Unrecognized BSA_K backup record attribute: %d, dsize = %d\n",
                       type, dsize);
             }
             break;
       }

       // Increment the attribute byte pointer by the size of the data and
       //  the attribute header bits (bsa_dol_w_size & bsa_dol_w_type).
      attr_ptr += dsize + 4;
    }

  end_of_summary:

    printf ("Save set:          %.*s\n", (int) saveset_size, saveset);
    printf ("Written by:        %.*s\n", (int) written_by_size, written_by);
    printf ("UIC:               [%06o,%06o]\n", grp, usr);
    printf ("Date:              %.*s\n", date_length, date);
    printf ("Command:           %.*s\n", (int) command_size, command);
    printf ("Operating system:  %s version %.*s\n", os,
             (int) osversion_size, osversion);
    printf ("BACKUP version:    %.*s\n", (int) backup_version_size,
            backup_version);
    printf ("CPU ID register:   %08x\n", (unsigned int) id);
    printf ("Node name:         %.*s\n", (int) nodename_size, nodename);
    printf ("Written on:        %.*s\n", (int) written_on_size, written_on);
    printf ("Block size:        %lu\n",  blksz);

    if (grpsz != 0) {
       printf ("Group size:        %u\n", grpsz);
    }

    printf ("Buffer count:      %u\n", bufcnt);
    if (comment_size > 0) {
       printf ("Comment:           %.*s\n", (int) comment_size, comment);
    }

     /* The extra \n is to provide the blank line between the summary
        and the list of files that follows. 
      */
    printf ("\n");

 }


/* Process file attributes. */
void
process_file (unsigned char *buffer, unsigned short int rsize)
 {
    int i;
    int status;

    char *raw_att_filename;
    char *att_filename;

    unsigned long long nblk;

    unsigned long  int eof_VBN = 0;
    unsigned short int eof_VBN_low  = 0;
    unsigned short int eof_VBN_high = 0;

    unsigned long  int highest_VBN      = 0;
    unsigned short int highest_VBN_low  = 0; 
    unsigned short int highest_VBN_high = 0; 

    unsigned int dsize;

    short global_buf_cnt;
    short dtype;

    unsigned char  *data;

    char  *cfname;
    char  *sfilename;

    char  attrib_proc[80];

    char  date_bakdat[MAX_DATE_STR];
    char  date_credat[MAX_DATE_STR];
    char  date_expdat[MAX_DATE_STR];
    char  date_revdat[MAX_DATE_STR];

    unsigned short int bucket_size     = 0;
    unsigned short int max_rec_size    = 0;
    unsigned short int first_free_byte = 0;
    unsigned short int def_extend_quan = 0;
    unsigned short int att_file_ver_limit  = 0;

    unsigned int reviseno   = 0;

    unsigned int fileid1    = 0;
    unsigned int fileid2    = 0;
    unsigned int fileid3    = 0;

    unsigned int struclev   = 0;
    unsigned int strucver   = 0;
    unsigned int strucvalid = 0;

    // Layout of VMS protection bits.
    struct vms_protection {
       union {
          unsigned short protect_code;

	  struct { // Breakout of file protection fields.
	     unsigned system : 4;
             unsigned owner  : 4;
             unsigned group  : 4;
             unsigned world  : 4;
          } prot_bits;
       } prot_word;
    };

    typedef struct vms_protection VMS_protection;
    VMS_protection file_protection = {277};
    VMS_protection rec_protection;

    unsigned int protection = 0;

    char recfmt;  /* record format */

    // Breakout of VMS record attributes of a file attribute.
    static struct vms_rec_att {
       union {
	 unsigned char rec_attrib; // Record attributes.
         struct {
	   unsigned char fortran_cc :1; // FAB$V_FTN - Fortran carriage control.
           unsigned char cr_cc      :1; /* FAB$V_CR  - Carriage return carriage
                                                        control */
           unsigned char print_file :1; // FAB$V_PRN - Print carriage control
           unsigned char non_span   :1; /* FAB$V_BLK - Bit set when records 
                                                   cannot be split across block
                                                   boundaries. */
           unsigned char msb_prn    :1; /* FAB$B_MSB - the PRN code bytes 
                                              are normally LSB, set this bit to
                                              use MSB format. */
         } att_codes;
       } att_field;
    } rec_att;

      /* Various other stuff extracted from the saveset.  */
    unsigned short grp      = 0377;
    unsigned short usr      = 0377;
    long int file_uchar     = 0;

    unsigned short int file_ver_limit = 0;
    unsigned short int dir_ver_limit  = 0;
    unsigned long int alloc_file_size = 0;

     /* Number of blocks which should appear in output.  This doesn't
       seem to always be the same as nblk.  
     */
    unsigned long long int blocks;
    unsigned long long int allocated_block_file_size;

    int c;

    int  procf;

    /* Check the header word. */
    if (buffer[0] != 1 || buffer[1] != 1) {
       printf("Snark: invalid data header word 0: %d word 1: %d\n", 
              buffer[0], buffer[1]);
       fflush (stdout);

        // Exit on this error.
       exit(1);
    }

    c = 2;

    /* 
    ** Process the file attribute record. 
    */
    while (c < rsize) { 

        // Get the size of the file attribute.
       dsize = (unsigned short int) getu16 (((struct bsa *)&buffer[c])->bsa_dol_w_size);

        // Get the record attribute type.
       dtype = (unsigned short int) getu16 (((struct bsa *)&buffer[c])->bsa_dol_w_type);

       data = ((struct bsa *)&buffer[c])->bsa_dol_t_text;

       if ( debugflags & D_FILE ) {
          fprintf (stderr, "\n--process_file: file attribute record debug_dump.\n");
          debug_dump(data, dsize, dtype);
       }

       if ( !dsize ) {
          if ( debugflags & D_FILE ) {
             fprintf (stderr, "\nnull file attribute dsize; slamming the type code.\n");
          }
         dtype = 0;
        }

        /* Record the file attributes in this record.  */
       switch (dtype) {

          case 0:
            if ( debugflags & D_FILE_X ) {
               fprintf (stderr, "null file attribute dtype; ignoring this record.\n");
            }
 	    break;

          case BSA_K_FILENAME:

              /* Copy the text into filename, and '\0'-terminate it.  */
             raw_att_filename = (char *)data;
             att_filename     = filename;

             for (i = 0;
                  i < dsize && att_filename < filename + sizeof (filename) - 1;
                  i++) {
                *att_filename++ = *raw_att_filename++;
             }

             *att_filename = '\0';

	     sprintf(attrib_proc, "filename (%d): %s", BSA_K_FILENAME,
                     att_filename);

             break;

          case BSA_K_STRUCLEV:
	      // Volume file structure level.

              /* In my example, two bytes, 0x1 0x2.
                 reportedly, the structure version and the structure level
              */
             strucver = (int) *data;       // Structure version number.
             struclev = (int) *(data + 1); // Major structure level.
             strucvalid = 1;

	     sprintf(attrib_proc, "Struture version (%d): %d-%d", 
                     BSA_K_STRUCLEV, strucver, struclev);

             break;

          case BSA_K_FID:

             /* In my example, 6 bytes, 0x7a 0x2 0x57 0x0 0x1 0x1.  */
             fileid1 = getu16(data);      // Primary File file number.
             fileid2 = getu16(data + 2);  // Primary File file sequence number.
             fileid3 = getu16(data + 4);  // Primary File relative volume number.
	     sprintf(attrib_proc, "File ID (%d): %d/%d/%d", 
                     BSA_K_FID, fileid1, fileid2, fileid3);

             break;

          case BSA_K_FILESIZE:

              /* 4 bytes, 0x00000004. The allocation.  */
             alloc_file_size = getu32(data); 

	     sprintf(attrib_proc, "Allocated file size (%d): %d", 
                     BSA_K_FILESIZE, (int) alloc_file_size);

             break;

          case BSA_K_UIC:

             if (dsize == 4) {
                usr = getu16 (data);
                grp = getu16 (data + 2);
             }
	     sprintf(attrib_proc, "UIC (%d): user/grp = %d/%d", 
                     BSA_K_UIC, usr, grp);

             break;

          case BSA_K_RECATTR:
	     /* Record attribute fields:
                 See:
 
                   Figure 1-4 "DSI & FAT Structures in an XAR" Guide to
                    OpenVMS File Applications.

                   or

                   Figure 1-5 "QCP-QIO Record Attributes Area"  I/O User's
                    Reference Manual.

                0: Record type - FAT$B_RTYPE / FAB$B_RFM - unsigned char: 
                                   Bits 0-3 - FAT$V_RTYPE   - Record type. 
                                   Bits 4-7 - FAT$V_FILEORG - File organization

                1: Record attributes - FAT$B_RATTRIB / FAB$B_RAT - 
                    unsigned char.

                2: Record size in bytes - FAT$W_RSIZE- unsigned short.

                4: Highest allocated VBN - FAT$L_HIBLK - unsigned long:
                               High order bits: FAT$W_HIBLKH - unsigned short.
                               Low  order bits: FAT$W_HIBLKL - unsigned short.

                8: End of file virtual block number (VBN) - FAT$L_EFBLK - 
                    unsigned long:
                               High order bits: FAT$W_EFBLKH - unsigned short.
                               Low  order bits: FAT$W_EFBLKL - unsigned short.

               12: First free bye in FAT$L_EFBLK - FAT$W_FFBYTE - 
                    unsigned short.

               14: Number of buckets in a block - FAT$B_BKTSIZE - relative
                    and indexed files only 0 otherwise - unsigned char.

               15: Number of control bytes in VFC record - FAT$B_VFCSIZE / 
                    FAB$B_FSZ - (fixed-length control area size (FSZ) 
                    (1-255, 0 default) - unsigned char.

               16: Maximum record size in bytes FAT$W_MAXREC / FAB$L_MRS - 
                    unsigned short.

               18: Default extend quantity FATW_DEFEXT / FAB$W_DEQ - 
                    unsigned short.

               20: Global buffer count FAT$W_GBC / FAB$W_GBC 
                    (0-32,767, 0 default) - unsigned short.

               22: Spare - unsigned int.

               24: Spare - unsigned int.

               30: Version limit for directory file FAT$W_VERSIONS - 
                    unsigned short.
	     */

	     sprintf(attrib_proc, "Record attribute %d", BSA_K_RECATTR);

	     recfmt  = data[0];           // Record type.
             rec_type.rec_type_word.rec_format = data[0];

             recatt  = data[1];           // Record attributes of current file.
             rec_att.att_field.rec_attrib = data[1];

             recsize = getu16 (&data[2]); // Record size of current file

              /* Highest Virtual Block Number (VBN) - count of the number of
                  virtual blocks allocated to the file.

                 The FAT$HIBLK is an inverted format field. The high and low
                  order bits are transposed for compatibility with PDB-11
                  software.
	      */

             highest_VBN_low  = getu16 (&data[4]); // Low order  alloc VBN.
             highest_VBN_high = getu16 (&data[6]); // High order alloc VBN.

             highest_VBN      = highest_VBN_high  + (highest_VBN_low <<16); 


              /* End of File VBN - virtual block number at which the end of
                  file is located.
 
                 The FAT$L_EFBLK is an inverted format field. The high and low
                  order bits are transposed for compatibility with PDB-11
                  software.
	      */

             eof_VBN_low  = getu16 (&data[ 8]); // Low order End of file VBN. 
             eof_VBN_high = getu16 (&data[10]); // High order End of file VBN. 

             eof_VBN = eof_VBN_high + (eof_VBN_low <<16); 

              /* Adding in the following amount is a change that I brought 
                  over from vmsbackup 3.1. The comment there said "subject to 
                  confirmation from backup expert here" but I'll put it
                  in until someone complains. 
              */

             nblk    = eof_VBN_high + (64 * 1024) * eof_VBN_low; 

	      /* First free bye in FAT$L_EFBLK - count of the number of bytes
                  in use in the virtual block containing the end of file - 
                  offset to the byte of the file available for appending.
	      */ 
             first_free_byte =  getu16 (&data[12]);

              /* Bucket size - number of 512-byte blocks per 
                 bucket (0-63). 0 if not indexed or relative format file.
	      */
             bucket_size   = data[14]; 

             /* Number of control bytes in the VFC record (At least 2
                 even if not VFC file.) */
             VFC_ctr_bytes = data[15]; 
             if (VFC_ctr_bytes == 0) {
                VFC_ctr_bytes = 2;
             }

             /* Maximum record size in bytes: 

                   The size of all records in a fixed-length record file.
                   The maximum record size in a variable-length record file. 
                   The maximum size of the data area in a VFC record file.
                   The cell size (minus overhead) for relative files.

               Organization - Record Format Max Record size
                 Sequential - Fixed Length  32,767
                 Sequential - VFC           32,767 - VFC_ctr_bytes
                 Sequential - Stream        32,767
                 Sequential - Stream-CR     32,767
                 Sequential - Stream-LF     32,767
                 Sequential - Indexed       16,362
                 Relative                   16,383
	     */
             max_rec_size = getu16 (&data[16]);

	      /* Number of blocks to extend a sequential file. (Used to
                 minimize file fragmentation.)
              */
             def_extend_quan = getu16 (&data[18]); 

	      /* Number of global buffers for a file (0 - 32,767). Default 0.
                 I/O buffers for 2 or more processes to access.
	      */
             global_buf_cnt = getu16 (&data[20]); 

              // Default version limit. Valid only if file is a directory. 
             att_file_ver_limit = getu16 (&data[30]); 

             break;

          case BSA_K_BACKLINK:
	     sprintf(attrib_proc, "Back link (%d)", BSA_K_BACKLINK);

              /* In my example, 6 bytes.  hex 2b3c 2000 0000.  */
             break;

          case BSA_K_FPRO:
              /* File Protection Bitmask. */

              /* In my example, 2 bytes.  0x44 0xee.  */
             protection = getu16 (&data[0]);

             file_protection.prot_word.protect_code = getu16 (&data[0]); 
	     
	     sprintf(attrib_proc, "File Protection (FPRO) (%d): s:0x%x, o:0x%x, g:0x%x, w:0x%x", 
                     BSA_K_FPRO, 
                     file_protection.prot_word.prot_bits.system,
	             file_protection.prot_word.prot_bits.owner,
                     file_protection.prot_word.prot_bits.group,
                     file_protection.prot_word.prot_bits.world);

             break;

          case BSA_K_RPRO:
              /* Record Protection Bitmask. */
              /* In my example, 2 bytes.  hex 0000.  */

             rec_protection.prot_word.protect_code = getu16 (&data[0]);

	     sprintf(attrib_proc, "Record protection (RPRO) (%d): s:0x%x, o:0x%x, g:0x%x, w:0x%x", 
                     BSA_K_RPRO, 
                     rec_protection.prot_word.prot_bits.system,
	             rec_protection.prot_word.prot_bits.owner,
                     rec_protection.prot_word.prot_bits.group,
                     rec_protection.prot_word.prot_bits.world);

             break;

          case BSA_K_ACLEVEL:
             /* File Access Level */
	     sprintf(attrib_proc, "ACL level (%d)", BSA_K_ACLEVEL);

              /* In my example, 1 byte.  hex 00.  */
             break;

          case BSA_K_UCHAR:
              /* File characteristics */ 
              /* In my example, 4 bytes.  hex 00 0000 00.  */
	     file_uchar = getu32(data);

	     sprintf(attrib_proc, "UCHAR (%d): %d", BSA_K_UCHAR, (int) file_uchar);

             break;

          case BSA_K_VERLIMIT:

              /* In my example, 2 bytes.  Hex 01 00.  */
             file_ver_limit = getu16(data);

	     sprintf(attrib_proc, "Version Limit (%d): %d", 
                     BSA_K_VERLIMIT, file_ver_limit);

             break;

          case BSA_K_JNL_FLAGS:
	     sprintf(attrib_proc, "JNL Flags (%d)", BSA_K_JNL_FLAGS);

              /* In my example, 1 byte.  hex 00.  */
             break;

          case BSA_K_RU_ACTIVE:
	     sprintf(attrib_proc, "RU Active (%d)", BSA_K_RU_ACTIVE);

              /* In my example, 1 byte.  hex 00.  */
             break;

          case BSA_K_HIGHWATER:
	     sprintf(attrib_proc, "High water (%d)", BSA_K_HIGHWATER);
             /* Highest block that has been written to the file. */
              /* In my example, 4 bytes.  05 0000 00.  */
             break;

          case BSA_K_REVISION:

              /* In my example, 2 bytes.  04 00.  */
             reviseno = getu16 (&data[0]);

	     sprintf(attrib_proc, "Revision (%d): %d", BSA_K_REVISION,
                     reviseno);

             break;

          case BSA_K_CREDATE:
	     sprintf(attrib_proc, "Creation date (%d)", BSA_K_CREDATE);

             create_time = time_vms_to_unix_asc( data, 8, date_credat, NULL );
             break;

          case BSA_K_REVDATE:
	     sprintf(attrib_proc, "Revision date (%d)", BSA_K_REVDATE);

             mod_time = time_vms_to_unix_asc( data, 8, date_revdat, NULL );
             break;

          case BSA_K_EXPDATE:
	     sprintf(attrib_proc, "Expiration date (%d)", BSA_K_EXPDATE);

             time_vms_to_unix_asc( data, 8, date_expdat, NULL );
             break;

          case BSA_K_BAKDATE:
	     sprintf(attrib_proc, "Backup date (%d)", BSA_K_BAKDATE);

             time_vms_to_unix_asc( data, 8, date_bakdat, NULL );
             break;

          case BSA_K_DIR_UIC:
	     sprintf(attrib_proc, "Directory UIC Protection (%d)", BSA_K_DIR_UIC);

              /* In my example, 4 bytes.  01 00c6 00.  */
             break;

          case BSA_K_DIR_FPRO:
	     sprintf(attrib_proc, "Directory FPRO (%d)", BSA_K_DIR_FPRO);

              /* In my example, 2 bytes.  88 aa.  */
             break;

          case BSA_K_DIR_VERLIM:

              /* In my example, 2 bytes.  01 00.  */
             dir_ver_limit = getu16(data);

	     sprintf(attrib_proc, "Directory version limit (%d): %d ", 
                     BSA_K_VERLIMIT, dir_ver_limit);

             break;

           /* then comes 0x0, offset 0x2b7.  */

          case BSA_K_RETAINMIN:
	     sprintf(attrib_proc, "Minimum file retention (%d)", BSA_K_RETAINMIN);

             break;

          case BSA_K_RETAINMAX:
	     sprintf(attrib_proc, "Maximum file retention (%d)", BSA_K_RETAINMAX);

             break;

          case BSA_K_ACLSEGMENT:
             sprintf(attrib_proc, "ACL Segment (%d)\n", BSA_K_ACLSEGMENT);

             break;

          default:
              /* unexpected or unrecognized code */
	     if ( debugflags & D_FILE_X )
	         sprintf(attrib_proc, 
                    "Unrecognized or unexpected BSA_K record attribute type code:  0x%x/%d\n", 
                    dtype, dtype);

            break;

       } /* Record the file attributes in this record.  */

       if (debugflags & D_FILE) {
          fprintf(stderr, "Processing %s\n", attrib_proc);
       }

       c += dsize + 4;

    } /* Process the File attribute record. */


    if (debugflags & D_FILE) {
       fprintf(stderr, "filename = %s\n", filename );
       fprintf(stderr, " Record format:           %d\n", recfmt);
       fprintf(stderr, "  Record type:            %d\n",
              rec_type.rec_type_word.rectype_bits.rtype);
       fprintf(stderr, "  File organization:      %d\n",
              rec_type.rec_type_word.rectype_bits.fileorg);
       fprintf(stderr, " Record attributes:       %d\n", recatt);
       fprintf(stderr, " Record size:             %d\n", recsize);
       fprintf(stderr, " Highest block:           %d\n", (int) highest_VBN);
       fprintf(stderr, " End of file block:       %d\n", (int) eof_VBN);
       fprintf(stderr, " End of file byte:        %d\n", first_free_byte);
       fprintf(stderr, " Bucket size:             %d\n", bucket_size);
       fprintf(stderr, " Fixed control area size: %d\n", VFC_ctr_bytes);
       fprintf(stderr, " Maximum record size:     %d\n", max_rec_size);
       fprintf(stderr, " Default extension size:  %d\n", def_extend_quan);
       fprintf(stderr, " Global buffer count:     %d\n", global_buf_cnt);
       fprintf(stderr, " Directory version limit  %d\n", att_file_ver_limit);

       if ( strucvalid ) {
          fprintf(stderr, " Structure version:       %d\n", strucver);
          fprintf(stderr, " Structure level:         ODS-%d\n\n", struclev);
       }
    }

     /* I believe that "512" here is a fixed constant which should not
         depend on the device, the saveset, or anything like that. 
     */
     /*  Last block is only partially used. Use the first free byte 
        (FAT$W_FFBYTE) to determine how many bytes in the last block are
        part of the file.
     */
    allocated_block_file_size = highest_VBN;
    allocated_bytes_file_size = allocated_block_file_size*VMS_BLOCK;
    if ( nblk > 0 ) {
            filesize  = (nblk-1)*VMS_BLOCK + first_free_byte;
	    blocks    = (filesize + (VMS_BLOCK-1)) / VMS_BLOCK;
    } else if (allocated_bytes_file_size > 0) {
	    // Use the allocated file size if the file size is bad. - Indexed files
	    // use different formula.
            filesize  = allocated_bytes_file_size;
	    blocks    = allocated_block_file_size;
     } else {
       printf("vmsbackup - file sizing error - unexpected nblk = %lld and allocated_bytes_file_size = %lld\n",
		       nblk, allocated_bytes_file_size );
       fflush (stdout);
       exit( 1 );
     }

    if (debugflags & D_FILE)  {
       fprintf(stderr, "Number of blocks = %lld, Highest block = %d, first free block = %d\n",
              nblk, highest_VBN_high, first_free_byte);
       fprintf(stderr, "File Size: 0x%llx/%lld, Allocated File Size Calc:%lld Recorded:%d\n",
              filesize, filesize, allocated_bytes_file_size, (int) alloc_file_size);
    }


     /* Close the prior extraced output file. */
    if (fptr_out != NULL) {
        // Close the file and set its time stamps.
       status = close_file (fptr_out, unix_filename, 
                            last_create_time, last_mod_time);

       file_byte_count = 0;
       reclen     = 0;
       fptr_out   = NULL;
    }

    procf = 0;
    if (goptind < gargc) {

       if (dflag) {
           // Include the directory path.
          cfname = filename;
       } else {
          cfname = strrchr(filename, ']') + 1;
       }

       sfilename = malloc (strlen (cfname) + 5);

       if (sfilename == NULL) {
          fprintf (stderr, "process_file - out of memory\n");
          fflush (stdout);

           // Exit on error.
          exit (1);
       }

       if (cflag) {
	   // Use complete file name including the version number.
          for (i = 0; i < strlen(cfname); i++) {
             sfilename[i]   = cfname[i];
             sfilename[i+1] = '\0';
          }

       } else {
	   // Strip off the version number.
          for (i = 0;
               i < strlen(cfname) && cfname[i] != ';';
               i++) {
             sfilename[i] = cfname[i];
             sfilename[i+1] = '\0';
          }
       }

        // Now match the selected file against the requested file.
       for (i = goptind; i < gargc; i++) {
          procf |= match (strlocase(sfilename),
          strlocase(gargv[i]));

       }
        // Release the temporary filename memory.
       free (sfilename);

    } else {
       procf = 1;
    }

    if (tflag && procf && !flag_full) {
// #ifdef __VMS
//       printf ("%-52s %lld %s\n", filename, blocks, date4);
// #else  /* __VMS */
       printf ("%-52s %lld %s\n", filename, blocks, date_revdat);
// #endif
    }

     // Report the full file info.
    if (tflag && procf && flag_full) {
       char fidbuf[20];

       printf ("%-132.132s\n", filename);
       printf ("  Size:       %6lld/%-6lld\tCreated: %s\n",
               blocks,allocated_block_file_size, date_credat );
       printf ("  Owner:    [%06o,%06o]\tRevised: %s (%u)\n",
               grp, usr ,date_revdat, reviseno);
       sprintf( fidbuf, "(%d,%d,%d)", fileid1, fileid2, fileid3);
       printf ("  File ID: %16.16s\tExpires: %s\n", fidbuf,date_expdat);
       printf ("\t\t\t\tBackup:  %s\n", date_bakdat);

       printf ("  Protection: (");
       for (i = 0; i <= 3; i++) {
          printf("%c:", "SOGW"[i]);

          if (((protection >> (i * 4)) & 1) == 0) {
             printf("R");
          }

          if (((protection >> (i * 4)) & 2) == 0) {
             printf("W");
          }

          if (((protection >> (i * 4)) & 4) == 0) {
             printf("E");
          }

          if (((protection >> (i * 4)) & 8) == 0) {
             printf("D");
          }

          if (i != 3) {
             printf(",");
          }
       }

       printf(")\n");

       printf ("  File Organization:  ");
       switch (rec_type.rec_type_word.rectype_bits.fileorg) { 

          case FAT_C_SEQUENTIAL: 
             printf("Sequential"); 
             break;

          case FAT_C_RELATIVE: 
             printf("Relative"); 
             break;

          case FAT_C_INDEXED: 
             printf("Indexed"); 
             break;

          case FAB_C_DIRECT:  // Not currently implemented in VMS.
             printf("Hashed"); 
             break;

          default: 
             printf("<Unknown %d>", 
                    rec_type.rec_type_word.rectype_bits.fileorg); 
             break;
       }

       printf("\n");

       printf("  File attributes:    Allocation %llu, Extend %d", 
              allocated_block_file_size, def_extend_quan);

       if (rec_type.rec_type_word.rectype_bits.fileorg == FAT_C_INDEXED) {
          printf (", Block size %d", bucket_size);
       }
       printf("\n");

       printf ("  Record format:      ");
       switch (rec_type.rec_type_word.rectype_bits.rtype) {

          case FAB_C_UDF: 
             printf ("(undefined)"); 
             break;

          case FAB_C_FIX: 
             printf ("Fixed length");
             if (recsize) {
                printf (" %u byte records", recsize);
             }
             break;

          case FAB_C_VAR: 
             printf ("Variable length");
             if (recsize) {
                printf (", maximum %u bytes", recsize);
             }
             break;

          case FAB_C_VFC: 
             printf ("VFC"); 
             if (recsize) {
                printf (", maximum %u bytes", recsize);
             }
             break;

          case FAB_C_STM: 
             printf ("Stream"); 
             break;

          case FAB_C_STMLF: 
             printf ("Stream_LF"); 
             break;

          case FAB_C_STMCR: 
             printf ("Stream_CR"); 
             break;

          default: 
             printf ("<unknown> %d", 
                     rec_type.rec_type_word.rectype_bits.rtype); 
             break;
       }

       printf ("\n");

       printf ("  Record attributes:  ");
       if (recatt & FAB_M_FTN) {
          printf ("Fortran ");
       }

       if (recatt & FAB_M_PRN) {
          printf ("Print file ");
       }

       if (recatt & FAB_M_CR) {
          printf ("Carriage return carriage control ");
       }

       if (recatt & FAB_M_BLK) {
          printf ("Non-spanned");
       }

       printf ("\n\n");
    } // Report the full file info.

    if (xflag && procf) {
        /* Extract the file from the backup saveset and write to disk. */
       fptr_out = openfile(filename,struclev);


       if ( fptr_out!= NULL && vflag) {
    	  if (outfile)
    		  printf("extracting: %s\n        to: %s\n",
    				  filename, outfile);
    	  else
    		  printf("extracting: %s\n        to: %s\n",
                 filename, unix_filename);
       }

    }

    ++nfiles;
    nblocks += blocks;
 }


/*
 *
 *  Process a virtual block record (file record).
 *
 */
void
process_vbn(unsigned char *buffer, unsigned short rsize)
 {
    int c;
    int i;
    int j;
    int status;

    if (fptr_out == NULL) {
       return;
    }

    i = 0;

    if (debugflags & D_VBN) {
       fprintf (stderr, "Writing output file %s count: %d, size: %lld, recsize: %d\n",
               unix_filename, file_byte_count, filesize, rsize);
    }

    while ((file_byte_count+i < filesize) && (i < rsize)) {

       if (debugflags & D_VBN) {
            fprintf(stderr, "Variable-length record code: 0x0%x\n",
		rec_type.rec_type_word.rectype_bits.rtype );
       }

        // Process the record type codes (FAB$B_RFM).
       switch (rec_type.rec_type_word.rectype_bits.rtype) {

          case FAB_C_FIX: 	/* FAB$C_FIX - fixed-length record format.*/
	     if (debugflags & D_VBN_X) {
                fprintf(stderr, "Writing fixed-length records\n");
             }

             if (reclen == 0) {
                reclen = recsize;
             }

             fputc(buffer[i], fptr_out);
             i++;
             reclen--;
             break;

          case FAB_C_VAR: /* FAB$C_VAR - variable-length record format. */
	     if (debugflags & D_VBN_X) {
                fprintf(stderr, "Writing variable-length records\n");
             }

          case FAB_C_VFC: /* FAB$C_VFC - variable-length with fixed-length
                                         control record format. This format is
                                         not supported for indexed files.*/
	     if (debugflags & D_VBN_X) {
                fprintf(stderr, "Writing VAR/VFC records\n");
             }
	     	 if (needToSkipVFC) {
                 if (flag_binary) { // && rec_type.rec_type_word.rectype_bits.rtype == FAB_C_VFC is true otherwise needToSkipVFC would be false
		         // If binary mode requested just write the data out.
                    for (j = 0; j < VFC_ctr_bytes; j++) {
                       fputc (buffer[i+j], fptr_out);
                    }
                 }
	     		 needToSkipVFC = 0;
	     		 i += VFC_ctr_bytes;
	     		 reclen -= VFC_ctr_bytes;
	     	 }
             if (reclen <= 0) {
	         // Get the record length from the buffer.
                reclen = getu16 (&buffer[i]);
#ifdef NEWD
                fprintf(lf, "---\n");
                fprintf(lf, "reclen = %d\n", reclen);
                fprintf(lf, "i = %d\n", i);
                fprintf(lf, "rsize = %d\n", rsize);
#endif
                fix = reclen;

	         // If binary mode requested then just write the next two
                 //  bytes out.
                if (flag_binary) {
                   for (j = 0; j < 2; j++) {
                      fputc (buffer[i+j], fptr_out);
                   }
                }

                i += 2; // Skip over the next two bytes.

                if (i >= rsize) {
                	needToSkipVFC = 1;
                	break;
                }

                if (rec_type.rec_type_word.rectype_bits.rtype == FAB_C_VFC) {
                   if (flag_binary) {
		       // If binary mode requested just write the data out.
                      for (j = 0; j < VFC_ctr_bytes; j++) {
                         fputc (buffer[i+j], fptr_out);
                      }
                   }

		    // Increment the buffer counter by the VFC record size.
                   i      += VFC_ctr_bytes;

		    // Decrement the record length by the number of bytes 
                    //  already written.
                   reclen -= VFC_ctr_bytes;
                   if (i >= rsize)
                	   break;

                }
   
	     } else if ((reclen == fix) && (recatt == FAB_M_FTN)) {
     /****
                if (buffer[i] == '0') {
                   fputc('\n', fptr_out);
                } else if (buffer[i] == '1') {
                   fputc('\f', fptr_out);
                }
     *** sow ***/
	         // For fixed length fortran control just write the records
                 //  out.
                fputc(buffer[i],fptr_out); /** sow **/

                i++;
                reclen--;

             } else {
	        // If we know the record size just write the data out.
                fputc(buffer[i], fptr_out);

                i++;
                reclen--;
             }

             if (reclen == 0) {

                if (!flag_binary) {
		    // Add a CR to this nonbinary mode file.
                   fputc('\n', fptr_out);    
                }

                if (i & 1) {
		    // If binary mode requested always write out the current 
                    //  record.
                   if (flag_binary) {
                      fputc (buffer[i], fptr_out);
                   }

                   i++;
                }
             }

             break;

       case FAB_C_STM: /* FAB$C_STM - stream record format. Records are  
                                      delimited by FF, VT, LF or CR LF, and all
                                      leading zeros are ignored.*/
	  if (debugflags & D_VBN_X) {
             fprintf(stderr, "Writing STM records\n");
          }

       case FAB_C_STMLF: /* FAB$C_STMLF - stream sequential with linefeed; 
                                          used by C and Unix */
	  if (debugflags & D_VBN_X) {
             fprintf(stderr, "Writing STMLF records\n");
          }

          if (reclen < 0) {
             fprintf(stderr, "Error - Negative record length %d\n", reclen);
          }

          if (reclen == 0) {
	      // Assume 512 byte records.
             reclen = VMS_BLOCK;
          }

          c = buffer[i++];
          reclen--;

	  // CR indicates at the end of the record.
          if (c == '\n') {
             reclen = 0;
          }

           // Just write the record out.
          fputc(c, fptr_out);
          break;

       case FAB_C_STMCR: /* FAB$C_STMCR - stream sequential with carriage 
                                          control */

	  if (debugflags & D_VBN_X) {
             fprintf(stderr, "Writing STMCR records\n");
          }

          c = buffer[i++];

          if (c == '\r' && !flag_binary) {
	      // If binary mode not requested change CR to LF for Unix.
             fputc('\n', fptr_out);
          } else {
             fputc(c, fptr_out);
          }

          break;

       default: /* FAB$C_UDF - unknown VMS record type. */

          /* On error delete the output file. */
          status = fclose(fptr_out);
          remove(unix_filename);

          fflush (stdout);

          fprintf(stderr, "Invalid VMS record format = %d for file %s\n",
                  rec_type.rec_type_word.rectype_bits.rtype, filename);

          fptr_out = NULL;

          return;

       } // Process the record type codes (FAB$B_RFM).

    }

    file_byte_count += i;

}


/*
 *
 *  Process a VMS backup block.
 *
 */
void
process_block(unsigned char *block, int file_blocksize)
 {

    unsigned short int bhsize;
    unsigned short int rsize;
    unsigned short int rtype;

    unsigned long  bsize;
    unsigned long  buff_ptr = 0;
    unsigned short int  bapplic;


     // VMS backup block header.
    struct bbhdef *block_header;
    int            alloc_block_header_size =  sizeof(*block_header);

     // VMS backup record header. 
    struct brhdef  *record_header;
    int            alloc_record_header_size = sizeof(*record_header);


     // Map the file data block into the backup block header struture. 
    block_header  = (struct bbhdef *) &block[buff_ptr];

     // Move the buffer pointer ahead past this block header.
    buff_ptr += alloc_block_header_size;

     // Get the recorded block header size in bytes.
    bhsize = getu16 (block_header->bbh_dol_w_size);

     // Get the number of bytes in the next block.
    bsize  = getu32 (block_header->bbh_dol_l_blocksize);

     /* check the validity of the header block. */
    if (bhsize != alloc_block_header_size) {
       fprintf (stderr,
                "Invalid header block size - expected: %d got: %d\n",
                alloc_block_header_size,
                bhsize);
       fflush (stdout);

        // Exit on error.
       exit(1);
    }

     // Expected size of the data block should be the same as the number
     //  of bytes read in.
    if ((bsize != 0) && (bsize != file_blocksize)) {
       fprintf(stderr, "Snark: Invalid block size - expected: %d got: %d\n",
               file_blocksize, (int) bsize);
       fflush (stdout);

        // Exit on error.
       exit(1);
    }

    bapplic = getu16 (block_header->bbh_dol_w_applic);
    if (debugflags & D_BLKREC) {
#define STRINGIFY(m) #m
       unsigned short int bopsys, bsubsys;
	   unsigned long bnumber;
       char *copsys, *csubsys, *capplic;
       bnumber = getu32 (block_header->bbh_dol_l_number);
       fprintf(stderr, "\nblock header\n file offset   = 0x%x\n size          = %d\n",
    		   (bnumber-1)*bsize, (int) bsize);
       bopsys = getu16 (block_header->bbh_dol_w_opsys);
       switch (bopsys) {
       case	BACKUP_K_OPSYS_VAX: copsys = &STRINGIFY(BACKUP_K_OPSYS_VAX)[15];
    		   break;
       case	BACKUP_K_OPSYS_ALPHA: copsys = &STRINGIFY(BACKUP_K_OPSYS_ALPHA)[15];
    		   break;
       case	BACKUP_K_OPSYS_IA64: copsys = &STRINGIFY(BACKUP_K_OPSYS_IA64)[15];
    		   break;
       default: copsys = "unknown";
    		   break;

       }
       bsubsys = getu16 (block_header->bbh_dol_w_subsys);
       switch (bsubsys) {
       case	BACKUP$K_BACKUP: csubsys = &STRINGIFY(BACKUP$K_BACKUP)[9];
    		   break;
       default: csubsys = "unknown";
    		   break;

       }
       switch (bapplic) {
       case	BACKUP$K_DATABLOCK: capplic = &STRINGIFY(BACKUP$K_DATABLOCK)[9];
    		   break;
       case	BACKUP$K_XORBLOCK: capplic = &STRINGIFY(BACKUP$K_XORBLOCK)[9];
    		   break;
       default: capplic = "unknown";
    		   break;

       }
       fprintf(stderr, " opsys         = %d (%s)\n subsys        = %d (%s)\n applic        = %d (%s)\n",
    		   bopsys, copsys, bsubsys, csubsys, bapplic, capplic);
       fprintf(stderr, " number        = %d\n", bnumber);
    }
    /* skip an XORBLOCK */
    if (bapplic==BACKUP$K_XORBLOCK)
    	return;
     /* Read the records. */
    while (buff_ptr < bsize) {
        /* Read the backup record header. */
       if (debugflags & D_BLKREC) {
          fprintf(stderr, "\nrecord header\n");
       }

       record_header = (struct brhdef *) &block[buff_ptr];

        // Move the pointer past the end of the block header.
       buff_ptr     += alloc_record_header_size;

        // Get the size of the record.
       rsize = getu16 (record_header->brh_dol_w_rsize);

        // Get the record type.
       rtype = getu16 (record_header->brh_dol_w_rtype);

       if (debugflags & D_BLKREC) {
          fprintf(stderr, " block offset  = 0x%x\n", (int) buff_ptr);
          fprintf(stderr, " rsize         = %d (bytes)\n",   rsize);
          fprintf(stderr, " type          = %d\n",   rtype);
          fprintf(stderr, " flags         = 0x%x\n",
                 (int) getu32 (record_header->brh_dol_l_flags));
          fprintf(stderr, " address       = 0x%x\n",
                 (int) getu32 (record_header->brh_dol_l_address));
          fprintf(stderr, " blockflags    = 0x%x\n",
                 (int) getu32 (record_header->brh_dol_l_blockflags));
       }

    
       if (debugflags & D_BLKREC) {
          fprintf(stderr, " rtype         = ");
       }
       switch (rtype) { /* Process each DOL record type. */

          case brh_dol_k_null:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "null\n");
             }

              /* This is the type used to pad to the end of a block.  */
             break;

          case brh_dol_k_summary:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "summary\n");
             }

              /* there can be only one; ignore the rest. */
             if ( summary_seen++ ) {
                break;
             }

             // Process the VMS backup summary record.
             process_summary (&block[buff_ptr], rsize);
             break;

          case brh_dol_k_file:
               /* Process file attributes. */
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "file\n");
             }

             process_file(&block[buff_ptr], rsize);
             break;

          case brh_dol_k_vbn:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "vbn\n");
             }

              // Process a virtual block record (file record).
             process_vbn(&block[buff_ptr], rsize);
             break;

          case brh_dol_k_physvol:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "physvol\n");
             }
             break;

          case brh_dol_k_lbn:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "lbn\n");
             }
             break;

          case brh_dol_k_fid:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "fid\n");
             }
             break;

          case brh_dol_k_file_ext:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "file_ext (unsupported)\n");
             }
             break;

          case brh_dol_k_lbn_576:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "lbn_576 (unsupported)\n");
             }
             break;

          case brh_dol_k_rs_dirattr:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "rs_dirattr (unsupported)\n");
             }
             break;

          case brh_dol_k_alias:
	     if (debugflags & D_BLKREC) {
                fprintf(stderr, "alias (unsupported)\n");
             }
             break;

          default:
              /*
              **  It is quite possible that we should just skip
              **  this without even printing a warning. 
              */
             if (debugflags & D_BLKREC) {
                fprintf (stderr, " Warning: unrecognized record type\n");
                fprintf (stderr, " Allowed record types: 0 to %d\n", 
                         brh_dol_k_max_rtype-1);
                fprintf (stderr, " rtype: %d, rsize: %d\n",
                         rtype, rsize);
             }
             break;

       } /* Process each record type. */

       if ( summary_seen > 1 ) {
          /*
          ** This shouldn't happen and it may be better to terminate with
          ** a proper exit code.
          */
          summary_seen = 1;
          fprintf(stderr, "secondary summary record found: rest of block skipped\n");
          break;
       }

#ifdef pyr
       buff_ptr  = buff_ptr + rsize;
#else
       buff_ptr += rsize;
#endif

    }  /* Read the records. */
 }


/* Read the VMS header records. */
int
rdhead(unsigned char *block, int *rec_blocksize)
 {
    int i;
    int nfound         = 1;
    int max_rec_size   = 0;
    int max_block_size = 0;
    int label_count    = 0;

    char label[LABEL_SIZE];
    char vol_name[80];
    char hdr1_name[80];
    char rec_format[1];


      /* Read the tape label - 4 records of 80 bytes. */
    while ((i = read(fd, label, LABEL_SIZE)) != 0) {

       if (i != LABEL_SIZE) {
          fprintf(stderr, 
                  "Snark: bad head label record - Expected: %d bytes found: %d\n",
                  LABEL_SIZE, i);
          fflush (stdout);

           // Report EOT found.
          return (1);
       }

       label_count += 1;

       if (strncmp(label, "VOL1",4) == 0) {
	  if (debugflags & D_IO) {
             fprintf (stderr, "\nFirst Volume Label:\n%s\n", label);
          }

          sscanf(label+4, "%17s", vol_name);

          if (vflag || tflag) { 
             fprintf(stderr, "Volume: %s\n", vol_name);
          }
       }

       if (strncmp(label, "HDR1",4) == 0) {
	  if (debugflags & D_IO) {
             fprintf (stderr, "\nFirst Header Label: \n%s\n", label);
          }

          sscanf(label+4, "%14s", hdr1_name);
          sscanf(label+31, "%4d", &setnr);
       }

        /* Get the block size. */
       if (strncmp(label, "HDR2", 4) == 0) {
          if (debugflags & D_IO) {
             fprintf (stderr, "\nSecond Header Label:\n %s\n", label);
          }

          nfound = 0;

          sscanf(label+4, "%1s", rec_format);
          if (vflag || tflag) {
             printf("Rec format: %s\n", rec_format);
          }

          sscanf(label+5, "%5d", &max_rec_size);
          if (debugflags & D_IO) {
             fprintf(stderr, "Max rec size = %d\n", max_rec_size);
          }

          sscanf(label+10, "%5d", &max_block_size);
          if (debugflags & D_IO) {
             fprintf(stderr, "Max block size = %d\n", max_block_size);
          }
       }
    } /* Read the tape label - 4 records of 80 bytes. */

    if (debugflags & D_IO) {
       fprintf (stderr, "Read %d labels\n", label_count);
    }

    if ((vflag || tflag) && !nfound) {
       printf("\n%s Saveset name: %s   Save set number: %d\n", 
              vol_name, hdr1_name,setnr);
    }

    *rec_blocksize = max_rec_size;

     // Release the initial block buffer memory.
    if (sizeof (block) > 0) {
       free (block);
    }

     /* Reallocate the block buffer using the header block size. */
    block = (unsigned char *) malloc(*rec_blocksize);
    if (block == (unsigned char *) 0) {
       fprintf(stderr, "memory allocation for %d block failed\n", 
               *rec_blocksize );
       fflush (stdout);

        // Exit on error.
       exit(1);
    }

    return(nfound);
 }


/* Read the VMS tape label records. */
void
rdtail()
 {
    int i;

    char label[LABEL_SIZE];
    char name[80];

     /* Read the tape label - 4 records of 80 bytes. */
    while ((i = read(fd, label, LABEL_SIZE)) != 0) {
       if (i != LABEL_SIZE) {
          fprintf(stderr, 
                  "Snark: bad tail label record - Expected: %d bytes found: %d\n",
                  LABEL_SIZE, i);
	  //          fflush (stdout);

           // Exit on error.
	  //          exit(1);
          return;
       }

       if (strncmp(label, "EOF1",4) == 0) {
          sscanf(label+4, "%14s", name);
          if (vflag || tflag) {
             printf("End of saveset: %s\n\n\n",name);
          }
       }
    }  /* Read the tape label - 4 records of 80 bytes. */
 }


/* Perform the actual operation.  The way this works is that main () parses
   the arguments, sets up the global variables like cflags, and calls us.
   Does not return--it always calls exit ().  */
void
vmsbackup()
 {
    int i;
    int n;
    int eoffl;
    int status;

    unsigned char *block;

     /* Nonzero if we are reading from a saveset on disk (as
        created by the /SAVE_SET qualifier to BACKUP) rather than from
         a tape. 
     */
    int ondisk = 0;

    if (tapefile == NULL) {
       tapefile = def_tapefile;
    }

#ifdef NEWD
     /* open debug file */
    lf = fopen("log", "w");
    if (lf == NULL) {
       fflush (stdout);
       perror("log");

        // Exit on error.
       exit(1);
    }
#endif

    /* Open the backup device. */
    if (0==strcmp(tapefile,"-"))
       fd = dup(STDIN_FILENO);
    else
       fd = open(tapefile, O_RDONLY);
    if (fd < 0) {
       printf ("Error accessing tape drive: ");
       fflush (stdout);
       perror(tapefile);

        // Exit on error.
       exit(1);
    }

#if HAVE_MT_IOCTLS
     /* rewind the tape */        
    op.mt_op    = MTREW;
    op.mt_count = 1;

    if ( debugflags & D_IO) {
       fprintf(stderr, "determining disk or tape; attempting rewind\n");
    }

    i = ioctl(fd, MTIOCTOP, &op);

    if (i < 0) {
       if (errno == EINVAL || errno == ENOTTY) {
          ondisk = 1;

	  if ( debugflags & D_IO) {
             fprintf(stderr, "Reading from disk\n");
          }

       } else {
          fflush (stdout);
          perror(tapefile);

	   // Exit on error.
          exit(1);
       }
    }
     
    if ( debugflags & D_IO) {
       fprintf(stderr, "IOCTL rewind status = %d\n", i);
    }

#else
     // Reading VMS backup file from on disk.
    ondisk = 1;
#endif

     /* process_block wants this to match the size which
         backup writes into the header.  Should it care in
         the ondisk case? 
     */ 
    block = (unsigned char *) malloc (blocksize);

    if (block == (unsigned char *) 0) {
       fflush (stdout);
       fprintf(stderr, "vmsbackup - Memory allocation for block failed\n");

        // Exit on error.
       exit(1);
    }

#if HAVE_MT_IOCTLS
    if (ondisk) {

       eoffl = 0;

    } else {

        /* Read the VMS header records and intialize the block buffer.*/
       eoffl = rdhead(block, &blocksize);

         /* Skip over the Tape Mark (TM) after the header records. */
       op.mt_op    = MTFSF;
       op.mt_count = 1;

       printf("Skipping EOF with ioctl MTFSF!\n");
       i = ioctl(fd, MTIOCTOP, &op);

       if (i < 0) {
	   // Error skipping first TM.
          printf("Error skipping EOF - status:%d ", i);
          fflush (stdout);

          perror(tapefile);

           // Exit on error.
          exit(1);
       }
    }
#else
    eoffl = 0;
#endif

    nfiles  = 0;
    nblocks = 0;

     /* Read the backup tape blocks until end of tape. */ 
    while (eoffl == 0) {

        // Find the selected VMS backup saveset. 
       if (sflag && setnr != selset) {

          if (ondisk) {
             fprintf(stderr, "-s not supported for disk savesets\n");
             fflush (stdout);

	      // Exit on error.
             exit(1);
          }

#if HAVE_MT_IOCTLS
          op.mt_op    = MTFSF;
          op.mt_count = 1;

          printf("Skipping EOF with ioctl MTFSF!\n");

          i = ioctl(fd, MTIOCTOP, &op);
          if (i < 0) {
             fflush (stdout);
             perror(tapefile);

	      // Exit on error.
             exit(1);
          }
#else
          abort ();
#endif
          i = 0;

       } else {

           // Process the current saveset.
          i = read(fd, block, blocksize);

       } // Find the selected VMS backup saveset. 

       /* Retry read upto MAX_RETRYS before giving up. */
       if (i == -1) {
	 for (n = 0; n < MAX_RETRYS; n++) {
             i = read(fd, block, blocksize);
             if (i != -1) {
	       n = MAX_RETRYS;
	     }
         }
       }

        // Check the saveset block record.
       if (i == 0) {
          if (ondisk) {
              /* No need to support multiple save sets.  */
             eoffl = 1;

          } else {
	     if (vflag || tflag) {
                printf ("\nTotal of %u files, %lu blocks\n", nfiles, nblocks);
             }

	      // Read the VMS tape label records.
             rdtail();

	     /* Read the VMS header records for the next save set and 
                 initialize the block buffer.
	     */
             eoffl = rdhead(block, &blocksize);
          }

       } else if (i == 3) {
          printf ("\nTried to read %d blocks\n", blocksize);
          fflush (stdout);
          perror ("\n error reading saveset");

           // Exit on error.
          exit (1);

       } else if (i != blocksize) {
          fflush (stdout);

          fprintf(stderr, 
                  "bad block read - expected: %d got: %d blocks\n", 
                  blocksize, i);

           // Exit on error.
          exit(1);

       } else {
          eoffl = 0;

	   // Process a VMS backup block.
          process_block(block, blocksize);

       }  // Check the saveset block record.

    } /* Read the backup tape blocks until end of tape. */ 

     /* Close the last extracted file. */
    if (fptr_out != NULL) {

        // Close the file and set its time stamps.
       status = close_file (fptr_out, unix_filename, 
                            last_create_time, last_mod_time);

       fptr_out   = NULL;
    }

    if (vflag || tflag) {

       if (ondisk) {
          printf ("\nTotal of %u files, %lu blocks\n", nfiles, nblocks);
          printf("End of save set\n");

       } else {
          printf ("\nTotal of %u files, %lu blocks\n", nfiles, nblocks);
          printf("End of tape\n");
       }
    }

    close(fd);

#ifdef NEWD
     /* close debug file */
    fclose(lf);
#endif

     /* exit cleanly */
    exit(0);
 }




static void debug_dump(const unsigned char* buffer, unsigned short int dsize, 
                       unsigned short int dtype)
 {
    long long max_dump_size;
    long long cur_dump_dsize = 0;

    // Determine the maximum number of bits that can be dumped.
    max_dump_size = (long long)buffer + sizeof(buffer) * 8;

    if (debugflags) {
       char comma = ' ';

       fprintf(stderr, "debug_dump: sizeof(buffer): %d, dsize: 0x%x/%u, dtype: 0x%x/%d: \n        ",
              (int) sizeof (buffer), dsize, dsize, dtype, dtype);

       while ((dsize-- > 0) && ((long long)buffer < max_dump_size)) {
          fprintf(stderr, "%c 0x%x/%d", comma, (int) *buffer, (int) *buffer);
          buffer++;
          comma = ',';

	  cur_dump_dsize++;
          if (!(cur_dump_dsize & 0x07)) {
             fprintf(stderr, ",\n        ");
             comma = ' ';
          }
       }

       fprintf(stderr, "\n\n");
    }
 }


/* 
*   convert the OpenVMS epoch to the Unix epoch, punting on
*   select special cases
*/
time_t 
time_vms_to_unix(unsigned long long vms_time_ll) 
 {
     /* On a 64 bit machine the VMS time is a Quadword long integer 
        repesenting the number of 100-nanosecond ticks since epoch.
     */
    unsigned long long byte_swapped_ll = 0;
    time_t unix_time;

#if !defined (__SVR4) && !defined (__sun)
    int altzone = 0;    
    daylight = 0;
    timezone = 0;
#endif

    if (debugflags & D_TIME) {
       fprintf(stderr, "time_vms_to_unix()     OpenVMS time in seconds (input) = %lld\n",
              vms_time_ll);
       fprintf(stderr, "                       daylight %d, timezone %d, altzone %d\n",
              daylight, (int) timezone, altzone );
    }

    byte_swapped_ll = getu64((unsigned char *)&vms_time_ll);

    if (debugflags & D_TIME) {
       fprintf(stderr, "                       OpenVMS time in seconds (swizzled) = %lld\n",
              byte_swapped_ll);
    }

    /*  OpenVMS dates can be negative (delta time), zero (17-Nov-1958; the base date)
        or a positive value.  The Unix epoch has zero as its base date (1-Jan-1970), 
        which means that a whole range of OpenVMS dates can't be presented.  Basically,
        any positive value less than 0x07c95674beb4000ull (1-Jan-1970 00:00 in the
        OpenVMS quadword format) or zero or any delta time can't be presented in the
        Unix epoch.  So punt, and return a zero.
     */
    if (byte_swapped_ll < 0x07c95674beb4000ull ) {
       return (time_t) 0;
    }

     /* Epoch for VMS  is 17-Nov-1858 00:00 (local time).
        Epoch for UNIX is  1-Jan-1970 00:00 (GMT/UTC).
        In C time_t holds the number of seconds since the Unix epoch. 

         Subtract the centisecond OpenVMS value for 1-Jan-1970 00:00 (the
         Unix epoch) and then divide by enough to convert the centiseconds
         of OpenVMS quadword to the seconds of the Unix epoch.  Done.
     */
    byte_swapped_ll -= 0x07c95674beb4000ull;
    byte_swapped_ll /= 10000000;

    unix_time = (time_t) byte_swapped_ll;

     // Convert from UTC to local time.
    if (daylight ) {
      unix_time += altzone;
    } else {
       unix_time += timezone;
    }

    return unix_time;
 }

/*
 *  Convert the OpenVMS epoch into text, selectively punting.
*/
time_t
time_vms_to_unix_asc( unsigned char *vmstime,   int vmstim_len, 
                      char *date_text, int *date_text_len )
 {
    time_t unix_time;

     // On a 64 bit machine the VMS time is a Quadword integer.
    unsigned long long int vms_time_ll = 0;

     if (debugflags & D_TIME) {
        fprintf(stderr, "time_vms_to_unix_asc() size of long long int = %lld\n", (long long) sizeof(vms_time_ll));
        fprintf(stderr, "                       size of char string = %lld\n", (long long) sizeof(vmstime));
     }

     memcpy(&vms_time_ll, vmstime, vmstim_len);
  
     if (debugflags & D_TIME) {
        fprintf(stderr, "                       OpenVMS time in seconds = %lld\n",
               vms_time_ll);
     }

     if ( vmstim_len != 8 ) {
         // VMS time is stored as a 64 bit word or 8 bytes (8 bits = 1 byte).  
        strcpy( date_text, "[OpenVMS date conversion error]");

        if ( date_text_len ) {
           *date_text_len = strlen( date_text );
        }
     }

     if (( vmstim_len == 8 ) && ( vms_time_ll != 0 )) {
        strcpy( date_text, "[no date set]");

        if ( date_text_len ) {
           *date_text_len = strlen( date_text );
        }
     }
 
     unix_time = time_vms_to_unix( vms_time_ll );
     strftime( date_text, 24, "%d-%b-%Y %H:%M:%S", localtime(&unix_time) );

     if (debugflags & D_TIME) {
        fprintf(stderr, "                       OpenVMS date = %s\n", date_text);
     }

     if ( date_text_len ) {
        *date_text_len = strlen( date_text );
     }

     return unix_time;
 }
