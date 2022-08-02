.\" SPDX-License-Identifier: BSD-3-Clause
.\" Copyright (c) 2021, 2022, Jeffrey H. Johnson <trnsz@pobox.com>
.\" Copyright (c) 1984, Jeff Damens, Columbia University Center for Computing Activites
.\" Copyright (c) 1985, Herman Fischer, Encino CA
.\" Copyright (c) 1981-2011, Trustees of Columbia University in the City of New York.
.TH UCKERMIT 1C LOCAL
.SH NAME
uckermit \- minimalistic kermit file transfer
.SH SYNOPSIS
.B uckermit
[ option ...] [file ...]
.SH DESCRIPTION
.I uCKermit
is a file transfer program that allows files to be moved
between machines of many different operating systems and architectures.
.PP
Arguments are optional. If
.I uCKermit
is executed without arguments, it will enter command mode. Otherwise,
.I uCKermit
will read the arguments off the command line and interpret them.
.RE 1
.PP
The following notation is used in command descriptions:
.RE 1
.TP 8
.I fn
A UNIX file specification, possibly containing the "wildcard"
characters '~', '*', or '?' ('~' matches a user's home directory name,
'*' matches all character strings, '?' matches any single character).
.TP
.I fn1
A UNIX file specification which may not contain '*' or '?'.
.TP
.I rfn
A remote file specification in the remote system's own syntax, which may
denote a single file or a group of files.
.TP
.I rfn1
A remote file specification which should denote only a single file.
.TP
.I n
A decimal number, in most cases between 0 and 94.
.TP
.I c
A decimal number between 0 and 127 representing the value of an ASCII
character.
.TP
.I cc
A decimal number between 0 and 31, or else exactly 127, representing the
value of an ASCII control character.
.TP
.B [ ]
Any field in square braces is optional.
.TP
{\fIx,y,z\fR}
Alternatives are listed in curly braces.
.RE 1
.PP
.I uCKermit
command line options may specify either actions or settings.  If
.I uCKermit
is invoked with a command line that specifies no actions, then it will
issue a prompt and begin interactive dialog.  Action options specify
either protocol transactions or terminal connection.
.RE
.SS COMMAND LINE OPTIONS
.TP 8
.BI \-s " fn"
Send the specified file or files. If
.I fn
contains wildcard (shell meta) characters, the UNIX shell expands it into a
list.  If
.I fn
is '-' then
.I uCKermit
sends from standard input, which may come from a file:
.sp 0.6v
            uckermit -s - < foo.bar
.sp 0.6v
or a parallel process:
.sp 0.6v
            ls -l | uckermit -s -
.IP
You cannot use this mechanism to send terminal typein.  If you want to
send a file whose name is "-" you can precede it with a path name, as in
.sp 0.6v
            uckermit -s ./-
.TP
.B \-r
Receive a file or files.  Wait passively for files to arrive.
.TP
.B \-k
Receive (passively) a file or files, sending them to standard output.
This option can be used in several ways:
.sp 0.6v
        uckermit -k
.IP
Displays the incoming files on your screen; to be used only in "local
mode" (see below).
.sp 0.6v
        uckermit -k > fn1
.IP
Sends the incoming file or files to the named file,
.I fn1.
If more than
one file arrives, all are concatenated together into the single file
.I fn1.
.sp 0.6v
        uckermit -k | command
.IP
Pipes the incoming data (single or multiple files) to the indicated
command, as in
.sp 0.6v
        uckermit -k | sort > sorted.stuff
.TP
.BI \-a " fn1"
If you have specified a file transfer option, you may specify an
alternate name for a single file with the
.B -a
option.  For example,
.sp 0.6v
        uckermit -s foo -a bar
.IP
sends the file foo telling the receiver that its name is bar.  If more
than one file arrives or is sent, only the first file is affected by the
.B \-a
option:
.sp 0.6v
        uckermit -ra baz
.IP
stores the first incoming file under the name baz.
.TP
.B \-x
Begin server operation.  May be used in either local or remote mode.
.RE 1
.PP
Before proceeding, a few words about remote and local operation are  necessary.
.I uCKermit
is "local" if it is running on a PC or workstation that you are using
directly, or if it is running on a multiuser system and transferring
files over an external communication line \(em not your job's controlling
terminal or console.
.I uCKermit
is remote if it is running on a multiuser system and transferring files
over its own controlling terminal's communication line, connected to
your PC or workstation.
.PP
On most systems,
.I uCKermit
runs in remote mode by default, so on a PC or workstation, you will have
to put it into local mode.  The following command sets
.IR uCKermit 's
"mode":
.RE 1
.TP 8
.BI \-l " dev"
Line \(em Specify a terminal line to use for file transfer and terminal
connection, as in
.sp 0.6v
            uckermit -l /dev/ttyS5
.RE 1
.PP
When an external line is being used, you might also need some additional
options for successful communication with the remote system:
.TP
.BI \-b " n"
Baud \(em Specify the baud rate for the line given in the
.B \-l
option, as in
.sp 0.6v
            uckermit -l /dev/ttyS5 -b 9600
.IP
This option should always be included with the
.B  \-l
option, since the speed of an external line is not necessarily what you
might expect.
.TP
.BI \-p " x"
Parity \(em
.BR e ,
.BR o ,
.BR m ,
.BR s ,
.B  n
(even, odd, mark, space, or none).  If parity is other than none, then
the 8th-bit prefixing mechanism will be used for transferring 8-bit
binary data, provided the opposite
.I Kermit
agrees. The default parity is none.
.TP
.B \-t
Specifies half duplex, line turnaround with XON as the handshake
character.
.RE 1
.PP
The following commands may be used only with a
.I uCKermit
which is local \(em either by default or else because the
.B  \-l
option has been specified.
.TP 8
.BI \-g " rfn"
Actively request a remote server to send the named file or files;
.I rfn
is a file specification in the remote host's own syntax.  If
.I fn
happens to contain any special shell characters, like '*', these must be
quoted, as in
.sp 0.6v
            uckermit -g x\\*.\\?
.TP
.B \-f
Send a 'finish' command to a remote server.
.TP
.B \-c
Establish a terminal connection over the specified or default
communication line, before any protocol transaction takes place.  Get
back to the local system by typing the escape character (normally
Control-Backslash) followed by the letter 'c'.
.TP
.B \-n
Like
.B \-c,
but after a protocol transaction takes place;
.B \-c
and
.B \-n
may both be used in the same command.  The use of
.B \-n
and
.B \-c
is illustrated below.
.RE 1
.PP
On a timesharing system, the
.B \-l
and
.B \-b
options will also have to be included with the
.BR \-r ,
.BR \-k ,
or
.B \-s
options if the other
.I uCKermit
is on a remote system.
.PP
If
.I uckermit
is in local mode, the screen (stdout) is continously updated to show the
progress of the file transer.  A dot is printed for every four data
packets, other packets are shown by type (e.g. 'S' for Send-Init), 'T'
is printed when there's a timeout, and '%' for each retransmission.  In
addition, you may type (to stdin) certain "interrupt" commands during
file transfer:
.IP
Control-F:  Interrupt the current File, and go on to the next (if any).
.IP
Control-B:  Interrupt the entire Batch of files, terminate the transaction.
.IP
Control-R:  Resend the current packet
.IP
Control-A:  Display a status report for the current transaction.
.RE
.IP
These interrupt characters differ from the ones used in other
.I Kermit
implementations to avoid conflict with UNIX shell interrupt characters.
With System III and System V implementations of UNIX, interrupt commands
must be preceeded by the escape character (e.g. Control-\\).
.RE
.PP
Several other command-line options are provided:
.TP 7
.B \-i
Specifies that files should be sent or received using text conversion.
.TP
.B \-e " n"
Specifies the (extended) receive-packet length, a number between 10
and about 1200 (depending on the system).  Lengths of 95 or greater require
that the opposite Kermit program support the long packet protocol extension.
.TP
.B \-w
Overwrite \(em Overwrite existing files with incoming files.
.TP
.B \-q
Quiet \(em Suppress screen update during file transfer, for instance to
allow a file transfer to proceed in the background.
.TP
.B \-d
Debug \(em Record debugging information in the file debug.log in the
current directory.  Use this option if you believe the program is
misbehaving, and show the resulting log to your local
.I uCKermit
maintainer.
.TP
.B \-h
Help \(em Display a brief synopsis of the command line options.
.RE 1
.PP
The command line may contain no more than one protocol action option.
.SS INTERACTIVE OPERATION
.RE 1
.PP
.IR uCKermit 's
interactive command prompt is "uCKermit>". In response to this prompt,
you may type any valid command.
.I uCKermit
executes the command and then prompts you for another command.  The
process continues until you instruct the program to terminate.
.PP
Commands begin with a keyword, normally an English verb, such as "send".
You may omit trailing characters from any keyword, so long as you
specify sufficient characters to distinguish it from any other keyword
valid in that field.
.PP
Certain characters have special functions in interactive commands:
.PP
.TP 8
.B ?
Question mark, typed at any point in a command, will produce a message
explaining what is possible or expected at that point.  Depending on the
context, the message may be a brief phrase, a menu of keywords, or a
list of files.
.TP 8
.B ESC
(The Escape, Meta, or Altmode key) \(em Request completion of the current
keyword or filename, or insertion of a default value.  The result will be
a beep if the requested operation fails.
.TP 8
.B DEL
(The Delete or Rubout key) \(em Delete the previous character from the
command.  You may also use BS (Backspace, Control-H) for this function.
.TP
.B ^W
(Control-W) \(em Erase the rightmost word from the command line.
.TP
.B ^U
(Control-U) \(em Erase the entire command.
.TP
.B ^R
(Control-R) \(em Redisplay the current command.
.TP
.B SP
(Space) \(em Delimits fields (keywords, filenames, numbers) within a
command.  HT (Horizontal Tab) may also be used for this purpose.
.TP
.B CR
(Carriage Return) \(em Enters the command for execution.
.B LF
(Linefeed) or
.B FF
(formfeed) may also be used for this purpose.
.TP
.B \e
(Backslash) \(em Enter any of the above characters into the command,
literally.  To enter a backslash, type two backslashes in a row (\\\\).
A single backslash immediately preceding a carriage return allows you
to continue the command on the next line.

.RE 1
.PP
You may type the editing characters (DEL, ^W, etc) repeatedly, to delete
all the way back to the prompt.  No action will be performed until the
command is entered by typing carriage return, linefeed, or formfeed.  If
you make any mistakes, you will receive an informative error message and
a new prompt \(em make liberal use of '?' and ESC to feel your way through
the commands.  One important command is "help" \(em you should use it the
first time you run
.I uCKermit.
.PP
Interactive
.I uCKermit
accepts commands from files as well as from the keyboard.  Upon
startup,
.I uCKermit
looks for the file .uckermrc in your home or current directory (first it
looks in the home directory, then in the current one) and executes any
commands it finds there.  These commands must be in interactive format,
not UNIX command-line format.  A "take" command is also provided for use
at any time during an interactive session.  Command files may be nested
to any reasonable depth.
.PP
Here is a brief list of
.I uCKermit
interactive commands:
.PD 0.6v
.TP 12
.B ! command
Execute a UNIX shell command.  A space is required after after the !.
.TP
.B % text
A comment.  Useful in take-command files.
.TP
.B bye
Terminate and log out a remote
.I uCKermit
server.
.TP
.B close
Close a log file.
.TP
.B connect
Establish a terminal connection to a remote system.
.TP
.B  cwd
Change Working Directory.
.TP
.B dial
Dial a telephone number.
.TP
.B directory
Display a directory listing.
.TP
.B echo
Display arguments literally.  Useful in take-command files.
.TP
.B exit
Exit from the program, closing any open logs.
.TP
.B finish
Instruct a remote
.I uCKermit
server to exit, but not log out.
.TP
.B get
Get files from a remote
.I uCKermit
server.
.TP
.B hangup
Hang up the phone.
.TP
.B help
Display a help message for a given command.
.TP
.B log
Open a log file \(em debugging, packet, session, transaction.
.TP
.B quit
Same as 'exit'.
.TP
.B receive
Passively wait for files to arrive.
.TP
.B remote
Issue file management commands to a remote
.I uCKermit
server.
.TP
.B script
Execute a login script with a remote system.
.TP
.B send
Send files.
.TP
.B server
Begin server operation.
.TP
.B set
Set various parameters.
.TP
.B show
Display values of 'set' parameters, program version, etc.
.TP
.B space
Display current disk space usage.
.TP
.B statistics
Display statistics about most recent transaction.
.TP
.B take
Execute commands from a file.
.TP
.B transmit
Send a file without error checking.
.sp 1
.PP
The 'set' parameters are:
.TP 21
.B attributes
Turn attribute packet exchange on or off (default is on).
.TP
.B block-check
Level of packet error detection.
.TP
.B delay
How long to wait before sending first packet.
.TP
.B duplex
Specify which side echoes during 'connect'.
.TP
.B escape-character
Character to prefix "escape commands" during 'connect'.
.TP
.B file
Set various file parameters.
.TP
.B
flow-control
Communication line full-duplex flow control.
.TP
.B handshake
Communication line half-duplex turnaround character.
.TP
.B line
Communication line device name.
.TP
.B  modem-dialer
Type of modem-dialer on communication line.
.TP
.B parity
Communication line character parity.
.TP
.B prompt
Change the
.I uCKermit
program's prompt.
.TP
.B receive
Set various parameters for inbound packets.
.TP
.B retry
Set the packet retransmission limit.
.TP
.B send
Set various parameters for outbound packets.
.TP
.B server
Set server-related parameters (like timeout).
.TP
.B speed
Communication line speed.
.sp 1
.PP
The 'remote' commands are:
.TP 12
.B cwd
Change remote working directory.
.TP
.B delete
Delete remote files.
.TP
.B directory
Display a listing of remote file names.
.TP
.B help
Request help from a remote server.
.TP
.B host
Issue a command to the remote host in its own command language.
.TP
.B  space
Display current disk space usage on remote system.
.TP
.B type
Display a remote file on your screen.
.TP
.B who
Display who's logged in, or get information about a user.
.SH CONTACT
Jeffrey H. Johnson
<trnsz@pobox.com>
.SH HOMEPAGE
https://github.com/BAN-AI-Communications/uckermit
.SH ORIGINAL AUTHORS
Frank da Cruz, Columbia University Center for Computing Activities,
with contributions from many others.
.SH LICENSE
SPDX-License-Identifier: BSD-3-Clause
.PP
Revised 3-Clause BSD License for Columbia University Kermit Software
.PP
Copyright (c) 2021, 2022, Jeffrey H. Johnson <trnsz@pobox.com>
.br
Copyright (c) 1984, Jeff Damens, Columbia University Center for Computing Activites
.br
Copyright (c) 1985, Herman Fischer, Encino CA
.br
Copyright (c) 1981-2011, Trustees of Columbia University in the City of New York.
.PP
All Rights Reserved.
.PP
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
.PP
.br
.IP 1.
Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
.PP
.IP 2.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and / or other materials provided with the distribution.
.PP
.IP 3.
Neither the name of Columbia University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
.PP
.B
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
.SH FILES
.ta \w"$HOME/.uckermrc  "u
$HOME/.uckermrc    \fIuCKermit\fR initialization commands
.br
\&./.uckermrc    more \fIuCKermit\fR initialization commands
.PD
.SH SEE ALSO
.TP
cu(1C), ecu(1), uucp(1C), kermit(1)
.TP
Christine Gianone,
.IR "Kermit User's Guide" ,
Columbia University, 8th Edition
.TP
Frank da Cruz,
.IR "Kermit, A File Transfer Protocol" ,
Digital Press (1986)
.SH DIAGNOSTICS
The diagnostics produced by
.I uCKermit
itself are intended to be self-explanatory.
.SH BUGS
Probably way too many.

