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
