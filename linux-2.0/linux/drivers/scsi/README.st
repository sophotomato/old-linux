This file contains brief information about the SCSI tape driver.
Last modified: Wed May  1 11:51:35 1996 by root@kai.makisara.fi


BASICS

The driver is generic, i.e., it does not contain any code tailored
to any specific tape drive. The tape parameters can be specified with
one of the following three methods:

1. Each user can specify the tape parameters he/she wants to use
directly with ioctls. This is administratively a very simple and
flexible method and applicable to single-user workstations. However,
in a multiuser environment the next user finds the tape parameters in
state the previous user left them.

2. The system manager (root) can define default values for some tape
parameters, like block size and density using the MTSETDRVBUFFER ioctl.
These parameters can be programmed to come into effect either when a
new tape is loaded into the drive or if writing begins at the
beginning of the tape. The second method is applicable if the tape
drive performs auto-detection of the tape format well (like some
QIC-drives). The result is that any tape can be read, writing can be
continued using existing format, and the default format is used if
the tape is rewritten from the beginning (or a new tape is written
for the first time). The first method is applicable if the drive
does not perform auto-detection well enough and there is a single
"sensible" mode for the device. An example is a DAT drive that is
used only in variable block mode (I don't know if this is sensible
or not :-).

The user can override the parameters defined by the system
manager. The changes persist until the defaults again come into
effect.

3. Up to four modes can be defined and selected using the minor number
(bits 5 and 6). Mode 0 corresponds to the defaults discussed
above. Additional modes are dormant until they are defined by the
system manager (root). When specification of a new mode is started,
the configuration of mode 0 is used to provide a starting point for
definition of the new mode.

Using the modes allows the system manager to give the users choices
over some of the buffering parameters not directly accessible to the
users (buffered and asynchronous writes). The modes also allow choices
between formats in multi-tape operations (the explicitly overridden
parameters are reset when a new tape is loaded).

If more than one mode is used, all modes should contain definitions
for the same set of parameters.

Many Unices contain internal tables that associate different modes to
supported devices. The Linux SCSI tape driver does not contain such
tables (and will not do that in future). Instead of that, a utility
program can be made that fetches the inquiry data sent by the device,
scans its database, and sets up the modes using the ioctls. Another
alternative is to make a small script that uses mt to set the defaults
tailored to the system.


The driver supports fixed and variable block size (within buffer
limits). Both the auto-rewind (minor equals device number) and
non-rewind devices (minor is 128 + device number) are implemented.

Support is provided for changing the tape partition and partitioning
of the tape with one or two partitions. By default support for
partitioned tape is disabled for each driver and it can be enabled
with the ioctl MTSETDRVBUFFER.

By default the driver writes one filemark when the device is closed after
writing and the last operation has been a write. Two filemarks can be
optionally written. In both cases end of data is signified by
returning zero bytes for two consecutive reads.

The compile options are defined in the file linux/drivers/scsi/st_options.h.


BUFFERING

The driver uses tape buffers allocated either at system initialization
or at run-time when needed. One buffer is used for each open tape
device. The size of the buffers is selectable at compile and/or boot
time. The buffers are used to store the data being transferred to/from
the SCSI adapter. The following buffering options are selectable at
compile time and/or at run time (via ioctl):

Buffering of data across write calls in fixed block mode (define
ST_BUFFER_WRITES). This should be disabled if reliable detection of
end of medium (EOM) for fixed block mode is desired.

Asynchronous writing. Writing the buffer contents to the tape is
started and the write call returns immediately. The status is checked
at the next tape operation. Should not used if reliable EOM detection
is desired.

Read ahead for fixed block mode (ST_READ_AHEAD). Filling the buffer is
attempted even if the user does not want to get all of the data at
this read command. Should be disabled for those drives that don't like
a filemark to truncate a read request or that don't like backspacing.

The buffer size is defined (in 1024 byte units) by ST_BUFFER_BLOCKS or
at boot time. If this size is not enough, the driver tries to allocate
a large enough temporary buffer that is released when the device is
closed. The maximum buffer size is defined by the kernel memory
allocation (currently 256 kB for Alphas and 128 kB for other
architectures).

Allocation of the buffers is done at run-time when they are
needed. Allocation of the specified number of buffers can be done at
initialization if ST_RUNTIME_BUFFERS is defined non-zero. The
advantage of run-time allocation is that memory is not wasted for
buffers not being used. The disadvantage is that there may not be
memory available at the time when a buffer is needed for the first
time (once a buffer is allocated, it is not released).

The maximum number of buffers allocated at initialization is defined by
ST_MAX_BUFFERS. One buffer is allocated for each drive detected when
the driver is initialized up to the maximum. The minimum number of
allocated buffers is ST_EXTRA_DEVS (in hosts.h). This ensures some
functionality also for the drives found after tape driver
initialization (a SCSI adapter driver is loaded as a module). The
default for ST_EXTRA_DEVS is two. The driver tries to allocate new
buffers at run-time if necessary.

The threshold for triggering asynchronous write in fixed block mode
is defined by ST_WRITE_THRESHOLD. This may be optimized for each
use pattern. The default triggers asynchronous write after three
default sized writes (10 kB) from tar.


BOOT TIME CONFIGURATION

The buffer size, write threshold, and the maximum number of allocated buffers
are configurable at boot time using, e.g., the LILO command line. The option
syntax is the following:

           st=aa[,bb[,cc]]

where
  aa is the buffer size in 1024 byte units
  bb is the write threshold in 1024 byte units
  cc is the maximum number of tape buffers to allocate (the number of
        buffers is bounded also by the number of drives detected)


IOCTLS

The tape is positioned and the drive parameters are set with ioctls
defined in mtio.h The tape control program 'mt' uses these ioctls. Try
to find an mt that supports all of the Linux SCSI tape ioctls and
opens the device for writing if the tape contents will be modified
(look for a package mt-st* from the Linux ftp sites; the GNU mt does
not open for writing for, e.g., erase).

The supported ioctls are:

The following use the structure mtop:

MTFSF   Space forward over count filemarks. Tape positioned after filemark.
MTFSFM  As above but tape positioned before filemark.
MTBSF	Space backward over count filemarks. Tape positioned before
        filemark.
MTBSFM  As above but ape positioned after filemark.
MTFSR   Space forward over count records.
MTBSR   Space backward over count records.
MTFSS   Space forward over count setmarks.
MTBSS   Space backward over count setmarks.
MTWEOF  Write count filemarks.
MTWSM   Write count setmarks.
MTREW   Rewind tape.
MTOFFL  Set device off line (often rewind plus eject).
MTNOP   Do nothing except flush the buffers.
MTRETEN Re-tension tape.
MTEOM   Space to end of recorded data.
MTERASE Erase tape.
MTSEEK	Seek to tape block count. Uses Tandberg-compatible seek (QFA)
        for SCSI-1 drives and SCSI-2 seek for SCSI-2 drives. The file and
	block numbers in the status are not valid after a seek.
MTSETBLK Set the drive block size. Setting to zero sets the drive into
        variable block mode (if applicable).
MTSETDENSITY Sets the drive density code to arg. See drive
        documentation for available codes.
MTLOCK and MTUNLOCK Explicitly lock/unlock the tape drive door.
MTLOAD and MTUNLOAD Explicitly load and unload the tape.
MTCOMPRESSION Sets compressing or uncompressing drive mode using the
	SCSI mode page 15. Note that some drives other methods for
	control of compression. Some drives (like the Exabytes) use
	density codes for compression control. Some drives use another
	mode page but this page has not been implemented in the
	driver.
MTSETPART Moves the tape to the partition given by the argument at the
	next tape operation. The block at which the tape is positioned
	is the block where the tape was previously positioned in the
	new active partition unless the next tape operation is
	MTSEEK. In this case the tape is moved directly to the block
	specified by MTSEEK. MTSETPART is inactive unless
	MT_ST_CAN_PARTITIONS set.
MTMKPART Formats the tape with one partition (argument zero) or two
	partitions (the argument gives in megabytes the size of
	partition 1 that is physically the first partition of the
	tape). The drive has to support partitions with size specified
	by the initiator. Inactive unless MT_ST_CAN_PARTITIONS set.
MTSETDRVBUFFER
	Is used for several purposes. The command is obtained from count
        with mask MT_SET_OPTIONS, the low order bits are used as argument.
	This command is only allowed for the superuser (root). The
	subcommands are:
	0
           The drive buffer option is set to the argument. Zero means
           no buffering.
        MT_ST_BOOLEANS
           Sets the buffering options. The bits are the new states
           (enabled/disabled) the following options (in the
	   parenthesis is specified whether the option is global or
	   can be specified differently for each mode):
	     MT_ST_BUFFER_WRITES write buffering (mode)
	     MT_ST_ASYNC_WRITES asynchronous writes (mode)
             MT_ST_READ_AHEAD  read ahead (mode)
             MT_ST_TWO_FM writing of two filemarks (global)
	     MT_ST_FAST_EOM using the SCSI spacing to EOD (global)
	     MT_ST_AUTO_LOCK automatic locking of the drive door (global)
             MT_ST_DEF_WRITES the defaults are meant only for writes (mode)
	     MT_ST_CAN_BSR backspacing over more than one records can
		be used for repositioning the tape (global)
	     MT_ST_NO_BLKLIMS the driver does not ask the block limits
		from the drive (block size can be changed only to
		variable) (global)
	     MT_ST_CAN_PARTITIONS enables support for partitioned
		tapes (global)
	     MT_ST_SCSI2LOGICAL the logical block number is used in
		the MTSEEK and MTIOCPOS for SCSI-2 drives instead of
		the device dependent address. It is recommended to set
		this flag unless there are tapes using the device
		dependent (from the old times) (global)
	     MT_ST_DEBUGGING debugging (global; debugging must be
		compiled into the driver)
	MT_ST_SETBOOLEANS
	MT_ST_CLEARBOOLEANS
	   Sets or clears the option bits.
        MT_ST_WRITE_THRESHOLD
           Sets the write threshold for this device to kilobytes
           specified by the lowest bits.
	MT_ST_DEF_BLKSIZE
	   Defines the default block size set automatically. Value
	   0xffffff means that the default is not used any more.
	MT_ST_DEF_DENSITY
	MT_ST_DEF_DRVBUFFER
	MT_ST_DEF_COMPRESSION
	   Used to set or clear the density (8 bits), drive buffer
	   state (3 bits), and compression (single bit). If the value is
	   MT_ST_CLEAR_DEFAULT (0xfffff), the default will not be used
	   any more. Otherwise the lower-most bits of the value contain
	   the new value of the parameter.

The following ioctl uses the structure mtpos:
MTIOCPOS Reads the current position from the drive. Uses
        Tandberg-compatible QFA for SCSI-1 drives and the SCSI-2
        command for the SCSI-2 drives.

The following ioctl uses the structure mtget to return the status:
MTIOCGET Returns some status information.
        The file number and block number within file are returned. The
        block is -1 when it can't be determined (e.g., after MTBSF).
        The drive type is either MTISSCSI1 or MTISSCSI2.
        The number of recovered errors since the previous status call
        is stored in the lower word of the field mt_erreg.
        The current block size and the density code are stored in the field
        mt_dsreg (shifts for the subfields are MT_ST_BLKSIZE_SHIFT and
        MT_ST_DENSITY_SHIFT).
	The GMT_xxx status bits reflect the drive status. GMT_DR_OPEN
	is set if there is no tape in the drive. GMT_EOD means either
	end of recorded data or end of tape. GMT_EOT means end of tape.

The following ioctls use the structure mtlocation that contains both
the block number and the partition number. These ioctls are available
only for SCSI-2 tape drives and the block number is the
device-independent logical block number defined by the standard.

MTGETLOC Returns the current block and partition number.
MTSETLOC Sets the tape to the block and partition specified by the
	arguments.


MISCELLANEOUS COMPILE OPTIONS

The recovered write errors are considered fatal if ST_RECOVERED_WRITE_FATAL
is defined.

The maximum number of tape devices is determined by the define
ST_MAX_TAPES. If more tapes are detected at driver initialization, the
maximum is adjusted accordingly.

Immediate return from tape positioning SCSI commands can be enabled by
defining ST_NOWAIT. If this is defined, the user should take care that
the next tape operation is not started before the previous one has
finished. The drives and SCSI adapters should handle this condition
gracefully, but some drive/adapter combinations are known to hang the
SCSI bus in this case.

The MTEOM command is by default implemented as spacing over 32767
filemarks. With this method the file number in the status is
correct. The user can request using direct spacing to EOD by setting
ST_FAST_EOM 1 (or using the MT_ST_OPTIONS ioctl). In this case the file
number will be invalid.

When using read ahead or buffered writes the position within the file
may not be correct after the file is closed (correct position may
require backspacing over more than one record). The correct position
within file can be obtained if ST_IN_FILE_POS is defined at compile
time or the MT_ST_CAN_BSR bit is set for the drive with an ioctl.
(The driver always backs over a filemark crossed by read ahead if the
user does not request data that far.)

Kai M{kisara
