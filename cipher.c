#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "blowfish.h"

int main(int argc, char *argv[]){
  int DEBUG = 1;

  //char from[128], to[128];

  /* Temp buffer to store user input (user password) */
  char temp_buf[16];
  //char temp_buf_chk[16];
  char *version = "$Revision: 1.5 $";
  char *infile_name;
  char *outfile_name;
  int len;

  /* define boolean ints for params */
  int deco, enco, vers, help, mmap, pass, opt, safe;
  /* define a structure to hold the key */
  // BF_KEY key;

  /* initialize and check params */
  len = 128;
  deco = 0; enco = 0; vers = 0; help = 0; mmap = 0; pass = 0; safe = 0;
  while((opt = getopt(argc, argv, "devhmsp:")) != -1) {
    switch(opt){
    case 'd':
      deco = 1;
      break;
    case 'e':
      enco = 1;
      break;
    case 'v':
      vers = 1;
      break;
    case 'h':
      help = 1;
      break;
    case 'm':
      mmap = 1;
      break;
    case 's':
      safe = 1;
      break;
    case 'p':
      strcpy(temp_buf, optarg);
      pass = 1;
      break;
    default:
      fprintf(stderr, "Usage: %s [OPTIONS] [-p PASSWORD] <infile> <outfile>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  
  if(DEBUG==1){
    printf("\ndecode=%i \nencode=%i \nversion=%i \nhelp=%i \nmmap=%i \npass=%i \n", deco, enco, vers, help, mmap, pass);

    if(pass==1){
      printf("\ntemp_buf=%s\n",temp_buf);
    } else {
      printf("\ntemp_buf=<BLANK>\n");
    }
  }

  if(help==1){
    printf("Blowfish Cipher Tool by Cody Moore \n");
    printf("Usage: %s [OPTIONS] [-p PASSWORD] <infile> <outfile> \n", argv[0]);
    printf("Options: \n");
    printf("   -p [PASS] :  Use [PASS] as password (skip prompt) \n");
    printf("   -d        :  Decrypt <infile> to <outfile> \n");
    printf("   -e        :  Encrypt <infile> to <outfile> \n");
    printf("   -v        :  Print out version number \n");
    printf("   -h        :  Show help screen (you are looking at it) \n");
    printf("   -m        :  Enable memory mapping - mmap() \n");
    printf("   -s        :  Safe Mode (prompt for password twice\n");
  } else if(vers==1){
    printf("Blowfish Cipher Tool - %s\n", version);
  } else{
    // Take <infile> and <outfile>
    infile_name=(char*)malloc(sizeof(char)*strlen(argv[argc-2]));
    outfile_name=(char*)malloc(sizeof(char)*strlen(argv[argc-1]));
    strcpy(outfile_name, argv[argc-1]);
    strcpy(infile_name, argv[argc-2]);
    if(DEBUG==1){
      printf("infile_name=%s\n",infile_name);
      printf("outfile_name=%s\n",outfile_name);
    }
    // Decryption Mode
    if(deco==1 && enco==0){
      // Password in cmd arg
      if(pass==1){
        // temp_buf has password
        
      } else{
        if(safe==1){
      
        } else{
      
        }
      }
    // Encryption Mode
    } else if(deco==0 && enco==1){
      // Password in the cmd arg
      if(pass==1){
        // temp_buf has password
      } else{
        if(safe==1){

        } else{

        }
      }
    // Both Encrypt/Decrypt OR Neither Encrypt/Decrypt
    } else {
      fprintf(stderr, "Error: Must use EITHER Encrypt (-e) OR Decrypt (-d)\n");
    }	

    /* don't worry about these two: just define/use them */
    // int n = 0;  /* internal blowfish variables */
    // unsigned char iv[8];  /* Initialization Vector */

    /* fill the IV with zeros (or any other fixed data) */
    // memset(iv, 0, 8);

    /* call this function once to setup the cipher key */
    // BF_set_key(&key, 16, temp_buf);

    /*
     * This is how you encrypt an input char* buffer "from", of length "len"
     * onto output buffer "to", using key "key".  Jyst pass "iv" and "&n" as
     * shown, and don't forget to actually tell the function to BF_ENCRYPT.
     */
    // BF_cfb64_encrypt(from, to, len, &key, iv, &n, BF_ENCRYPT);

    /* Decrypting is the same: just pass BF_DECRYPT instead */
    // BF_cfb64_encrypt(from, to, len, &key, iv, &n, BF_DECRYPT);
  
  }
  return 0;
}
