#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

int main(int argc,char **argv){
	FILE *fp;
	char c;
	int inTag=0;
	bool errTag = false;
	char TagMsg[1024];
	int  iCount = 0;

	if(argc == 1)
		fp = stdin;
	else if (argc == 2)
		fp = fopen(argv[1],"r");
	else{ 
		fprintf(stderr,"Usage:%s <file>\n",argv[1]);
		exit(1);
	}
	while((c = fgetc(fp))!=EOF)
	{
		if(c == '<'){
			inTag = 1;
			continue;
		}
		if(c == '>'){
			inTag = 0;
			continue;

		}
		if(inTag)
		{
			TagMsg[iCount++] = c;
			if(!isalpha(c) && c!='/')
				errTag = true;
		}

		if(!inTag)
		{
			if(errTag)
			{
				TagMsg[iCount] = '\0';
				fprintf (stderr, "Error: illegal tag \"%s\"\n",TagMsg);
				errTag = false ;
			}
			iCount = 0;
			memset( TagMsg, '\0', strlen(TagMsg));
			fputc(c, stdout);
		}
	}  
	fflush(stdout);
	fclose(fp);
	return(0);
}          
