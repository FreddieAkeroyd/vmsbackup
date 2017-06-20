/*
** vmsconstants.h
**
** The following is intended to parallel OpenVMS definitions
** found in FABDEF and other structures, but to avoid the
** inclusion of any of the OpenVMS definition files here.
*/

#define VMS_BLOCK       512

/* the file organization codes */

#define FAB_C_SEQ        0      /* sequential-format file */
#define FAB_C_REL	16	/* relative-format file */
#define FAB_C_IDX	32	/* indexed-format file (not portable!) */
#define FAB_C_HSH	48	/* hashed-format file (not used?) */

#define FAT_C_SEQUENTIAL 0      /* sequential-format file */
#define FAT_C_RELATIVE   1      /* relative-format file */
#define FAT_C_INDEXED	 2	/* indexed-format file (not portable!) */
#define FAB_C_DIRECT     3	/* direct-format file (not implemented) */

/* FAB$B_RAT - Record attributes. */


#define FAB_V_FTN	0	/* FAB$V_FTN - Fortran carriage control */
/*
** Fortran carriage control byte definition follows:

** 0x00: null             carriage control
** 0x20: single-space     carriage control (LF,buffer,CR)
** 0x24: prompting-format carriage control (LF,buffer)
** 0x2B: over-printing    carriage control (buffer,CR)
** 0x30: double-space     carriage control (LF,LF,buffer,CR)
** 0x31: page-eject       carriage control (FF,buffer,CR)
*/ 

#define FAB_V_CR	(1)	/* FAB$V_CR - Carriage return carriage 
                                              control */

#define FAB_V_PRN	(2)	/* FAB$V_PRN - Print carriage control; 
/* Two contro bytes follow.
**
** the following are the definition of each of the two bytes are the prefix 
** code and the suffix code.  With PRN, the prefix byte is processed, then 
** the data written, then the suffix byte is processed.
**
**   7 6 5 4 3 2 1 0: the bits of each byte of the PRN code 
**   0 0 0 0 0 0 0 0: no carriage control
**   0 ? ? ? ? ? ? ?: bits 0:6 are newline count
**   1 0 0 ? ? ? ? ?: bits 0:4 are the ASCII C0 control character
**   1 1 0 0 ? ? ? ?: bits 0:3 are the printer's vertical format unit 
**                             (VFU) code, or (if no VFU print hardware 
**                             is present today) comprise the count of
**                             blank lines to advance.
*/ 

#define FAB_V_BLK	(3)	/* FAB$V_BLK - bit set when records cannot be 
                                               split across block boundaries */
#define FAB_V_MSB	(4)     /* FAB$B_MSB - the PRN code bytes are normally 
                                               LSB, set this bit to use MSB 
                                               format. */

#define FAB_M_FTN	(1<<FAB_V_FTN)
#define FAB_M_CR	(1<<FAB_V_CR)
#define FAB_M_PRN	(1<<FAB_V_PRN)
#define FAB_M_BLK	(1<<FAB_V_BLK)
#define FAB_M_MSB	(1<<FAB_V_MSB)

/* the record structure codes */

#define FAB_C_UDF	0	/* undefined, or stream-format binary */
#define FAB_C_FIX	1	/* fixed-length */
#define FAB_C_VAR	2	/* variable-length */
#define FAB_C_VFC	3	/* the infamous variable-length with fixed control */
#define FAB_C_STM	4	/* RMS stream sequential, also used by some Windows and Unix files */
#define FAB_C_STMLF	5	/* Stream sequential with linefeed; used by C and Unix */
#define FAB_C_STMCR	6	/* Stream sequential with carriage control */

#define FAT_C_UDEFINED	0	/* undefined, or stream-format binary */
#define FAT_C_FIXED	1	/* fixed-length */
#define FAT_C_VARIABLE	2	/* variable-length */
#define FAT_C_VFC	3	/* the infamous variable-length with fixed control */
#define FAT_C_STREAM	4	/* RMS stream sequential, also used by some Windows and Unix files */
#define FAT_C_STREAMLF	5	/* Stream sequential with linefeed; used by C and Unix */
#define FAT_C_STREAMCR	6	/* Stream sequential with carriage control */

/* BACKUP data structure constants */

#define BACKUP_K_OPSYS_VAX 	0x0400       	/* OpenVMS VAX */
#define BACKUP_K_OPSYS_ALPHA 	0x0800		/* OpenVMS Alpha */
#define BACKUP_K_OPSYS_IA64	0x01000		/* OpenVMS I64 */
#define BACKUP$K_BACKUP 1		/* Subsystem ID */
#define BACKUP$K_DATABLOCK 1	/* Application ID normal blocks */
#define BACKUP$K_XORBLOCK 2		/* Application ID XOR blocks */

/* BACKUP summary record attribute codes.  */

#define BSA_K_SSNAME	1	/* size and value of saveset name */
#define BSA_K_COMMAND	2       /* the BACKUP command line */
#define BSA_K_COMMENT	3       /* whatever the user included as a comment */
#define BSA_K_USERNAME	4       /* saveset creator username */
#define BSA_K_USERUIC	5       /* saveset creator UIC */
#define BSA_K_DATE	6       /* saveset creation date (VMS quadword) */
#define BSA_K_OPSYS	7       /* the OPSYS code (above) */
#define BSA_K_SYSVER	8       /* the operating system version */
#define BSA_K_NODENAME	9       /* the node name of the system that wrote the saveset */
#define BSA_K_SIR	10      /* System ID Register */
#define BSA_K_DRIVEID 	11      /* Identification of associated device */
#define BSA_K_BACKVER 	12      /* The BACKUP version */
#define BSA_K_BLOCKSIZE 13      /* saveset blocksize */
#define BSA_K_XORSIZE 	14      /* XOR group size */
#define BSA_K_BUFFERS 	15      /* the number of buffers used */
#define BSA_K_VOLSETNAM 16      /* the name of the volumeset */
#define BSA_K_NVOLS 	17      /* the number of volumes in the volumeset */
#define BSA_K_BACKSIZE 	18      /* the total storage used within the saveset */
#define BSA_K_BACKFILES 19      /* the total number of files within the saveset  */

/* BACKUP volume record attribute codes.  */

#define BSA_K_VOLSTRUCT 20      /* volume structure level */
#define BSA_K_VOLNAME   21      /* volume label */
#define BSA_K_OWNERNAME 22      /* volume owner name */
#define BSA_K_FORMAT    23      /* volume file format name */
#define BSA_K_RVN       24      /* relative volume number */
#define BSA_K_VOLOWNER  25      /* volume owner UIC */
#define BSA_K_PROTECT   26      /* volume protection bitmask */
#define BSA_K_FILEPROT  27      /* volume file protection (default) */
#define BSA_K_RECPROT   28      /* volume record protection (default) */
#define BSA_K_VOLCHAR   29      /* volume characteristics bitmask */
#define BSA_K_VOLDATE   30      /* volume creation date */
#define BSA_K_WINDOW    31      /* file window size (default) */
#define BSA_K_LRU_LIM   32      /* directory LRU limit (default)*/
#define BSA_K_EXTEND    33      /* file extend size (default) */
#define BSA_K_CLUSTER   34      /* storage map cluster factor */
#define BSA_K_RESFILES  35      /* number of reserved files */
#define BSA_K_VOLSIZE   36      /* orig. volume size in blocks */
#define BSA_K_TOTSIZE   37      /* total file space in volume set */
#define BSA_K_TOTFILES  38      /* total files in volume set */
#define BSA_K_MAXFILES  39      /* maximum files allowed  */
#define BSA_K_MAXFILNUM 40      /* highest file number */
#define BSA_K_SERIALNUM 41      /* volume pack serial number */

/* BACKUP file record attribute codes.  */

#define BSA_K_FILENAME  42      /* The filename */
#define BSA_K_STRUCLEV  43      /* Volume file structure level */
#define BSA_K_FID       44      /* The File Id */
#define BSA_K_BACKLINK  45      /* The Directory Id; the Backlink  */
#define BSA_K_FILESIZE  46      /* The file size (blocks) */
#define BSA_K_UIC       47      /* The file owner UIC */
#define BSA_K_FPRO      48      /* Protection Bitmask */
#define BSA_K_RPRO      49      /* Record Protection Bitmask */
#define BSA_K_ACLEVEL   50      /* File Access Level */
#define BSA_K_UCHAR     51      /* File characteristics */
#define BSA_K_RECATTR   52      /* File record attributes */
#define BSA_K_REVISION  53      /* Revision number */
#define BSA_K_CREDATE   54      /* The creation date (VMS native) */
#define BSA_K_REVDATE   55      /* The revision date (VMS native) */
#define BSA_K_EXPDATE   56      /* The expiration date (VMS native) */
#define BSA_K_BAKDATE   57      /* The backup date (VMS native) */

/* BACKUP physical volume record attribute codes.  */

#define BSA_K_SECTORS   58      /* Geometry: sectors */
#define BSA_K_TRACKS    59      /* Geometry: tracks */
#define BSA_K_CYLINDERS 60      /* Geometry: cylinders */
#define BSA_K_MAXBLOCK  61      /* Volume Max Block Count */
#define BSA_K_DEVTYP    62      /* The device type code */
#define BSA_K_SERIAL    63      /* Device serial number */
#define BSA_K_DEVNAM    64      /* The device name */
#define BSA_K_LABEL     65      /* The volume label */
#define BSA_K_BADBLOCK  66      /* Bad block descriptor */

#define BSA_K_INDEXLBN   67     /* Index file bitmap starting LBN */
#define BSA_K_BOOTBLOCK  68     /* The Boot block image name */
#define BSA_K_BOOTVBN    69     /* file VBN for first boot image */
#define BSA_K_PLACEMENT  70     /* File Placement data */
#define BSA_K_DIR_UIC    71     /* Directory UIC */
#define BSA_K_DIR_FPRO   72     /* Directory Protection */
#define BSA_K_DIR_STATUS 73     /* Directory Status */
#define BSA_K_DIR_VERLIM 74     /* DIrectory Version Limit */
#define BSA_K_VERLIMIT   75     /* The file version limit */
#define BSA_K_RETAINMIN  76     /* file retention: minimum  */
#define BSA_K_RETAINMAX  77     /* file retention: maximum  */
#define BSA_K_ACLSEGMENT 78     /* ACL segment */
#define BSA_K_HIGHWATER  79     /* File Highwater mark */
#define BSA_K_JNL_FLAGS  80     /* Journaling control flags */
#define BSA_K_CRYPDATKEY 81     /* File encryption attributes */
#define BSA_K_LBNSIZE    82     /* Disk block size in bytes */
#define BSA_K_RU_ACTIVE  87     /* Active recovery units recoverable facility id (if !0) */
#define BSA_K_BACKREV    88     /* Backup revision count */
#define BSA_K_MIN_CLASS  89     /* SEVMS volume security class: minimum  */
#define BSA_K_MAX_CLASS  90     /* SEVMS security class: maximum   */
#define BSA_K_CLASS_PROT 91     /* SEVMS File security class */
#define BSA_K_BOOTVBN2   92     /* file VBN for second boot image */
#define BSA_K_ACCDATE    93     /* ODS-5 file access date */
#define BSA_K_ATTDATE    94     /* ODS-5 file attribute revision date */
#define BSA_K_EX_RECATTR 95     /* ODS-5 Extended RMS record attributes */
#define BSA_K_LENGTH_HINT 96    /* ODS-5 File length hint */
#define BSA_K_LINKCOUNT  97     /* ODS-5 File (hard) link count */
#define BSA_K_GPT_VBN    98     /* VBN within file for EFI/GPT partition image */
#define BSA_K_GPT_TYPE   99     /* Type of EFI/GPT partition image. */
#define BSA_K_CRYPAESDATKEY 100 /* File encryption attribute for AES */
#define BSA_K_EXPSIZE   101     /* Volume Expansion Size       */
#define BSA_K_NUM_ATRS  102     /* Number of attribute codes        */

