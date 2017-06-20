$ write sys$output "Build procedure intended for OpenVMS testing only."
$ write sys$output "On OpenVMS, use the (real) BACKUP and not this tool."
$
$
$ VBVER = "4.4.3hb"
$
$ if f$locate(",D,",",''p1',") .ne. f$length(",''p1',")
$ then
$   ccopt = "/DEBUG/NOOPT"
$   linkopt = "/DEBUG"
$ else
$   ccopt = "/NODEBUG"
$   linkopt = "/NODEBUG"
$ endif
$
$ close/nolog opt
$ open/write opt sys$scratch:vmsbackupbld.opt
$ write opt "IDENTIFICATION=""VMSBACK''VBVER'"""
$ close opt
$ close/nolog ver
$ open/write ver vmsbackupver.h
$ write ver "// scratch file; can be deleted"
$ write ver "#define VMSBACKUPVERSION ""''VBVER'"" "
$ close ver
$
$
$ CC GETOPTMAIN.C 'CCOPT'  /DEFINE=(HAVE_MT_IOCTLS=0,HAVE_UNIXIO_H=1)
$ CC VMSBACKUP.C 'CCOPT' /DEFINE=(HAVE_MT_IOCTLS=0,HAVE_UNIXIO_H=1)
$ CC match 'CCOPT'
$
$ LINK/execut=VMSBACKUP.EXE 'LINKOPT' -
  getoptmain.obj, vmsbackup.obj, match.obj, -
  sys$scratch:vmsbackupbld.opt/opt
$
$ if f$search("sys$scratch:vmsbackupbld.opt") .nes. "" then delete sys$scratch:vmsbackupbld.opt;*
$ exit
$
$
$! The following was the old CLI-based build, and it's been disabled.
$
$ CC VMSBACKUP.C 'CCOPT' /DEFINE=(HAVE_MT_IOCTLS=0,HAVE_UNIXIO_H=1,VBVER="''VBVER'")
$ CC DCLMAIN.C 'CCOPT' /DEFINE=(VBVER="''VBVER'")
$! Probably we don't want match as it probably doesn't implement VMS-style
$! matching, but I haven't looking into the issues yet.
$ CC match 'CCOPT' /DEFINE=(VBVER="''VBVER'")
$
$ LINK/execut=VMSBACKUP.EXE 'LINKOPT' -
  vmsbackup.obj,dclmain.obj,match.obj, -
  sys$scratch:vmsbackupbld.opt/opt
$
$ if f$search("sys$scratch:vmsbackupbld.opt") .nes. "" then delete sys$scratch:vmsbackupbld.opt;*
$ if f$search("vmsbackupver.h") .nes. "" then delete vmsbackupver.h;*
$ exit
