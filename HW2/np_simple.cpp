#include<iostream>
#include<cstring>
#include<sstream>
#include<string>
#include<vector>
#include<cctype>
#include<map>
#include<pwd.h>
#include<errno.h>
#include<fcntl.h>
#include<netdb.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<strings.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define MAX_COUNT 1001
#define cmdTime first
#define processState second
#define f firest
#define s second

#define SA struct sockaddr
#define	MAXLINE	15000
using namespace std;
struct processState{
	int numberedPipe[2] = {-1, -1};
	vector<pid_t> pids;
};

map<int, processState> numberedPipes;
int state, counter;
auto NPIN = STDIN_FILENO;
auto NPOUT = STDOUT_FILENO;
auto NPERR = STDERR_FILENO;

void handler(int signo){
	while(waitpid(-1, &state, WNOHANG) > 0);
}

void splitOrdinaryPipe(vector<string> &dst, string &src){
	dst.clear();
	int begin = 0, cur = 0;
	for(; cur < src.size(); cur++){
		if(src[cur] == '|'){
			dst.push_back(src.substr(begin, cur - begin));
			begin = cur + 1;
		}else if(src[cur] == '>'){
			if(cur + 1 < src.size() && src[cur + 1] == '>'){
				dst.push_back(src.substr(begin, cur - begin));
				dst.push_back(">>" + src.substr(cur + 1));
				return;
			}
			dst.push_back(src.substr(begin, cur - begin));
			dst.push_back(">" + src.substr(cur + 1));
			return;
		}
	}
	if(begin != src.size()) dst.push_back(src.substr(begin));
}

void splitArgument(vector<string> &dst, string &src){
	dst.clear();
	string arg;
	int begin = 0, charType = 0; //0 otherwise 1 space 2 " 3 '
	if(src[0] == '>'){
		if(src.size() >= 2 && src[1] == '>'){
			dst.push_back(">>");
			begin = 2;
		}else{
			dst.push_back(">");
			begin = 1;

		}
	}
	for(; begin < src.size(); begin++){
		if(src[begin] == ' ' && charType != 2){
			if(charType == 1){
				dst.push_back(arg);
				arg.clear();
				charType = 0;
			}
		}else if(src[begin] == '"'){
			if(charType != 2){
				charType = 2;
			}else if(charType == 2){
				charType = 1;
			}
		}else if(src[begin] == '\''){
			if(charType != 3){
				charType = 3;
			}else if(charType == 3){
				charType = 1;
			}
		}else{
			if(charType == 0) charType = 1;
			arg.push_back(src[begin]);
		}
	}
	if(arg.size() != 0){
		dst.push_back(arg);
	} 
}

void splitline(vector<vector<string>> &dst, string &src){
	dst.clear();
	vector<string> tmp;
	splitOrdinaryPipe(tmp, src);
	dst.resize(tmp.size());
	int i = 0;
	for(string cmd: tmp){
		splitArgument(dst[i], cmd);
		i++;
	}
}

char **stoc(vector<string> &src){ //vector<string> to char[][]
	int argc = src.size();
	char **argv = new char*[argc+1]; 
	for(int i = 0; i < argc; i++){
		argv[i] = new char[src[i].size()];
		strcpy(argv[i], src[i].c_str());
	}
	argv[argc] = new char;
	argv[argc] = NULL;
	return argv;
}

int buildInCommand(vector<vector<string>> &cmd){
	if(cmd[0][0] == "exit"){
		while(waitpid(-1, NULL, WNOHANG) > 0);
		return -1;
	}else if(cmd[0][0] == "printenv"){
		if(cmd[0].size() < 2 || getenv(cmd[0][1].c_str()) == NULL);
		else cout << getenv(cmd[0][1].c_str()) << '\n';
		return 1;
	}else if(cmd[0][0] == "setenv"){
		if(cmd[0].size() < 2);
		else if(cmd[0].size() < 3) 	setenv(cmd[0][1].c_str(), "", 1);
		else 						setenv(cmd[0][1].c_str(), cmd[0][2].c_str(), 1);
		return 1;
	}
	return 0;
}

void execute(vector<vector<string>> &cmd, int pipeTime){
	int numOrdinaryPipe = cmd.size() - 1, fd = -1;
	//file redirection check
	bool fileRedirection = cmd[numOrdinaryPipe][0] == ">";
	bool demo = cmd[numOrdinaryPipe][0] == ">>";
	if(fileRedirection || demo) numOrdinaryPipe--;
	//memory for pipe
	int ordinaryPipe[numOrdinaryPipe * 2];
	memset(ordinaryPipe, -1, sizeof(ordinaryPipe));
	pid_t pid;
	for(int i = 0; i < numOrdinaryPipe + 1; i++){
		//construct pipe
		if(i < numOrdinaryPipe) pipe(ordinaryPipe + i * 2);
		if(cmd[i].size() <= 0){
			cerr << "Unknown command: [].\n";
			continue;
		}
		while(waitpid(-1, &state, WNOHANG) > 0);
		//fork
		pid = fork();
		if(pid < 0){ //fork error
			i--;
			usleep(1000);
			continue;
		}else if(pid == 0){ //child
			//dup2
			if(i != 0)
				dup2(ordinaryPipe[(i - 1) * 2 ], STDIN_FILENO);
			else if(numberedPipes.find(counter) != numberedPipes.end())
				dup2(numberedPipes[counter].numberedPipe[0], STDIN_FILENO);
			if(i != numOrdinaryPipe){
				dup2(ordinaryPipe[i * 2 + 1], STDOUT_FILENO);
			}else if(fileRedirection){
				fd = open(cmd[numOrdinaryPipe + 1][1].c_str(), O_TRUNC | O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
				dup2(fd, STDOUT_FILENO);
			}else if(demo){
				fd = open(cmd[numOrdinaryPipe + 1][2].c_str(), O_RDWR | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
				dup2(fd, STDOUT_FILENO);
			}else{
				dup2(NPOUT, STDOUT_FILENO);
				dup2(NPERR, STDERR_FILENO);
			}
			//close
			for(int j = 0; j < (i + 1) * 2 && j < numOrdinaryPipe * 2; j++) close(ordinaryPipe[j]);
			for(auto process : numberedPipes){
				close(process.processState.numberedPipe[0]);
				close(process.processState.numberedPipe[1]);
			} 
			//exec
			char **argv = stoc(cmd[i]);
			int e = execvp(argv[0], argv);
			if(e == -1){
				cerr << "Unknown command: [" << &cmd[i][0][0] << "].\n";
				exit(1);
			}
			exit(0);
		}else{ //parent
			while(waitpid(-1, &state, WNOHANG) > 0);
			//close ordinary pipe
			if(i != 0){
				close(ordinaryPipe[(i - 1) * 2]);
				close(ordinaryPipe[(i - 1) * 2 + 1]);
			}
			//close and wait numbered pipe
			if(numberedPipes.find(counter) != numberedPipes.end()){
				close(numberedPipes[counter].numberedPipe[0]);
				close(numberedPipes[counter].numberedPipe[1]);
				for(auto pid : numberedPipes[counter].pids) waitpid(pid, &state, 0);
				numberedPipes.erase(counter);
			}
		}
	}
	//wait child or load pid
	if(pipeTime == -1) 	waitpid(pid, &state, 0);
	else 				numberedPipes[pipeTime].pids.push_back(pid);
	while(waitpid(-1, &state, WNOHANG) > 0);
	//close fd
	if(fileRedirection || demo) close(fd);
	//init fd
	NPOUT = STDOUT_FILENO;
	NPERR = STDERR_FILENO;
}

void dealNumberedPipes(string &line){ //split, execute, load pipe and pid
	int begin = 0, num = 0, pipeTime, cur ,newCur;
	for(cur = 0; cur < line.size(); cur++){
		if(line[cur] == '|' || line[cur] == '!'){
			for(newCur = cur + 1; newCur < line.size() && isdigit(line[newCur]); newCur++){
				num *= 10;
				num += (int) line[newCur] - '0';
			}
			if(num == 0) continue;
			counter = (counter + 1) % MAX_COUNT;
			pipeTime = ((counter + num) % MAX_COUNT);
			if(numberedPipes.find(pipeTime) == numberedPipes.end()){
				pipe(numberedPipes[pipeTime].numberedPipe);
			}
			NPOUT = numberedPipes[pipeTime].numberedPipe[1];
			if(line[cur] == '!') NPERR = numberedPipes[pipeTime].numberedPipe[1];
			vector<vector<string>> cmd;
			string tline = line.substr(begin, cur - begin);
			splitline(cmd, tline);
			execute(cmd, pipeTime);
			begin = newCur;
			cur = newCur - 1;
			num = 0;
		}
	}
	line = line.substr(begin);
}

void init(){
	clearenv();
	setenv("PATH", "bin:.", 1);
	counter = 0;
	numberedPipes.clear();
}

void server(int port){
	//socket
	int connectfd, listenfd, val = 1, len = sizeof(val), n;
	struct sockaddr_in serverAddress, clientAddress;
	char buf[MAXLINE];
	socklen_t clientLen;
	//loop
	string in;
	vector<vector<string>> cmd;
	//socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1){
		perror("socket");
		exit(1);
	}
	//serverAddress
	bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
	//setsockopt
	n = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &val, len);
	if(n == -1){
		perror("setsockopt(REUSEADDR)");
		exit(1);
	}
	n = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &val, len);
	if(n == -1){
		perror("setsockopt(REUSEPORT)");
		exit(1);
	}
	//bind
	n = bind(listenfd, (SA*) &serverAddress, sizeof(serverAddress));
	if(n == -1){
		perror("bind");
		exit(1);
	}
	//listen
	n = listen(listenfd, 3);
	if(n == -1){
		perror("listen");
		exit(1);
	}
	signal(SIGCHLD, handler);
	for(;;){
		//backup STD
		dup2(0, 4);
		dup2(1, 5);
		dup2(2, 6);
		//connect
		connectfd = accept(listenfd, (SA*)&clientAddress, &clientLen);
		if(connectfd == -1){
			perror("accept");
			exit(1);
		}
		//
		in.clear();
		//dup2
		dup2(connectfd, 0);
		dup2(connectfd, 1);
		dup2(connectfd, 2);
		init();
		cout << "% ";
		while(n = read(0, buf, MAXLINE)){
			if(n == -1){
				perror("read");
				continue;
			}else if(n == 0){
				break;
			}else{
				int e = 0;
				for(int i = 0; i < n; i++){
					if(buf[i] == '\r') continue;
					if(buf[i] != '\n'){
						in.push_back(buf[i]);
						continue;
					}
					string line = in;
					in.clear();
					dealNumberedPipes(line);
					splitline(cmd, line);
					if(cmd.size() <= 0){
						cout << "% ";
						continue;
					}
					counter = (counter + 1) % MAX_COUNT;
					e = buildInCommand(cmd);
					if(e == 1){
						cout << "% ";
						continue;
					}else if(e == -1){
						break;
					}
					execute(cmd, -1);
					cout << "% ";
				}
				if(e == -1) break;
			}
		}
		//STD callback
		dup2(4, 0);
		dup2(5, 1);
		dup2(6, 2);
		//close
		n = close(connectfd);
		if(n == -1){
			perror("close");
		}
	}
}

int main(int argc, char**argv){
	std::cout.setf(std::ios::unitbuf);
	int port = 11111;
	if(argc > 1){
		sscanf(argv[1], "%d", &port);
	}
	if(port < 1024 || port > 65535){
		cerr << "Error port!";
		exit(1);
	}
	server(port);
	return 0;
}