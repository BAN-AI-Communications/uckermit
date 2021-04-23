# μCKermit

## NAME

uckermit - minimal kermit file transfer

## SYNOPSIS

**uckermit** \[ option \...\] \[file \...\]

## DESCRIPTION

_μCKermit_ is a file transfer program that allows files to be moved between
machines of many different operating systems and architectures.

Arguments are optional. If _μCKermit_ is executed without arguments, it will
enter command mode. Otherwise, _μCKermit_ will read the arguments off the
command line and interpret them.

The following notation is used in command descriptions:

_fn_

: A UNIX file specification, possibly containing either of the \"wildcard\"
characters \'\~\', \'\*\', or \'?\' (\'\~\' matches a user\'s home directory
name,

_fn1_

: A UNIX file specification which may not contain \'\*\' or \'?\'.

_rfn_

: A remote file specification in the remote system\'s own syntax, which may
denote a single file or a group of files.

_rfn1_

: A remote file specification which should denote only a single file.

_n_

: A decimal number, in most cases between 0 and 94.

_c_

: A decimal number between 0 and 127 representing the value of an ASCII
character.

_cc_

: A decimal number between 0 and 31, or else exactly 127, representing the value
of an ASCII control character.

**\[ \]**

: Any field in square braces is optional.

{_x,y,z_}

: Alternatives are listed in curly braces.

_μCKermit_ command line options may specify either actions or settings. If
_μCKermit_ is invoked with a command line that specifies no actions, then it
will issue a prompt and begin interactive dialog. Action options specify either
protocol transactions or terminal connection.

## COMMAND LINE OPTIONS

**-s\*** fn\*

: Send the specified file or files. If _fn_ contains wildcard (meta) characters,
the UNIX shell expands it into a list. If _fn_ is \'-\' then _μCKermit_ sends
from standard input, which may come from a file:

uckermit -s - \< foo.bar

or a parallel process:

ls -l \| uckermit -s -

> You cannot use this mechanism to send terminal typein. If you want to send a
> file whose name is \"-\" you can precede it with a path name, as in

uckermit -s ./-

**-r**

: Receive a file or files. Wait passively for files to arrive.

**-k**

: Receive (passively) a file or files, sending them to standard output. This
option can be used in several ways:

uckermit -k

> Displays the incoming files on your screen; to be used only in \"local mode\"
> (see below).

uckermit -k \> fn1

> Sends the incoming file or files to the named file, _fn1._ If more than one
> file arrives, all are concatenated together into the single file _fn1._

uckermit -k \| command

> Pipes the incoming data (single or multiple files) to the indicated command,
> as in

uckermit -k \| sort \> sorted.stuff

**-a\*** fn1\*

: If you have specified a file transfer option, you may specify an alternate
name for a single file with the **-a** option. For example,

uckermit -s foo -a bar

> sends the file foo telling the receiver that its name is bar. If more than one
> file arrives or is sent, only the first file is affected by the **-a** option:

uckermit -ra baz

> stores the first incoming file under the name baz.

**-x**

: Begin server operation. May be used in either local or remote mode.

Before proceeding, a few words about remote and local operation are necessary.
_μCKermit_ is \"local\" if it is running on a PC or workstation that you are
using directly, or if it is running on a multiuser system and transferring files
over an external communication line --- not your job\'s controlling terminal or
console. _μCKermit_ is remote if it is running on a multiuser system and
transferring files over its own controlling terminal\'s communication line,
connected to your PC or workstation.

If you are running _μCKermit_ On most systems, _μCKermit_ runs in remote mode by
default, so on a PC or workstation, you will have to put it into local mode. The
following command sets _μCKermit_\'s \"mode\":

**-l\*** dev\*

: Line --- Specify a terminal line to use for file transfer and terminal
connection, as in

uckermit -l /dev/ttyS5

When an external line is being used, you might also need some additional options
for successful communication with the remote system:

**-b\*** n\*

: Baud --- Specify the baud rate for the line given in the **-l** option, as in

uckermit -l /dev/ttyS5 -b 9600

> This option should always be included with the **-l** option, since the speed
> of an external line is not necessarily what you might expect.

**-p\*** x\*

: Parity --- **e**, **o**, **m**, **s**, **n** (even, odd, mark, space, or
none). If parity is other than none, then the 8th-bit prefixing mechanism will
be used for transferring 8-bit binary data, provided the opposite _μCKermit_
agrees. The default parity is none.

**-t**

: Specifies half duplex, line turnaround with XON as the handshake character.

The following commands may be used only with a _μCKermit_ which is local ---
either by default or else because the **-l** option has been specified.

**-g\*** rfn\*

: Actively request a remote server to send the named file or files; _rfn_ is a
file specification in the remote host\'s own syntax. If _fn_ happens to contain
any special shell characters, like \'\*\', these must be quoted, as in

uckermit -g x\\\*.\\?

**-f**

: Send a \'finish\' command to a remote server.

**-c**

: Establish a terminal connection over the specified or default communication
line, before any protocol transaction takes place. Get back to the local system
by typing the escape character (normally Control-Backslash) followed by the
letter \'c\'.

**-n**

: Like **-c,** but after a protocol transaction takes place; **-c** and **-n**
may both be used in the same command. The use of **-n** and **-c** is
illustrated below.

On a timesharing system, the **-l** and **-b** options will also have to be
included with the **-r**, **-k**, or **-s** options if the other _μCKermit_ is
on a remote system.

If _uckermit_ is in local mode, the screen (stdout) is continously updated to
show the progress of the file transer. A dot is printed for every four data
packets, other packets are shown by type (e.g. \'S\' for Send-Init), \'T\' is
printed when there\'s a timeout, and \'%\' for each retransmission. In addition,
you may type (to stdin) certain \"interrupt\" commands during file transfer:

> Control-F: Interrupt the current File, and go on to the next (if any).
>
> Control-B: Interrupt the entire Batch of files, terminate the transaction.
>
> Control-R: Resend the current packet
>
> Control-A: Display a status report for the current transaction.

> These interrupt characters differ from the ones used in other _μCKermit_
> implementations to avoid conflict with UNIX shell interrupt characters. With
> System III and System V implementations of UNIX, interrupt commands must be
> preceeded by the escape character (e.g. control-\\).

Several other command-line options are provided:

**-i**

: Specifies that files should be sent or received exactly \"as is\" with no
conversions. This option is necessary for transmitting binary files. It may also
be used to slightly boost efficiency in UNIX-to-UNIX transfers of text files by
eliminating CRLF/newline conversion.

**-e n**

: Specifies the (extended) receive-packet length, a number between 10 and about
1000 (depending on the system). Lengths of 95 or greater require that the
opposite μCKermit support the long packet protocol extension.

**-w**

: Write-Protect --- Avoid filename collisions for incoming files.

**-q**

: Quiet --- Suppress screen update during file transfer, for instance to allow a
file transfer to proceed in the background.

**-d**

: Debug --- Record debugging information in the file debug.log in the current
directory. Use this option if you believe the program is misbehaving, and show
the resulting log to your local _μCKermit_ maintainer.

**-h**

: Help --- Display a brief synopsis of the command line options.

The command line may contain no more than one protocol action option.

## INTERACTIVE OPERATION

_μCKermit_\'s interactive command prompt is \"μCKermit\>\". In response to this
prompt, you may type any valid command. _μCKermit_ executes the command and then
prompts you for another command. The process continues until you instruct the
program to terminate.

Commands begin with a keyword, normally an English verb, such as \"send\". You
may omit trailing characters from any keyword, so long as you specify sufficient
characters to distinguish it from any other keyword valid in that field. Certain
commonly-used keywords (such as \"send\", \"receive\", \"connect\") have special
non-unique abbreviations (\"s\" for \"send\", \"r\" for \"receive\", \"c\" for
\"connect\").

Certain characters have special functions in interactive commands:

**?**

: Question mark, typed at any point in a command, will produce a message
explaining what is possible or expected at that point. Depending on the context,
the message may be a brief phrase, a menu of keywords, or a list of files.

**ESC**

: (The Escape or Altmode key) --- Request completion of the current keyword or
filename, or insertion of a default value. The result will be a beep if the
requested operation fails.

**DEL**

: (The Delete or Rubout key) --- Delete the previous character from the command.
You may also use BS (Backspace, Control-H) for this function.

**\^W**

: (Control-W) --- Erase the rightmost word from the command line.

**\^U**

: (Control-U) --- Erase the entire command.

**\^R**

: (Control-R) --- Redisplay the current command.

**SP**

: (Space) --- Delimits fields (keywords, filenames, numbers) within a command.
HT (Horizontal Tab) may also be used for this purpose.

**CR**

: (Carriage Return) --- Enters the command for execution. **LF** (Linefeed) or
**FF** (formfeed) may also be used for this purpose.

**\\**

: (Backslash) --- Enter any of the above characters into the command, literally.
To enter a backslash, type two backslashes in a row (\\\\). A single backslash
immediately preceding a carriage return allows you to continue the command on
the next line.

You may type the editing characters (DEL, \^W, etc) repeatedly, to delete all
the way back to the prompt. No action will be performed until the command is
entered by typing carriage return, linefeed, or formfeed. If you make any
mistakes, you will receive an informative error message and a new prompt ---
make liberal use of \'?\' and ESC to feel your way through the commands. One
important command is \"help\" --- you should use it the first time you run
_μCKermit._

Interactive _μCKermit_ accepts commands from files as well as from the keyboard.
Upon startup, _μCKermit_ looks for the file .uckermrc in your home or current
directory (first it looks in the home directory, then in the current one) and
executes any commands it finds there. These commands must be in interactive
format, not UNIX command-line format. A \"take\" command is also provided for
use at any time during an interactive session. Command files may be nested to
any reasonable depth.

Here is a brief list of _μCKermit_ interactive commands:

**! command**

: Execute a UNIX shell command. A space is required after after the !.

**% text**

: A comment. Useful in take-command files.

**bye**

: Terminate and log out a remote _μCKermit_ server.

**close**

: Close a log file.

**connect**

: Establish a terminal connection to a remote system.

**cwd**

: Change Working Directory.

**dial**

: Dial a telephone number.

**directory**

: Display a directory listing.

**echo**

: Display arguments literally. Useful in take-command files.

**exit**

: Exit from the program, closing any open logs.

**finish**

: Instruct a remote _μCKermit_ server to exit, but not log out.

**get**

: Get files from a remote _μCKermit_ server.

**hangup**

: Hang up the phone.

**help**

: Display a help message for a given command.

**log**

: Open a log file --- debugging, packet, session, transaction.

**quit**

: Same as \'exit\'.

**receive**

: Passively wait for files to arrive.

**remote**

: Issue file management commands to a remote _μCKermit_ server.

**script**

: Execute a login script with a remote system.

**send**

: Send files.

**server**

: Begin server operation.

**set**

: Set various parameters.

**show**

: Display values of \'set\' parameters, program version, etc.

**space**

: Display current disk space usage.

**statistics**

: Display statistics about most recent transaction.

**take**

: Execute commands from a file.

**transmit**

: Send a file without error checking.

The \'set\' parameters are:

**attributes**

: Turn attribute packet exchange on or off (default is on).

**block-check**

: Level of packet error detection.

**delay**

: How long to wait before sending first packet.

**duplex**

: Specify which side echoes during \'connect\'.

**escape-character**

: Character to prefix \"escape commands\" during \'connect\'.

**file**

: Set various file parameters.

**flow-control **

: Communication line full-duplex flow control.

**handshake**

: Communication line half-duplex turnaround character.

**line**

: Communication line device name.

**modem-dialer**

: Type of modem-dialer on communication line.

**parity**

: Communication line character parity.

**prompt**

: Change the _μCKermit_ program\'s prompt.

**receive**

: Set various parameters for inbound packets.

**retry**

: Set the packet retransmission limit.

**send**

: Set various parameters for outbound packets.

**server**

: Set server-related parameters (like timeout).

**speed**

: Communication line speed.

The \'remote\' commands are:

**cwd**

: Change remote working directory.

**delete**

: Delete remote files.

**directory**

: Display a listing of remote file names.

**help**

: Request help from a remote server.

**host**

: Issue a command to the remote host in its own command language.

**space**

: Display current disk space usage on remote system.

**type**

: Display a remote file on your screen.

**who**

: Display who\'s logged in, or get information about a user.

## CONTACT

Jeffrey H. Johnson \<trnsz\@pobox.com\>

## HOMEPAGE

\<[https://github.com/johnsonjh/uckermit/](https://github.com/johnsonjh/uckermit/)\>

## ORIGINAL AUTHORS

Frank da Cruz, Columbia University Center for Computing Activities, with
contributions from many others.

## LICENSE

Revised 3-Clause BSD License for Columbia University Kermit Software

:

Copyright (C) 1981-2011,

: Trustees of Columbia University in the City of New York.

All rights reserved.

:

Redistribution and use in source and binary forms, with or without

: modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this

: list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice,

: this list of conditions and the following disclaimer in the documentation and
/ or other materials provided with the distribution.

Neither the name of Columbia University nor the names of its contributors

: may be used to endorse or promote products derived from this software without
specific prior written permission.

**THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS
IS\"**

: AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## FILES

\$HOME/.uckermrc _μCKermit_ initialization commands\
./.uckermrc more _μCKermit_ initialization commands

## SEE ALSO

cu(1C), ecu(1), uucp(1C), kermit(1)

:

Christine Gianone,

: _Kermit User\'s Guide_, Columbia University, 8th Edition

Frank da Cruz,

: _Kermit, A File Transfer Protocol_, Digital Press (1987)

## DIAGNOSTICS

The diagnostics produced by _μCKermit_ itself are intended to be
self-explanatory.

## BUGS

Probably way too many.
