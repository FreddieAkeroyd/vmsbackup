/* This is the version of main() for POSIX.2 (getopt) style arguments.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef   __VMS
#include <descrip.h>
#include <stdarg.h>
#else
#include <getopt.h>
#endif

#include "vmsbackup.h"
#include "vmsbackupver.h"


#ifdef HAVE_GETOPTLONG
#define HELPTEXT(SHORT,LONG,TEXT) "\t -" SHORT "\t --" LONG "\t" TEXT "\n"
#else
#define HELPTEXT(SHORT,NULL,TEXT) "\t -" SHORT "\t" TEXT "\n"
#endif


static char *VersionString = VMSBACKUPVERSION;
void
usage (progname)
char *progname;
{
	fprintf (stderr, "Usage:  %s [OPTION]... [FILE]...\n",
		 progname);
        fprintf (stderr, "  Version %s\n", VersionString );
        fprintf (stderr, "  Reads and optionally unpacks OpenVMS BACKUP savesets\n" );
	// pad the getoptlong text switch lengths to the same for display purposes
	fprintf(stderr, "\nMain operation mode:\n"
            HELPTEXT( "t", "list     ", "List files in saveset" )
            HELPTEXT( "x", "extract  ", "Extract files" ) );
	fprintf(stderr, "\nAvailable options:\n"
            HELPTEXT( "b", "blocksize", "Use specified blocksize" )
            HELPTEXT( "B", "binary   ", "Extract as binary files" )
	    HELPTEXT( "c", "complete ", "Retain complete filename" )
            HELPTEXT( "d", "directory", "Create subdirectories" )
            HELPTEXT( "e", "extensions", "Extract all filename extensions" )
            HELPTEXT( "f", "file     ", "Read from file" )
            HELPTEXT( "F", "full     ", "Full detail in listing" )	    
            HELPTEXT( "o", "output   ", "Extract to file" )
            HELPTEXT( "s", "saveset  ", "Read saveset number" )
            HELPTEXT( "v", "verbose  ", "List files as they are processed" )
	    HELPTEXT( "V", "version  ", "Show program version number" )
            HELPTEXT( "w", "confirm  ", "Confirm files before restoring" )
            HELPTEXT( "?", "help     ", "Display this help message") );
	return;
}

extern int optind;
extern char *optarg;

#ifdef HAVE_GETOPTLONG
static const struct option OptionListLong[] =
{
	{"blocksize", 1, 0, 'b'},
	{"complete", 0, 0, 'c'},
	{"directory", 0, 0, 'd'},
	{"extensions", 0, 0, 'e'},
	{"file", 1, 0, 'f'},
	{"output", 1, 0, 'o'},
	{"saveset", 1, 0, 's'},
	{"list", 0, 0, 't'},
	{"verbose", 0, 0, 'v'},
	{"confirm", 0, 0, 'w'},
	{"extract", 0, 0, 'x'},
	{"full", 0, 0, 'F'},
	{"version", 0, 0, 'V'},
	{"binary", 0, 0, 'B'},
	{"debug", 1, 0, 'D'},
	{"help", 0, 0, '?'},
	{0, 0, 0, 0}
};
#endif

int
main (argc, argv)
int argc;
char *argv[];
{
	char *progname;
	int c;
#ifdef HAVE_GETOPTLONG
	int OptionIndex;
#endif

	progname = argv[0];
	if(argc < 2){
		usage(progname);
		exit(1);
	}

	gargv = argv;
	gargc = argc;

	cflag=dflag=eflag=sflag=tflag=vflag=wflag=xflag=debugflags=0;
	flag_binary = 0;
	flag_full = 0;
	tapefile = NULL;
	outfile = NULL;

#ifdef HAVE_GETOPTLONG
	while((c=getopt_long(argc,argv,"b:cdef:o:s:tvwxFVBD:?",
		OptionListLong, &OptionIndex)) != EOF)
#else
	while((c=getopt(argc,argv,"b:cdef:o:s:tvwxFVBD:?")) != EOF)
#endif
		switch(c){
		case 'b':
			sscanf (optarg, "%d", &blocksize);
			break;
		case 'c':
			cflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 'e':
			eflag++;
			break;
		case 'f':
			tapefile = optarg;
			break;
		case 'o':
			outfile = optarg;
			break;
		case 's':
			sflag++;
			sscanf(optarg,"%d",&selset);
			break;
		case 't':
			tflag++;
			break;
		case 'v':
			vflag++;
			break;
		case 'w':
			wflag++;
			break;
		case 'x':
			xflag++;
			break;
		case 'F':
			/* I'd actually rather have this be --full, but at
			   the moment I don't feel like worrying about
			   infrastructure for parsing long arguments.  I
			   don't like the GNU getopt_long--the interface
			   is noreentrant and generally silly; and it might
			   be nice to have something which synergizes more
			   closely with the VMS options (that one is a bit
			   of a can of worms, perhaps, though) like parseargs
			   or whatever it is called.  */
			flag_full = 1;
			break;
		case 'V':
			printf ("VMSBACKUP version %s\n", VersionString );
			exit (EXIT_FAILURE);
			break;
		case 'B':
			/* This of course should be --binary; see above
			   about long options.  */
			flag_binary = 1;
			break;
		case 'D':
			/* Knob to enable the debugging code; select the bitmask */
			sscanf(optarg,"%x",&debugflags);
			break;
		case '?':
			usage(progname);
			exit(1);
			break;
		};
	goptind = optind;
	if(!tflag && !xflag) {
		usage(progname);
		exit(1);
	}

	/*
	** Try what we came here for.
	** This call never returns.
	*/
	vmsbackup ();
}

