#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "blowfish.h"

/* Handles checking if file exists
 * SUCCESS CODE(S):
 * 0 = Success (File DOES NOT exist)
 * ERROR CODE(S):
 * 1 = File does exist
 */
int fileExists(char *fName){
  struct stat buffer;
  return (stat(fName, &buffer)==0);
}

/* Handles checking if path is directory
 * SUCCESS CODE(S):
 * 0 = Success (Path is NOT a directory)
 * ERROR CODE(S):
 * 1 = Path is a directory
 */
int isDirectory(char *path){
  struct stat buffer;
  stat(path,&buffer);
  if(buffer.st_mode & S_IFDIR){
    return 0;
  } else {
    return 1;
  }
}

/* Handles checking if two files are the same.
 * This method assumes the files exist.
 * (checks for symlinks and hardlinks)
 * SUCCESS CODE(S):
 * 0 = Success (NOT the same file) 
 * ERROR CODE(S):
 * 1 = Paths are the same (same referencing)
 * 2 = Hardlinks to same file
 * 3 = In/Out symlinks point to same file
 * 4 = Input symlink points to outfile
 * 5 = Output symlink points to infile
 */
int isSameFiles(char *in_path, char *out_path){
	struct stat in_buffer;
	struct stat out_buffer;
	stat(in_path,&in_buffer);
	stat(out_path,&out_buffer);
	// Checks if the basic paths are the same
	if(strcmp(in_path,out_path)==0){
		return 1;
	} else {
		// Check if either are symlinks
		lstat(in_path,&in_buffer);
		lstat(out_path,&out_buffer);
		if(!(in_buffer.st_mode & S_IFLNK) && !(out_buffer.st_mode & S_IFLNK)){
			// If both are not links then check to be SURE they are same file
			//  (Aka Hardlinks to same file)
			if((in_buffer.st_dev==out_buffer.st_dev) && (in_buffer.st_ino==out_buffer.st_ino)){
				return 2;
			} else {
				// They must be pointing to two different files
				// (Not Hardlinks)
				return 0;
			}
		} else if((in_buffer.st_mode & S_IFLNK) && (out_buffer.st_mode & S_IFLNK)){
			// Both input file & output file are symlinks
			// Check if they point to the same file
			stat(in_path,&in_buffer);
			stat(out_path,&out_buffer);
			if((in_buffer.st_dev==out_buffer.st_dev) && (in_buffer.st_ino==out_buffer.st_ino)){
				return 3;
			} else {
				return 0;
			}
		} else if((in_buffer.st_mode & S_IFLNK) && !(out_buffer.st_mode & S_IFLNK)){
			// Only the input file is a symlink
			// Check if input points to output
			stat(in_path,&in_buffer);
			stat(out_path,&in_buffer);
			if((in_buffer.st_dev==out_buffer.st_dev) && (in_buffer.st_ino==out_buffer.st_ino)){
				return 4;
			} else {
				return 0;
			}
		} else {
			// Only the output file is a symlink
			// Check if output points to input
			stat(in_path,&in_buffer);
			stat(out_path,&out_buffer);
			if((in_buffer.st_dev==out_buffer.st_dev) && (in_buffer.st_ino==out_buffer.st_ino)){
				return 5;
			} else {
				return 0;
			}
		}
	}
}

/* [-------- Main Method --------]
 * SUCCESS CODE(S):
 * 0 = Encrypt OR Decrypt Successful
 * 1 = Help menu displayed for user
 * 2 = Version displayed for user
 * ERROR CODE(S):
 * 3 = Invalid execution
 * 4 = No <outfile> given
 * 5 = <infile> does not exist
 * 6 = <infile> OR <outfile> is directory
 * 7 = Same file error
 * 8 = File i/o OR permission error
 */
int main(int argc, char *argv[]){
  // 1=Enable Debug Mode
  // 0=Disable Debug Mode
  int DEBUG = 1;
  errno = 0;
  
  /* Initialize from and to buffers to OS pagesize */
  //int PAGE_SIZE=getpagesize();
  //char from[PAGE_SIZE], to[PAGE_SIZE];

  /* Temp buffer to store user input (user password) */
  char temp_buf[16];
  //char temp_buf_chk[16];
  char rcs_vers[18] = "$Revision: 1.14 $";
  char *rcs_vers_cp,*version;
  int passArgNum = 0;
  
  /* File names/descriptors/stats */
  FILE *infile;
  FILE *outfile;
  char *infile_name;
  char *outfile_name;
  int stdin_infile;
  int stdout_outfile;
  int sf_code;
  int len;

  /* Define boolean ints for params */
  int deco, enco, vers, help, mmap, pass, opt, safe;
  /* Define a structure to hold the key */
  // BF_KEY key;
  
  /* Parse out version properly for display */
  rcs_vers_cp = strdup(rcs_vers);
  version = strtok(rcs_vers_cp," ");
  version = strtok(NULL," ");
  free(rcs_vers_cp);
  
  /* Initialize and check params */
  len = 128;
  stdin_infile = 0; stdout_outfile = 0;
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
      // Gets argc number from optind for comparison
      //  and error checking later
      passArgNum = optind-1;
      pass = 1;
      break;
    default:
    	fprintf(stderr,"Error Code 3: Invalid execution\n");
      fprintf(stderr,"Usage: %s [OPTIONS] [-p PASSWORD] <infile> <outfile>\n", argv[0]);
      return 3;
    }
  }
  
  // DEBUGGING CODE //
  if(DEBUG==1){
    printf("\n[----- DEBUGGING ENABLED -----]\n");
    printf("To disable, change DEBUG variable in cipher.c from 1 to 0 and remake\n");
    printf("\ndecode=%i \nencode=%i \nversion=%i \nhelp=%i \nmmap=%i \npass=%i \n", deco, enco, vers, help, mmap, pass);

    if(pass==1){
      printf("temp_buf=%s \npassArgNum=%i\n",temp_buf,passArgNum);
    } else {
      printf("temp_buf=<BLANK> \npassArgNum=%i\n",passArgNum);
    }
    printf("argc=%i\n",argc);
  }
  ////////////////////
  
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
    return 1;
  } else if(vers==1){
    printf("Blowfish Cipher Tool - v%s\n", version);
    return 2;
  } else if(argc>=3){
    // Check for proper format of <infile> and <outfile>
    if((strcmp(argv[passArgNum],argv[argc-2])==0) && passArgNum!=0){
      if(DEBUG==1){
        printf("pass=%s \nargc-2=%s \n",argv[passArgNum],argv[argc-2]);
      }
      fprintf(stderr,"Error Code 4: No <outfile> specified\n");
      return 4;
    }
    // Take <infile> and <outfile>
    infile_name=(char*)malloc(strlen(argv[argc-2]));
    outfile_name=(char*)malloc(strlen(argv[argc-1]));
    strcpy(outfile_name, argv[argc-1]);
		strcpy(infile_name, argv[argc-2]);
    // MAKE SURE TO FREE THESE
    
    // Check if stdout or stdin is used
    //  in replace of <infile> or <outfile>
    if(strcmp(infile_name,"-")==0){
      stdin_infile=1;
    }
    if(strcmp(outfile_name,"-")==0){
      stdout_outfile=1;
    }
    
    // ----- <infile> Error Checking -----
    // (Only if <infile> is NOT set to STDIN)
    if(stdin_infile!=1){
      if((fileExists(infile_name))!=1){
        // <infile> DOES NOT exist
        fprintf(stderr,"Error Code 5: <infile> does not exist\n");
        free(infile_name);
        free(outfile_name);
        return 5;
      } else if((isDirectory(infile_name))!=1){
        // <infile> DOES exist AND is a directory
        fprintf(stderr,"Error Code 6: <infile> is a directory\n");
        free(infile_name);
        free(outfile_name);
        return 6;
      } else {
      	// <infile> DOES exist AND is NOT a directory
      	// Check permissions and file type of <infile>
      	errno=0;
      	infile = fopen(infile_name,"r");
      	if(errno != 0){
      		perror("Error Code 8: On <infile>");
      		free(infile_name);
      		free(outfile_name);
      		return 8;
      	}
      }
    }
    // Postconditions:
    // <infile> DOES exist and IS NOT a directory
    // OR <infile> is set to STDIN
    // with no i/o or permission errors
    
    // ----- <outfile> Error Checking -----
    // (Only if <outfile> is NOT set to STDOUT)
    if(stdout_outfile!=1){
    	if((fileExists(outfile_name))!=1){
    		// <outfile> DOES NOT exist
    		// Check permissions of creating <outfile>
    		errno=0
    		outfile = fopen(outfile_name,"w");
    		if((outfile==NULL) || (errno !=0)){
    			perror("Error Code 8: On <outfile>");
    			free(infile_name);
    			free(outfile_name);
    			return 8;
    		}
    	} else if((isDirectory(outfile_name))!=1){
    		// <outfile> DOES exist AND is directory
    		fprintf(stderr,"Error Code 6: <outfile> is a directory\n");
    		free(infile_name);
    		free(outfile_name);
    		return 6;
    	} else {
    		// <outfile> DOES exist AND is NOT a directory
    		// Check permissions and file type of <outfile>
    	}
    }
    // Postconditions:
    // <outfile> IS NOT a directory
    // OR <outfile> is set to STDOUT
    
    // Check if infile and outfile are the same
    // (NOTE: Assumes both files exist)
    sf_code = isSameFiles(infile_name, outfile_name);
    if(sf_code==1){
    	// Paths are the same references
    	fprintf(stderr,"Error Code 7: <infile> and <outfile> are the same path\n");
    	free(infile_name);
    	free(outfile_name);
    	return 7;
    } else if(sf_code==2){
    	// Hardlinks to same file
    	fprintf(stderr,"Error Code 7: <infile> and <outfile> are hardlinks to same file\n");
    	free(infile_name);
    	free(outfile_name);
    	return 7;
    } else if(sf_code==3){
    	// In/Out symlinks point to same file
    	fprintf(stderr,"Error Code 7: <infile> and <outfile> are symlinks to same file\n");
    	free(infile_name);
    	free(outfile_name);
    	return 7;
    } else if(sf_code==4){
    	// Input symlink points to outfile
    	fprintf(stderr,"Error Code 7: <infile> symlink points to <outfile>\n");
    	free(infile_name);
    	free(outfile_name);
    	return 7;
    } else if(sf_code==5){
    	// Output symlink points to infile
    	fprintf(stderr,"Error Code 7: <outfile> symlink points to <infile>\n");
    	free(infile_name);
    	free(outfile_name);
    } else{
    	// No error, continue on
    }
    
    // Check if <infile> or <outfile> is a char/block special device
    
    // Check if current user contains permissions for both files
    
    
    
    //	Preconditions: 
    //	<infile> exists, it is NOT a directory, and if it is a 
    //		symlink/hardlink, it points to a seperate file than 
    //		<outfile>, else it is not a link but a regular file.
    //	<outfile> may exist, and if it does exist and is a
    //		symlink/hardlink, it points to a seperate file than
    //		<infile>, else it is not a link but a regular file	 
    
    
    // DEBUGGING CODE //
    if(DEBUG==1){
      printf("infile_name=%s\n",infile_name);
      printf("outfile_name=%s\n",outfile_name);
      printf("read from stdin=%i\n",stdin_infile);
      printf("print to stdout=%i\n",stdout_outfile);
    }
    ////////////////////
    
    // Decryption Mode
    if(deco==1 && enco==0){
      // Password in cmd arg
      if(pass==1){
        // temp_buf has password
        
      } else{
        if(safe==1){
          // Ask for password twice
          
        } else{
          // Ask for password once
          
        }
      }
    // Encryption Mode
    } else if(deco==0 && enco==1){
      // Password in the cmd arg
      if(pass==1){
        // temp_buf has password
      } else{
        if(safe==1){
          // Ask for password twice
          
        } else{
          // Ask for password once
          
        }
      }
    // Both Encrypt/Decrypt OR Neither Encrypt/Decrypt
    } else {
      fprintf(stderr,"Error Code 3: Invalid execution\n");
      fprintf(stderr,"Must use EITHER Encrypt (-e) OR Decrypt (-d)\n");
      fprintf(stderr,"       i.e.  %s [-e|-d] <infile> <outfile>\n",argv[0]);
      return 3;
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
     * onto output buffer "to", using key "key".  Just pass "iv" and "&n" as
     * shown, and don't forget to actually tell the function to BF_ENCRYPT.
     */
    // BF_cfb64_encrypt(from, to, len, &key, iv, &n, BF_ENCRYPT);

    /* Decrypting is the same: just pass BF_DECRYPT instead */
    // BF_cfb64_encrypt(from, to, len, &key, iv, &n, BF_DECRYPT);
  
  } else{
 		fprintf(stderr,"Error Code 3: Invalid execution\n");
    fprintf(stderr,"Must include <infile> and <outfile> parameters\n");
    fprintf(stderr,"       i.e.  %s [-e|-d] <infile> <outfile>\n", argv[0]);
    return 3;
  }
  return 0;
}
