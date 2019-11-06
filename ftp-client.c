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
	else if (argc == 4 && strcmp(argv[3], "get") == 0) {
		cmdline = strdup(argv[4]) ; //파일 명을 보내주네요..
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
	char *ptr = strtok(argv[2], ":");
	char args1[16]/*IP*/, args2[16]/*port #*/;
	int i=0;
	for(int i=0; i<2; i++){
		strcpy(args1, ptr);

		if(ptr==NULL) { break; }
		ptr = strtok(NULL, ":");
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(atoi(args2)); 
	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
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
			printf("%s", buf) ;
		}
		printf("\n") ;
	}
	else /* get file */ {
        FILE *fp2;
        char* buffer2;
        int size = 2048;
        int recv_file;

        fp2 = fopen("recvieved.txt", "a");

        if(fp2==NULL){
            printf("File cannot be opened.ㅜㅜ\n");
        }
        bzero(buffer2, 2048);
        while((recv_file = recv(sock_fd, buffer2, 2048, 0))>0){
                int check_write = fwrite(/*stream: 정보 받아올 원천*/buffer2, sizeof(char), recv_file, fp2);
                if(check_write < recv_file) {
                    printf("Error ; file write\n");
                }
                bzero(buffer2, size);
        }
        printf("We ended receiving the file and creating the file.\n");
	}
} 
