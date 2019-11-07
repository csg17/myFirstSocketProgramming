#include <stdio.h> 

#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 

int 
main (int argc, char const *argv[]) 
{ 
	// ./client 127.0.0.0:8090 get a.txt ||  ./client 127.0.0.0:8090 list
	struct sockaddr_in serv_addr; 
	int sock_fd ;
	int s, len ;
	char buffer[1024] = {0}; 
	char * data ;

	char * cmdline ; // 'list' or 'get'
	char * filename ;

	if (argc == 3 && strcmp(argv[2], "list") == 0) {
		cmdline = strdup("#list") ;
	}
	else if (argc == 4 && strcmp(argv[2], "get") == 0) {
		cmdline = strdup(argv[3]) ; //파일 명을 보내주네요..
	}
	else {
		//argv[1]: IP
		//argv[2]: port
		//argv[3]: command = {list, get}
		//argv[4]: filename
		fprintf(stderr, "Wrong number of arguments\n") ;
		exit(EXIT_FAILURE) ;
	}
	
	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	//port number 분리해주기. 
	char first[1024];
	strcpy(first, argv[1]);

	char *ptr = strtok(first, ":");
	char args1[16]/*IP*/, args2[16]/*port #*/;
	for(int i=0; i<2; i++){
		if(i==0){
			strcpy(args1, ptr);
		}
		else if(i==1){
			strcpy(args2, ptr);
		}

		if(ptr==NULL) { break; }
		printf("%s\n", ptr);
		ptr = strtok(NULL, ":");
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	//serv_addr.sin_port = htons(atoi(args2)); 
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(9013);

	/*if (inet_pton(AF_INET, args1, &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	}*/
	int connection_status;
	if (connection_status = connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("conn: %d \n", connection_status);
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
	printf("[CONNECT]\n");
	// 연결됨!!!!! 서버랑 연결됨!!!!!1 
	
	data = cmdline ;
	len = strlen(cmdline) ;
	s = 0 ;
	while (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
		data += s ;
		len -= s ;
	}
	shutdown(sock_fd, SHUT_WR) ;
	
    // 그럼 이제 받아온걸 읽어주거나 파일 만들어서 저장.
	if (strcmp(cmdline, "#list") == 0) {
		char buf[1024] ;
		//dat//a = 0x0 ;
		//len = 0 ;
		while ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
			buf[s] = 0x0 ;
			printf(">>> %s", buf) ;
		}
		printf("\n") ;
	}
	else {
        	FILE *fp2;
        	char buffer2[1024];
        	int recv_file = 0;

        	fp2 = fopen("receivedFile.txt", "a");

			printf("[GET]\n");
        	if(fp2==NULL){
            		printf("File cannot be opened.ㅜㅜ\n");
        	}
        	bzero(buffer2, 1024);	

       		while((recv_file = recv(sock_fd, buffer2, 1023, 0))>0){
				printf("number of %d\n", recv_file);
				int check_write = fwrite(buffer2, sizeof(char), recv_file, fp2);

				if(check_write < recv_file) {
					printf("Error ; file write\n");
				}
				bzero(buffer2, 1024);
        	}

        printf("We ended receiving the file and creating the file.\n");
	}
} 
