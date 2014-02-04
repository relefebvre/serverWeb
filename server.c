#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>


//char *path;

typedef struct {
	int socket; 
} argTh;

void *codeFrere(void *arg)
{
	argTh *val = arg;
	char buffer[2048];	
	char *pathName;
	int file;

	if (read(val->socket,buffer,sizeof(buffer)) < 0 )
	{
		fprintf(stderr,"Read error on thread\n");
		exit(1);
	}
	
	printf("Buffer : %s\n",buffer);
	//file = open(buffer,"r");
		
	free(val);
	pthread_detach(pthread_self());
}

void setPath(char **path, char *src)
{
	*path = (char *)malloc(sizeof(src));
        if (*path == NULL)
        {
          perror("malloc ");
          exit(errno);
        }
        strcpy(*path,src);
}

int main(int argc, char** argv)
{
	char c;
	char *path=NULL;
	unsigned long maxTh = 20;
	unsigned debug = 0;
	argTh *arg;	
        int fd, cfd;
        struct sockaddr_in sin, cin;
        unsigned int port=8080;
        char hostname[]="0.0.0.0";
        socklen_t taille;
        struct hostent *hostinfo;


	int option_index=0;
	struct  option long_options[] = {
	{"debug",	0, 0, 0},
	{"root",	1, 0, 0},
	{"max",		1, 0 ,0},
	{0, 		0, 0, 0}
	};	

	while( (c = getopt_long(argc, argv, "dr:m:",long_options,&option_index)) != -1)
	{
		switch (c) {
			case 0 :
				if (strcmp(long_options[option_index].name,"debug") == 0) 
				{
					if (optarg)
					{
						printf("Debug ne prend pas d'argument\n");
						exit(1);
					}
				}
				if (strcmp(long_options[option_index].name,"root") == 0)
				{
					setPath(&path,optarg);
				}
				if (strcmp(long_options[option_index].name,"max") == 0)
				{
					sscanf(optarg,"%lu",&maxTh);
				}
				break;
			case 'r' :
				setPath(&path,optarg);
				break;
			case 'm' :
				sscanf(optarg,"%lu",&maxTh);
				break;
			case 'd' :
				debug = 1;
				break;
			default :
				printf("erreur\n");
				break;
		}
	}
	
	if (path == NULL)
	{
		setPath(&path,"./html");
	}
	
        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
        hostinfo = gethostbyname (hostname);
        if (hostinfo == NULL)
	{
        	fprintf (stderr, "Unknown host %s.\n", hostname);
        	exit (EXIT_FAILURE);
        }
    	sin.sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];

        fd = socket(AF_INET,SOCK_STREAM,0);

        if (bind(fd,(struct sockaddr *) &sin, sizeof(sin)) != 0)
        {
                perror("bind : ");
                exit(errno);
        }
	
	if (listen(fd,maxTh) != 0)
        {
                perror("listen : ");
                exit(errno);
        }

	while ((cfd = accept(fd,(struct sockaddr*)&cin,&taille)))
	{
		pthread_t th;
		arg = (argTh *)malloc(sizeof(argTh));
		arg->socket = cfd;
		if (pthread_create (&th,NULL,codeFrere,(void *)&arg) < 0)
		{
			fprintf(stderr,"pthread_create\n");
			exit(1);
		}
	}


	return 0;
}
