# Blowfish Cipher Tool
Author: Cody Moore

This program allows the user to either encrypt and decrypt any given file or string using the blowfish encryption algorithm.  A password must be provided for either use that is used for the key.

## Description

The Blowfish Cipher Tool was made for the purpose of either encrypting or decrypting a file or string.  It currently uses the blowfish encryption algorithm that conforms to Netscapes SSL, developed by Eric Young (C) 1995-1997. The blowfish encryption algorithm was first designed by Bruce Schneier in 1993.

This program includes a few helper methods for error checking on conditions such as improper file permissions and improper file types given as input. The approach taken to maximize robustness of the program while minimizing possible bugs was to double check both the parameters given, for improper input, as well as catch common mistype mistakes or file i/o errors.



## Installation

To install, run the command:

		make
	
while inside of the directory "cipher.c" is located.


NOTE: If there is any issues running the program, run

		make clean

then,

		make


## Usage

./cipher [OPTIONS] [-p PASSWORD] <infile> <outfile>

Options are as follows:
```
   -p [PASS] :  Use [PASS] as password (skip prompt)
   -d        :  Decrypt <infile> to <outfile>
   -e        :  Encrypt <infile> to <outfile>
   -v        :  Print version number
   -h        :  Show help screen
   -m        :  Enable memory mapping - mmap()
   -s        :  Safe Mode (prompt for password twice)
```


## Changelog

----------------------------
revision 1.26
date: 2012/09/30 21:36:47;
lines: +2 -2

Removed debug flag
----------------------------
revision 1.25
date: 2012/09/30 21:34:43;
lines: +261 -96

Added mmap() - unfinished
----------------------------
revision 1.24
date: 2012/09/30 07:53:35;
lines: +50 -63

Finished STDIN and STDOUT
----------------------------
revision 1.23
date: 2012/09/30 04:16:09;
lines: +180 -147

Doing stdin to stdout
----------------------------
revision 1.22
date: 2012/09/29 21:25:45;
lines: +170 -67

Fixed memory issues with key, finished encrypt/decrypt
----------------------------
revision 1.21
date: 2012/09/28 22:43:56;
lines: +88 -76

Added both decryption and encryption modes
cleaned up code
----------------------------
revision 1.20
date: 2012/09/26 21:23:27;
lines: +107 -33

Added encryption part
----------------------------
revision 1.19
date: 2012/09/26 17:38:12;
lines: +17 -17

More stuff
----------------------------
revision 1.18
date: 2012/09/25 19:37:26;
lines: +86 -36

Added password checking
----------------------------
revision 1.17
date: 2012/09/23 21:40:42;
lines: +14 -21

Fixed formatting
----------------------------
revision 1.16
date: 2012/09/23 21:13:08;
lines: +69 -63

Debugged
----------------------------
revision 1.15
date: 2012/09/22 23:45:23;
lines: +144 -17

Finished all file i/o checking
----------------------------
revision 1.14
date: 2012/09/19 18:47:37;
lines: +9 -10

More work on file i/o error checking
----------------------------
revision 1.13
date: 2012/09/19 18:22:13;
lines: +77 -9

Added more error checking with permissions and file i/o (unfinished)
----------------------------
revision 1.12
date: 2012/09/19 05:41:22;
lines: +54 -13

Completed same file error checking
----------------------------
revision 1.11
date: 2012/09/19 04:26:00;
lines: +10 -4

Versioning parsing completed
----------------------------
revision 1.10
date: 2012/09/19 03:57:00;
lines: +3 -3

Adding parsing for version
----------------------------
revision 1.9
date: 2012/09/19 03:32:11;
lines: +61 -37

Fixed syntax of error checking for same files
----------------------------
revision 1.8
date: 2012/09/19 02:33:10;
lines: +82 -7

Added error checking for same files
----------------------------
revision 1.7
date: 2012/09/18 19:27:21;
lines: +34 -4

Even more error checking
----------------------------
revision 1.6
date: 2012/09/18 17:39:44;
lines: +57 -7

Included more error checking for input and output files
----------------------------
revision 1.5
date: 2012/09/18 16:08:43;
lines: +45 -8

Added reading infile name and outfile name
----------------------------
revision 1.4
date: 2012/09/17 21:39:13;
lines: +3 -3

Added functionality
----------------------------
revision 1.3
date: 2012/09/17 19:51:52;
lines: +32 -23

Added new error checking
----------------------------
revision 1.2
date: 2012/09/17 02:10:05;
lines: +10 -2

Add help menu.
----------------------------
revision 1.1
date: 2012/09/16 21:52:47;

Initial revision
----------------------------

