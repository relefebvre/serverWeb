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
#include <sys/stat.h>
#include <fcntl.h>


typedef struct {
	int socket; 
} argTh;

typedef struct {
	char *version;
	char *codeRetour;
	char *filename;
} enTete;

void *codeFrere(void *arg)
{
	argTh *val = arg;
	char buffer[2048];	
	char *pathName;
	int file,nbSend;
	enTete tete;
	//struct msghdr enTeteHtml;
	//struct iovec msg; 
	char *reponse;

	if (read(val->socket,buffer,sizeof(buffer)) < 0 )
	{
		fprintf(stderr,"Read error on thread\n");
		exit(1);
	}
	
	parser(&tete,buffer);
	printf("File : %s\n",tete.filename);
	printf("Version : %s\n",tete.version);
	file = openFile(&tete);

	/*Tentage de truc pour sendmsg()*/
	/*msg.iov_base = &tete;
	msg.iov_len = strlen(tete.version)+strlen(tete.codeRetour);
	enTeteHtml.msg_iov = &msg;	
	*/

	reponse = (char *)malloc(strlen(tete.version)+strlen(tete.codeRetour)+2);
	if (reponse == NULL)
	{
		close(val->socket);
		printf("Problème malloc\n");
		exit(errno);
	}

	nbSend = write(val->socket,tete.version,strlen(tete.version)-1);
	nbSend = write(val->socket,tete.codeRetour,strlen(tete.codeRetour));
	strcpy(buffer,"\n");
	nbSend = write(val->socket,buffer,strlen(buffer));
	//while((nbSend = read(file,buffer,sizeof(buffer))) > 0)
	//	nbSend = write(val->socket,buffer,nbSend); 


	printf("NbLu : %d\n",nbSend);
	
	if (file == 0)
	{
	}
		
	close(val->socket);
	free(val);
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}

int openFile(enTete *tete)
{
	int ret;
	ret = open(tete->filename,O_RDONLY);
	
	if (ret >= 0)
		tete->codeRetour = strdup(" 200 OK\r\n");
	else
		tete->codeRetour = strdup(" 404 Not Found\r\n");
	return ret;
}

void parser(enTete *tete, char *http)
{
	char *token=NULL;
	token = strtok(http," ");
	while(token)
	{
		switch (token[0]) {
			case '/' :
				tete->filename = strdup(&token[1]);
				break;
			case 'H' :
				token = strtok(token,"\n");
				tete->version = strtok(token,"\n");
				break;
		}

		token = strtok(NULL," ");
	}
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
        int fd, cfd;
        struct sockaddr_in sin, cin;
        unsigned int port=8080;
        char hostname[]="127.0.0.1";
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
		argTh *arg;
		arg = (argTh *)malloc(sizeof(argTh));
		arg->socket = cfd;
		if (pthread_create (&th,NULL,codeFrere,arg) < 0)
		{
			fprintf(stderr,"pthread_create\n");
			exit(1);
		}
	}


	return 0;
}
