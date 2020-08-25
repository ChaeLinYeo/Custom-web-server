/*
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>

// 추가한 헤더
#include <string.h>
#include <time.h>
#include <fcntl.h>

#define MAX_BUFF	(2048 * 2)
// 에러 처리 함수
void error(char* msg)
{
	perror(msg);
	exit(1);
}

// 메인 함수
int main(int argc, char* argv[])
{
	int sockfd = 0, newsockfd = 0; //descriptors rturn from socket and accept system calls
	int portno = 0; // 포트 번호 
	socklen_t clilen;
	char szRecv[MAX_BUFF] = { 0 }; // Client의 요청 메시지를 저장
	
	 /*sockaddr_in: Structure Containing an Internet Address*/
	struct sockaddr_in serv_addr, cli_addr;

	int n = 0;
	if (argc < 2) // 입력에 대한 오류 처리 
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	/*Create a new socket
	  AF_INET: Address Domain is Internet
	  SOCK_STREAM: Socket Type is STREAM Socket */
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // 소켓 생성. 
	// PF_INET는 IPv4이다. 소켓의 생성된 값은 sockfd라는 파일디스크립터 변수에 저장된다.
	if (sockfd < 0)
	{
		error("ERROR opening socket");
	}

	bzero((char*)& serv_addr, sizeof(serv_addr));
	// bzero는 서버 어드레스 내용(&serv_addr, 주소에 있는 필드를)을 서버 어드레스의 크기만큼 0으로 채워준다. 즉, 서버 어드레스 변수를 0으로 초기화해준다.
	portno = atoi(argv[1]); //atoi converts from String to Integer
	serv_addr.sin_family = AF_INET; // serv_addr 구조체를 채워주게 되는데, AF_INET은 IPv4를 쓰겠다는 것이다. 
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //for the server the IP address is always the address that the server is running on
	// INADDR_ANY는 서버의 주소를 자동으로 채워주는 매크로이다. host to network long타입으로 해서 s_addr에 집어넣어준다.
	serv_addr.sin_port = htons(portno); //convert from host to network byte order
	// portno를 htons로 해서 sin_port에 넣어준다. 

	// IP와 포트 값을 서버의 sockfd에 바인딩해준다. 
	if (bind(sockfd, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
	{
		error("ERROR on binding");
	}

	listen(sockfd, 5); // Listen for socket connections. Backlog queue (connections to wait) is 5
	// sockfd를 넣고, 큐의 크기는 5로 한다. 

	while (1) //http request를 여러번 받는다. 
	{
		clilen = sizeof(cli_addr); //클라이언트 주소의 길이. 클라이언트 주소의 사이즈를 저장하고 clien변수에 저장한다.
		/*accept function:
		  1) Block until a new connection is established
		  2) the new socket descriptor will be used for subsequent communication with the newly connected client.
		*/
		newsockfd = accept(sockfd, (struct sockaddr*) & cli_addr, &clilen);
		// 클라이언트 주소의 사이즈를 저장하고 clien변수에 저장한 뒤 &clilen로 변수를 집어넣어 주고 sockaddr 타입으로 변환해준다(*&사라짐). 
		// accept의 크기를 newsockfd에 집어넣어 준다. 
		if (newsockfd < 0)
		{
			error("ERROR on accept"); // newsockfd가 0보다 작다면 오류 처리
		}

		bzero(szRecv, MAX_BUFF); // newsockfd가 0보다 작지 않다면 Client의 요청 메시지를 저장하는 szRecv를 0으로 초기화 
		n = read(newsockfd, szRecv, MAX_BUFF); //newsockfd를 읽어오는데 szRecv로 MAX_BUFF 크기만큼 읽어온다.
		//Read 는 block function이다. 
		
		if (n < 0) // 읽어온 크기가 0보다 작으면 에러
		{
			error("ERROR reading from socket");
		}
		if (n == 0) //읽어온 크기가 0과 같으면(127.0.0.1:portnum만 입력했을때 서버가 죽는 예외 처리를 위함)
		{
			printf("n is zero\n");
			close(newsockfd);
			continue;
		}
		printf("%s", szRecv);

		/*
			Web Server 주요 기능 구현
		*/
		//== 클라이언트에 전송할 파일을 파싱하여 저장 ==//
		char szCopyData[MAX_BUFF] = { 0 };
		char szFileFormat[8] = { 0 };
		char szRunFile[32] = { 0 };
		char szProtocol[32] = { 0 };
		char szStatus[32] = { 0 };
		char* szTokenData = NULL;

		// 정상요청일 때 파일이름을 구한다
		strcpy(szCopyData, szRecv); //szCopyData에다가 szRecv(Client의 요청 메시지를 저장하는 변수) 복사 
		szTokenData = strtok(szCopyData, " "); // GET
		szTokenData = strtok(NULL, " ");		// /index.html (형식)
		if(szTokenData) // Token 후 데이터가 NULL이 아닌지 항상 확인
		{
			sprintf(szRunFile, "%s", szTokenData);
		}

		if (strlen(szRunFile) <= 1) // 파일이 생략되거나 없을 경우 default로 index.html을 지정한다.
		{
			strcpy(szRunFile, "/index.html"); // /index.html로 문자열 복사
		}

		//== 서버가 파일을 가지고 있으면 response message를 만들어 전송 ==//
		// 날짜 정보를 만든다
		char tempBuffer[32] = { 0 };
		char szDate[64] = "Date: ";
		time_t timer = time(NULL); // 현재 시간 
		struct tm* t = localtime(&timer);
		strftime(tempBuffer, 32, "%c", t);
		strcat(szDate, tempBuffer); // 날짜 출력 
		
		// 이전 토큰 파일을 이용해 다음 파싱 값을 구한다.
		if (szTokenData) // Token 후 데이터가 NULL이 아닌지 항상 확인
		{
			szTokenData = strtok(NULL, " \r\n"); // HTTP/1.1
			if(szTokenData) // Token 후 데이터가 NULL이 아닌지 항상 확인
			{
				strcpy(szProtocol, szTokenData); // szProtocol에다가 szTokenData 복사 
			}
		}
		strcpy(szStatus, szProtocol); // szStatus에다가 szProtocol 복사 
		strcat(szStatus, " 200 OK\r\n"); //szStatus에 " 200 OK"을 붙여넣음

		// 실행파일을 파싱해서 확장자(파일형식)를 구한다.
		strcpy(szCopyData, szRunFile); //szCopyData에다가 szRunFile 복사 
		szTokenData = strtok(szCopyData, "."); // /index
		szTokenData = strtok(NULL, "."); // html	
		
		if (szTokenData) // token한 결과값이 있으면 확장자를 복사한다.
		{
			strcpy(szFileFormat, szTokenData); // szFileFormat에다가 szTokenData 복사 
		}
		else // 확장자가 없을 때 처리
		{
			strcpy(szFileFormat, ""); // 빈 파일 형식으로 저장한다.
		}

		// content type을 만든다.
		char szContentType[64] = "\nContent-Type: ";
		if (strcmp(szFileFormat, "html") == 0 || strcmp(szFileFormat, "htm") == 0) // szFileFormat(파일형식)이 html혹은 htm이면 
		{
			strcat(szContentType, "text/html"); // szContentType에 "text/html"을 붙여넣음 (Content-Type: text/html) 
		}
		else if (strcmp(szFileFormat, "gif") == 0) // szFileFormat(파일형식)이 gif이면 
		{
			strcat(szContentType, "image/gif"); // szContentType에 "image/gif"을 붙여넣음 (Content-Type: image/gif)
		}
		else if (strcmp(szFileFormat, "jpeg") == 0 || strcmp(szFileFormat, "jpg") == 0) // szFileFormat(파일형식)이 jpeg혹은 jpg이면 
		{
			strcat(szContentType, "image/jpeg"); // szContentType에 "image/jpeg"을 붙여넣음 (Content-Type: image/jpeg)
		}
		else if (strcmp(szFileFormat, "mp3") == 0) // szFileFormat(파일형식)이 mp3이면 
		{
			strcat(szContentType, "audio/mp3"); // szContentType에 "audio/mp3"을 붙여넣음 (Content-Type: audio/mp3)
		}
		else if (strcmp(szFileFormat, "pdf") == 0) // szFileFormat(파일형식)이 pdf이면 
		{
			strcat(szContentType, "application/pdf"); // szContentType에 "application/pdf"을 붙여넣음 (Content-Type: application/pdf)
		}
		else if (strcmp(szFileFormat, "ico") == 0) // szFileFormat(파일형식)이 ico이면 
		{
			strcat(szContentType, "image/x-icon"); // szContentType에 "image/x-icon"을 붙여넣음 (Content-Type: image/x-icon)
		}
		strcat(szContentType,"; charset=utf-8\r\n");  // 문자 인코딩 방법을 추가
		
		// Header 만들기
		char* szResponseHeader = NULL; //데이터 저장 
		int nHeaderSize = strlen(szStatus) + strlen(szDate) + strlen(szContentType) + strlen("\r\n"); // szResponseHeader의 크기값을 구한다 
		szResponseHeader = (char*)malloc(sizeof(char) * nHeaderSize + 1); // 동적할당 
		bzero(szResponseHeader, sizeof(char) * nHeaderSize); // 응답헤더 0으로 초기화 
		strcat(szResponseHeader, szStatus); // szResponseHeader(응답헤더)에 szStatus을 붙여넣음 
		strcat(szResponseHeader, szDate); // szResponseHeader(응답헤더)에 szDate을 붙여넣음 
		strcat(szResponseHeader, szContentType); // szResponseHeader(응답헤더)에 szContentType을 붙여넣음 
		strcat(szResponseHeader, "\r\n"); // szResponseHeader(응답헤더)에 "\r\n"을 붙여넣음 

		// 요청 파일이 없으면 404 not found 메시지 작성
		char szErrorStatus[32] = { 0 };
		char* szErrorHeader = NULL; // 데이터 저장 
		int nErrorSize = 0;
		strcpy(szErrorStatus, szProtocol); // szErrorStatus에 szProtocol을 복사함 
		strcat(szErrorStatus, " 404 Not Found\r\n"); // szErrorStatus에 "404 Not Found"를 붙여넣음 
		nErrorSize = strlen(szErrorStatus) + strlen(szDate) + strlen("\r\n"); //에러 메시지 사이즈 
		szErrorHeader = (char*)malloc(sizeof(char) * nErrorSize + 1); // 동적할당 
		bzero(szErrorHeader, sizeof(char) * nErrorSize); // 0으로 초기화 
		strcpy(szErrorHeader, szErrorStatus); // szErrorHeader에다가 szErrorStatus 복사 
		strcat(szErrorHeader, szDate); // szErrorHeader에 szDate을 붙여넣음 
		strcat(szErrorHeader, "\r\n");

		// 파일 찾기
		int nExistFile = 0;
		char szPath[1024] = { 0 };
		getcwd(szPath, 1024); // 현재 작업중인 디렉토리 주소를 찾아온다.
		strcat(szPath, szRunFile); // szPath에 szRunFile 붙여넣음 

		// 파일 데이터를 읽어온다.
		char* bFileData = NULL;
		int nFileSize = 0;
		int file_fd = open(szPath, O_RDONLY); //읽기 전용으로 파일 열기 
		if (file_fd > 0) // 파일이 있음
		{
			// 200 성공 헤더를 전송하고, 파일 데이터를 전송
			int n = write(newsockfd, szResponseHeader, nHeaderSize);
			if (n <= 0)
			{
				printf("error Send Response Header Message, n(%d)\n", n);
			}

			// 요청 파일 전송
			char bFileData[524288] = { 0 }; // 지역변수는 전형적으로 윈도우는 1MB 리눅스는 8MB, 512K로 설정해서 보낸다.
			while ( (n = read(file_fd, bFileData, 524288)) > 0) // 파일을 512K씩 읽어와서 내용이 있다면
			{
				n = write(newsockfd, bFileData, n); //bFileData가 가리키는 메모리 영역에서 n바이트만큼 읽어 파일에 쓰기 
				if (n <= 0)
				{
					printf("error Send File Data, n(%d)\n", n);
				}
				bzero(bFileData, sizeof(bFileData)); // bFileData 0으로 초기화 
			}
			// 다 쓴 파일 데이터를 정리하고, 파일을 닫는다.
			close(file_fd);
		}
		else // 파일이 없음
		{
			// 404 에러 메시지가 저장된 헤더를 전송
			n = write(newsockfd, szErrorHeader, nErrorSize);
		}
		free(szResponseHeader); // 동적할당한 메모리 해제 
		free(szErrorHeader); // 동적할당한 메모리 해제 
		// 전송 실패
		if (n < 0)
		{
			error("ERROR writing to socket");
		}

		// 소켓 초기화
		close(newsockfd);
	}
	close(sockfd);

     
	 // 종료
     return 0; 
}
