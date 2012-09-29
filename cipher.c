#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "blowfish.h"

/* Handles checking if file exists
 * as well as proper permissions on file
 * SUCCESS CODE(S):
 * 0 = Success (File DOES NOT exist)
 * ERROR CODE(S):
 * 1 = File does exist
 * 2 = File does exist & no permissions
 */
int fileExists(char *fName){
  struct stat buf;
  errno=0;
  int chk = stat(fName, &buf);
  if(errno!=0){
  	return (chk==0);
	} else {
		return 2;
	}
}

/* Handles checking if path is directory
 * SUCCESS CODE(S):
 * 0 = Success (Path is NOT a directory)
 * ERROR CODE(S):
 * 1 = Path is a directory
 * 2 = Out of memory
 */
int isDirectory(char *path){
  struct stat *buf;
  buf = (struct stat*)malloc(sizeof(struct stat));
  if(buf==NULL){
  	free(buf);
  	return 2;
  }
  stat(path,buf);
  if(buf->st_mode & S_IFDIR){
  	free(buf);
    return 0;
  } else {
  	free(buf);
    return 1;
  }
}

/* Handles checking if the given path of a file
 * is pointing to a regular file.
 * (NOTE: Assumes file path exists and is
 * neither a directory or symlink)
 * SUCCESS CODE(S):
 * 0 = Success (Path points to regular file)
 * ERROR CODE(S):
 * 1 = Path is a character device
 * 2 = Path is a block device
 * 3 = Path is a named pipe
 * 4 = Path is a socket
 * 5 = Unknown (symlink or directory)
 */
int isRegularFile(char *path){
	struct stat *buf;
	buf = (struct stat*)malloc(sizeof(struct stat));
	stat(path,buf);
	if(buf->st_mode & S_IFREG){
		free(buf);
		return 0;
	} else if(buf->st_mode & S_IFCHR){
		free(buf);
		return 1;
	} else if(buf->st_mode & S_IFBLK){
		free(buf);
		return 2;
	} else if(buf->st_mode & S_IFIFO){
		free(buf);
		return 3;
	} else if(buf->st_mode & S_IFSOCK){
		free(buf);
		return 4;
	} else {
		free(buf);
		return 5;
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
	struct stat *in_buffer;
	struct stat *out_buffer;
	in_buffer = (struct stat*)malloc(sizeof(struct stat));
	out_buffer = (struct stat*)malloc(sizeof(struct stat));
	stat(in_path,in_buffer);
	stat(out_path,out_buffer);
	// Checks if the basic paths are the same
	if(strcmp(in_path,out_path)==0){
		free(in_buffer); free(out_buffer);
		return 1;
	} else {
		// Check if either are symlinks
		lstat(in_path,in_buffer);
		lstat(out_path,out_buffer);
		if(!(in_buffer->st_mode & S_IFLNK) 
		&& !(out_buffer->st_mode & S_IFLNK)){
			// If both are not links then check to be SURE they are same file
			//  (Aka Hardlinks to same file)
			if((in_buffer->st_dev==out_buffer->st_dev) 
			&& (in_buffer->st_ino==out_buffer->st_ino)){
				free(in_buffer); free(out_buffer);
				return 2;
			} else {
				// They must be pointing to two different files
				// (Not Hardlinks)
				free(in_buffer); free(out_buffer);
				return 0;
			}
		} else if((in_buffer->st_mode & S_IFLNK) 
		&& (out_buffer->st_mode & S_IFLNK)){
			// Both input file & output file are symlinks
			// Check if they point to the same file
			stat(in_path,in_buffer);
			stat(out_path,out_buffer);
			if((in_buffer->st_dev==out_buffer->st_dev) 
			&& (in_buffer->st_ino==out_buffer->st_ino)){
				free(in_buffer); free(out_buffer);
				return 3;
			} else {
				free(in_buffer); free(out_buffer);
				return 0;
			}
		} else if((in_buffer->st_mode & S_IFLNK) 
		&& !(out_buffer->st_mode & S_IFLNK)){
			// Only the input file is a symlink
			// Check if input points to output
			stat(in_path,in_buffer);
			stat(out_path,in_buffer);
			if((in_buffer->st_dev==out_buffer->st_dev) 
			&& (in_buffer->st_ino==out_buffer->st_ino)){
				free(in_buffer); free(out_buffer);
				return 4;
			} else {
				free(in_buffer); free(out_buffer);
				return 0;
			}
		} else {
			// Only the output file is a symlink
			// Check if output points to input
			stat(in_path,in_buffer);
			stat(out_path,out_buffer);
			if((in_buffer->st_dev==out_buffer->st_dev) 
			&& (in_buffer->st_ino==out_buffer->st_ino)){
				free(in_buffer); free(out_buffer);
				return 5;
			} else {
				free(in_buffer); free(out_buffer);
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
  errno = 0;
  // 1=Enable Debug Mode
  // 0=Disable Debug Mode
  int DEBUG = 1;
  int i = 0; // General purpose index
  
  /* Initialize from and to buffers to OS pagesize */
  int page_size=getpagesize();
  unsigned char *from;
  unsigned char *to; 

  /* Temp buffer to store user input (user password) */
	const int PASS_MAX = 16;  
	char *temp_pass;
	char *password_final;
  char temp_buf[17];
  char temp_buf_chk[17];
  char rcs_vers[18] = "$Revision: 1.22 $";
  char *rcs_vers_cp,*version;
  int passArgNum = 0;
  
  /* File names/descriptors/stats */
  int infile_fd;
  int outfile_fd;
  char *infile_name;
  char *outfile_name;
  int stdin_infile;
  int stdout_outfile;
  int sf_code;
  int bytes_read;
  int bytes_write;

  /* Define boolean ints for params */
  int deco, enco, vers, help, mmap, pass, opt, safe;
  
  /* don't worry about these two: just define/use them */
  int n = 0;  /* internal blowfish variables */
  unsigned char iv[8];  /* Initialization Vector */
  
  /* Define a structure to hold the key */
  BF_KEY key;
  
  /* Parse out version properly for display */
  rcs_vers_cp = strdup(rcs_vers);
  version = strtok(rcs_vers_cp," ");
  version = strtok(NULL," ");
  free(rcs_vers_cp);
  
  /* fill the IV with zeros (or any other fixed data) */
  memset(iv, 0, 8);
  
  /* clear buffers */
  memset(temp_buf,0,17);
  memset(temp_buf_chk,0,17);
  
  /* Initialize and check params */
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
    	if(strlen(optarg) > PASS_MAX){
    		fprintf(stderr,"Error Code 3: Invalid Execution\n");
    		fprintf(stderr,"Password cannot exceed 16 characters.\n");
    		exit(3);
    	} else {
      	strcpy(temp_buf, optarg);
      	// Gets argc number from optind for comparison
				//  and error checking later
				passArgNum = optind-1;
    	  pass = 1;
      	break;
      }
    default:
    	fprintf(stderr,"Error Code 3: Invalid execution\n");
      fprintf(stderr,"Usage: %s [OPTIONS] [-p PASSWORD] <infile> <outfile>\n", argv[0]);
      exit(3);
    }
  }
  
  // DEBUGGING CODE //
  if(DEBUG==1){
    printf("\n[----- DEBUGGING ENABLED -----]\n");
    printf("To disable, change DEBUG variable in cipher.c from 1 to 0 and remake\n");
    printf("\ndecode=%i \nencode=%i \nversion=%i \nhelp=%i \nmmap=%i \npass=%i \nsafe=%i\n", deco, enco, vers, help, mmap, pass, safe);

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
    printf("   -v        :  Print version number \n");
    printf("   -h        :  Show help screen (you are looking at it) \n");
    printf("   -m        :  Enable memory mapping - mmap() \n");
    printf("   -s        :  Safe Mode (prompt for password twice)\n");
    exit(1);
  } else if(vers==1){
    printf("Blowfish Cipher Tool - v%s\n", version);
    exit(2);
  } else if(argc>=3){
    // Check for proper format of <infile> and <outfile>
    if((strcmp(argv[passArgNum],argv[argc-2])==0) && passArgNum!=0){
      if(DEBUG==1){
        printf("pass=%s \nargc-2=%s \n",argv[passArgNum],argv[argc-2]);
      }
      fprintf(stderr,"Error Code 4: No <outfile> specified\n");
      exit(4);
    }
    // Take <infile> and <outfile>
    infile_name=(char*)malloc(strlen(argv[argc-2]));
    outfile_name=(char*)malloc(strlen(argv[argc-1]));
    strcpy(outfile_name, argv[argc-1]);
		strcpy(infile_name, argv[argc-2]);
/********************************* BREAKPOINT 1 ***********************************/
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
    	i=fileExists(infile_name);
      if(i==0){
        // <infile> DOES NOT exist
        fprintf(stderr,"Error Code 5: <infile> does not exist\n");
        free(infile_name);
        free(outfile_name);
        exit(5);
      } else if(i==2){
      	// <infile> DOES exist
      	// Is it a directory?
      	if((isDirectory(infile_name))!=1){
      		// <infile> DOES exist AND is a directory
        	fprintf(stderr,"Error Code 6: <infile> is a directory\n");
        	free(infile_name);
       		free(outfile_name);
      	  exit(6);
      	}
      } else {
      	// <infile> DOES exist AND is NOT a directory
      }
    }
    // Postconditions:
    // <infile> DOES exist and IS NOT a directory
    // OR <infile> is set to STDIN
    // with no i/o or permission errors
/******************************** BREAKPOINT 2 ***********************************/
    // ----- <outfile> Error Checking -----
    // (Only if <outfile> is NOT set to STDOUT)
    if(stdout_outfile!=1){
    	if((fileExists(outfile_name))!=1){
    		// <outfile> DOES NOT exist
    		// do nothing - possible error caught
    		// when calling open later on.
    	} else if((isDirectory(outfile_name))!=1){
    		// <outfile> DOES exist AND is directory
    		fprintf(stderr,"Error Code 6: <outfile> is a directory\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(6);
    	} else {
    		// <outfile> DOES exist AND is NOT a directory
    		fprintf(stderr, "Warning: <outfile> exists, overwritting...\n");
    	}
    }
    // Postconditions:
    // <outfile> IS NOT a directory AND exists (either made or overwrote)
    // OR <outfile> is set to STDOUT
/******************************** BREAKPOINT 3 ***********************************/
    // Check if infile and outfile are the same
    sf_code = isSameFiles(infile_name, outfile_name);
    if(sf_code==1){
    	// Paths are the same references
    	fprintf(stderr,"Error Code 7: <infile> and <outfile> are the same path\n");
    	free(infile_name);
    	free(outfile_name);
    	exit(7);
    } else if(sf_code==2){
    	// Hardlinks to same file
    	fprintf(stderr,"Error Code 7: <infile> and <outfile> are hardlinks to same file\n");
    	free(infile_name);
    	free(outfile_name);
    	exit(7);
    } else if(sf_code==3){
    	// In/Out symlinks point to same file
    	fprintf(stderr,"Error Code 7: <infile> and <outfile> are symlinks to same file\n");
    	free(infile_name);
    	free(outfile_name);
    	exit(7);
    } else if(sf_code==4){
    	// Input symlink points to outfile
    	fprintf(stderr,"Error Code 7: <infile> symlink points to <outfile>\n");
    	free(infile_name);
    	free(outfile_name);
    	exit(7);
    } else if(sf_code==5){
    	// Output symlink points to infile
    	fprintf(stderr,"Error Code 7: <outfile> symlink points to <infile>\n");
    	free(infile_name);
    	free(outfile_name);
    	exit(7);
    } else{
    	// No error, continue on
    }
    
    // Check if <infile> or <outfile> is a char/block special device
    switch(isRegularFile(infile_name)){
    	case 0:
    		// Regular File
    		break;
    	case 1:
    		// Character Device
    		fprintf(stderr,"Error Code 7: <infile> is a character device\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	case 2:
    		// Block Device
    		fprintf(stderr,"Error Code 7: <infile> is a block device\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	case 3:
    		// FIFO - Named Pipe
    		fprintf(stderr,"Error Code 7: <infile> is a named pipe\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	case 4:
    		// Socket
    		fprintf(stderr,"Error Code 7: <infile> is a socket\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	default:
    		// Other (symlink or directory, unreachable with preconditions)
    		fprintf(stderr,"Error Code 7: Unknown file <infile>\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    }
    switch(isRegularFile(outfile_name)){
    	case 0:
    		// Regular File
    		break;
    	case 1:
    		// Character Device
    		fprintf(stderr,"Error Code 7: <outfile> is a character device\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	case 2:
    		// Block Device
    		fprintf(stderr,"Error Code 7: <outfile> is a block device\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	case 3:
    		// FIFO - Named Pipe
    		fprintf(stderr,"Error Code 7: <outfile> is a named pipe\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	case 4:
    		// Socket
    		fprintf(stderr,"Error Code 7: <outfile> is a socket\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    	default:
    		// Other (symlink or directory, unreachable with preconditions)
    		fprintf(stderr,"Error Code 7: Unknown file <outfile>\n");
    		free(infile_name);
    		free(outfile_name);
    		exit(7);
    }
    //	Post-conditions: 
    //	<infile> exists, it is NOT a directory, and if it is a 
    //		symlink/hardlink, it points to a seperate regular file
    //		than <outfile>, else it is not a link but a regular file.
    //	<outfile> may exist, and if it does exist and is a
    //		symlink/hardlink, it points to a seperate regular file
    //		than <infile>, else it is not a link but a regular file.    
/******************************** BREAKPOINT 4 ***********************************/
    // DEBUGGING CODE //
    if(DEBUG==1){
      printf("infile_name=%s\n",infile_name);
      printf("outfile_name=%s\n",outfile_name);
      printf("read from stdin=%i\n",stdin_infile);
      printf("print to stdout=%i\n",stdout_outfile);
    }
    ////////////////////
   	
   	// Check for password/safe inputs
   	passArgNum=0; // Re-using var for pass matching
   	password_final=(char*)malloc(PASS_MAX);
   	memset(password_final,0,PASS_MAX);
   	if(pass==1 && safe==0){
   		// temp_buf has password
   		if(DEBUG==1){
   			printf("BEFORE COPY\n");
   			printf("password_final_len=%i\n",strlen(password_final));
   			printf("temp_buf_len=%i\n",strlen(temp_buf));
   		}
   		// BP
   		strcpy(password_final,temp_buf);
   		if(DEBUG==1){
   			printf("AFTER COPY\n");
   			printf("password_final_len=%i\n",strlen(password_final));
   			printf("temp_buf_len=%i\n",strlen(temp_buf));
   		}
   	} else if(pass==0){
   		// Start password matching loop
   		while(passArgNum==0){
   			// Ask for first password
   			if(DEBUG==1){
   				printf("Checking for password..\n");
   			}
   			while(strlen(temp_buf)==0){
   				temp_pass = getpass("Enter Password: ");
   				if((unsigned)strlen(temp_pass) <= 16){
   					strcpy(temp_buf,temp_pass);
   				} else {
   					fprintf(stderr,"Error: Password cannot be longer than 16 characters\n");
   				}
   			}
   			if(DEBUG==1){
   				printf("The 1st password is: %s\r\n",temp_buf);
   			}
   			if(safe==1){
     			// Ask for second password
   				temp_pass = getpass("Confirm Password: ");
   				if((unsigned)strlen(temp_pass) <= 16){
   					strcpy(temp_buf_chk,temp_pass);
   					if(DEBUG==1){
     					printf("The 2nd password is: %s\r\n",temp_buf_chk);
     				}
     				// Do the passwords match?
     				if(strcmp(temp_buf,temp_buf_chk)==0){
     					// Yes! :)
     					passArgNum=1;
     					strcpy(password_final,temp_buf);
     				} else {
     					// No :(
     					passArgNum=0;
     					fprintf(stderr,"Error: Passwords do not match, re-prompting...\n");
     					memset(temp_buf,0,17);
     					memset(temp_buf_chk,0,17);
     				}
   				} else {
   					fprintf(stderr,"Error: Password cannot be longer than 16 characters\n");
   					fprintf(stderr,"Error: Passwords do not match, re-prompting...\n");
   					memset(temp_buf,0,17);
     				memset(temp_buf_chk,0,17);
   				}
     		} else {
     			// No safe statement given, no password to match
     			passArgNum=1;
     			strcpy(password_final,temp_buf);
     		}
     	}
   	} else {
   		// Error both -p and -s given
   		fprintf(stderr,"Error Code 3: Cannot use \'-p\' and \'-s\'\n");
   		free(infile_name);
   		free(outfile_name);
   		exit(3);
   	}
    // temp_buf contains the right password for encryption or decryption
    if(DEBUG==1){
    	printf("password_final= %s \n",password_final);
    }
    password_final[strlen(password_final)]='\0';
/******************************** BREAKPOINT 5 ***********************************/
    /* call this function once to setup the cipher key */
    BF_set_key(&key, PASS_MAX, (unsigned char *)password_final);
/******************************** BREAKPOINT 6 ***********************************/
    /* Open up file descriptors */
    errno=0;
    // Open up infile fd
    infile_fd=open(infile_name, O_RDONLY, S_IREAD);
    if(DEBUG==1){
    	printf("infile_fd= %i \n",infile_fd);
    }
    if(infile_fd==-1){
    	perror("Error Code 8: On <infile> ");
    	free(infile_name);
    	free(outfile_name);
    	exit(8);
    }
/******************************** BREAKPOINT 7 ***********************************/
    errno=0;
    // Open up outfile fd
    outfile_fd=open(outfile_name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IWRITE);
    if(DEBUG==1){
    	printf("outfile_fd= %i \n",outfile_fd);
    }
    if(outfile_fd==-1){
    	perror("Error Code 8: On <outfile> ");
    	free(infile_name);
    	free(outfile_name);
    	exit(8); 
    }
/******************************** BREAKPOINT 8 ***********************************/
    // Initialize to and from and clear them
    to = (unsigned char*)malloc(page_size);
    from = (unsigned char*)malloc(page_size);
    memset(to,0,page_size);
    memset(from,0,page_size);
/******************************** BREAKPOINT 9 ***********************************/
    // PERMISSIONS TEST
    chmod(infile_name,755);
    chmod(outfile_name,755);
/******************************** BREAKPOINT 10 ***********************************/
    ///// [---------- DECRYPTION START ----------] /////
    if(deco==1 && enco==0){
    	printf("Decrypting the file: %s \n",infile_name);
			// Start reading the bytes of length page_size from infile
			bytes_read = -1;
    	do {
    		bytes_read=read(infile_fd,from,page_size);
/******************************** BREAKPOINT 11 ***********************************/
    		if(DEBUG==1){
    			printf("\nbytes_read=%i\n",bytes_read);
    		}
    		if(bytes_read==-1){
    			// Maybe partial read encountered?
    			perror("Error Code 8: On <infile> ");
    			free(infile_name);
    			free(outfile_name);
    			close(infile_fd);
    			close(outfile_fd);
    			exit(8);
    		} else if(bytes_read > 0){
    			// Decrypt the buffer
/******************************** BREAKPOINT 12 ***********************************/
    			BF_cfb64_encrypt(from, to, page_size, &key, iv, &n, BF_DECRYPT);
    			// Was the read even with page_size?
    			if(bytes_read == page_size){
    				// Write buffer to file
/******************************** BREAKPOINT 13 ***********************************/
    				bytes_write=write(outfile_fd,to,page_size);
    				if(DEBUG==1){
    					printf("bytes_read == page_size\n");
    					printf("bytes_write=%i\n",bytes_write);
    				}
    				if(bytes_write==-1){
    					// Maybe partial write encountered?
    					perror("Error Code 8: On <outfile> ");
    					free(infile_name);
    					free(outfile_name);
    					close(infile_fd);
    					close(outfile_fd);
    					exit(8);
    				}
    			} else if(bytes_read < page_size){
/******************************** BREAKPOINT 14 ***********************************/
    				// Last iteration
    				// Write buffer to file
    				bytes_write=write(outfile_fd,to,bytes_read);
    				if(DEBUG==1){
    					printf("bytes_read < page_size\n");
    					printf("bytes_write=%i\n",bytes_write);
    				}
    				if(bytes_write==-1){
    					// Maybe partial write encountered?
    					perror("Error Code 8: On <outfile> ");
    					free(infile_name);
    					free(outfile_name);
    					close(infile_fd);
    					close(outfile_fd);
    					exit(8);
    				}
    			}
    			// Clear buffers
    			memset(to,0,page_size);
    			memset(from,0,page_size);
    		} // else bytes_read == 0, terminate while
    	} while(bytes_read!=0);
    printf("Saving to: %s \n",outfile_name);
    ///// [---------- DECRYPTION END ----------] /////
/******************************** BREAKPOINT 15 ***********************************/
    ///// [---------- ENCRYPTION START ----------] /////
    } else if(deco==0 && enco==1){
    	printf("Encrypting the file: %s \n",infile_name);
    	// Start reading the bytes of length page_size from infile
    	bytes_read=-1;
    	do {
    		bytes_read=read(infile_fd,from,page_size);
/******************************** BREAKPOINT 16 ***********************************/
    		if(DEBUG==1){
    			printf("\nbytes_read=%i\n",bytes_read);
    		}
    		if(bytes_read==-1){
    			// Maybe partial read encountered?
    			perror("Error Code 8: On <infile> ");
    			free(infile_name);
    			free(outfile_name);
    			close(infile_fd);
    			close(outfile_fd);
    			exit(8);
    		} else if(bytes_read > 0){
/******************************** BREAKPOINT 17 ***********************************/
    			// Encrypt the buffer
    			BF_cfb64_encrypt(from, to, page_size, &key, iv, &n, BF_ENCRYPT);
/******************************** BREAKPOINT 18 ***********************************/
    			// Was the read even with page_size?
    			if(bytes_read == page_size){
/******************************** BREAKPOINT 19 ***********************************/
    				// Write buffer to file
						bytes_write=write(outfile_fd,to,page_size);
    				if(DEBUG==1){
    					printf("bytes_read == page_size\n");
    					printf("bytes_write=%i\n",bytes_write);
    				}
    				if(bytes_write==-1){
    					// Maybe partial write encountered?
    					perror("Error Code 8: On <outfile> ");
    					free(infile_name);
    					free(outfile_name);
    					close(infile_fd);
    					close(outfile_fd);
    					exit(8);
    				}
    			} else if(bytes_read < page_size){
/******************************** BREAKPOINT 20 ***********************************/
    				// Last iteration
    				// Write buffer to file
    				bytes_write=write(outfile_fd,to,bytes_read);
    				if(DEBUG==1){
    					printf("bytes_read < page_size\n");
    					printf("bytes_write=%i\n",bytes_write);
    				}
    				if(bytes_write==-1){
    					// Maybe partial write encountered?
    					perror("Error Code 8: On <outfile> ");
    					free(infile_name);
    					free(outfile_name);
    					close(infile_fd);
    					close(outfile_fd);
    					exit(8);
    				}
    			}
    			// Clear buffers
    			memset(to,0,page_size);
    			memset(from,0,page_size);
    		} // else bytes_read == 0, terminate while
    	} while(bytes_read!=0);
    printf("Saving to: %s \n",outfile_name);
    ///// [---------- ENCRYPTION END ----------] /////
/******************************** BREAKPOINT 21 ***********************************/
    // Both Encrypt/Decrypt OR Neither Encrypt/Decrypt
    } else {
      fprintf(stderr,"Error Code 3: Invalid execution\n");
      fprintf(stderr,"Must use EITHER Encrypt (-e) OR Decrypt (-d)\n");
      fprintf(stderr,"       i.e.  %s [-e|-d] <infile> <outfile>\n",argv[0]);
      free(infile_name);
      free(outfile_name);
      exit(3);
    }
  	free(infile_name);
  	free(outfile_name);
  	free(to);
  	free(from);
  	free(password_final);
  	close(infile_fd);
  	close(outfile_fd);
  } else{
 		fprintf(stderr,"Error Code 3: Invalid execution\n");
    fprintf(stderr,"Must include <infile> and <outfile> parameters\n");
    fprintf(stderr,"       i.e.  %s [-e|-d] <infile> <outfile>\n", argv[0]);
    exit(3);
  }
  exit(0);
}
