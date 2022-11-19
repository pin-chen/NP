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
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/mman.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#define MAX_COUNT 1001
#define cmdTime first
#define processState second
#define f first
#define s second

#define SA struct sockaddr
#define	MAXLINE	15000

using namespace std;

struct processState{
	int numberedPipe[2] = {-1, -1};
	vector<pid_t> pids;
};

struct usersInfo{
	bool alive;
	pid_t pid;
	char ip4[INET_ADDRSTRLEN];
	int port;
	char name[20];
};

struct userPipe{
	int fd[31];
	pid_t pid[31];
};

int state, counter, shmUsersData, shmMsg, shmUserPipe, ME;
char *msg;
auto NPIN = STDIN_FILENO;
auto NPOUT = STDOUT_FILENO;
auto NPERR = STDERR_FILENO;
map<int, processState> numberedPipes;
userPipe *userPipes;
usersInfo *usersData;

void handler(int signo){
	while(waitpid(-1, &state, WNOHANG) > 0);
}

void terminate(int signo){
	if(signo != SIGINT) return;
	munmap(msg, 150000);
	munmap(usersData, 31 * sizeof(struct usersInfo));
	munmap(userPipes, 31 * sizeof(struct userPipe));
	exit(0);
}

void sendMsg(int signo){
	if(signo != SIGUSR1) return;
	cout << msg;
}

void openFifo(int singo){
	if(singo != SIGUSR2) return;
	char fifoName[30];
	for(int i = 1; i < 31; i++){
		sprintf(fifoName, "user_pipe/%d-%d", i, ME);
		if(userPipes[i].fd[ME] == -2){
			userPipes[i].fd[ME] = open(fifoName, O_RDONLY | O_NONBLOCK);
		}
	}
}

void dup2Client(int fd){
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
}

void broadcast(string out) {
	memset(msg, '\0' ,sizeof(msg));
	strcpy(msg, out.c_str());
	for(int i = 1; i < 31; i++){
		if(usersData[i].alive) kill(usersData[i].pid, SIGUSR1);
	}
	usleep(1000);
}

void privmsg(int i, string out) {
	memset(msg, '\0' ,sizeof(msg));
	strcpy(msg, out.c_str());
	if(usersData[i].alive) kill(usersData[i].pid, SIGUSR1);
}

void splitOrdinaryPipe(vector<string> &dst, string &src){
	dst.clear();
	int begin = 0, cur = 0;
	for(; cur < src.size(); cur++){
		if(src[cur] == '|'){
			dst.push_back(src.substr(begin, cur - begin));
			begin = cur + 1;
		}else if(src[cur] == '>'){
			if(cur + 1 < src.size()){
				if(src[cur + 1] == '>'){
					dst.push_back(src.substr(begin, cur - begin));
					dst.push_back(">>" + src.substr(cur + 1));
					return;
				}else if(src[cur + 1] != ' '){
					continue;
				}
			}
			dst.push_back(src.substr(begin, cur - begin));
			dst.push_back("> " + src.substr(cur + 1));
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
		}else if(src.size() >= 2 && src[1] == ' '){
			dst.push_back(">");
			begin = 1;
		}
	}
	for(; begin < src.size(); begin++){
		if(src[begin] == ' '){
			if(charType == 1){
				dst.push_back(arg);
				arg.clear();
				charType = 0;
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

int PJ2buildInCommand(string line, int me){
	int space1 = 0;
	for(; space1 < line.size() && line[space1] != ' '; space1++);
	string cmd = line.substr(0, space1);
	if(cmd == "tell"){
		int n = space1;
		int num = 0;
		for(; n < line.size() && line[n] == ' '; n++);
		for(; n < line.size() && isdigit(line[n]); n++){
			num *= 10;
			num += (int) line[n] - '0';
		}
		if(!usersData[num].alive){
			cerr << "*** Error: user #" << num << " does not exist yet. ***" << '\n';
			return 1;
		}
		privmsg(num, "*** " + string(usersData[me].name) + " told you ***: " + line.substr(n + 1) + "\n"); 
		return 1;
	}else if(cmd == "yell"){
		broadcast("*** " + string(usersData[me].name) + " yelled ***: " + line.substr(space1+1) + "\n");
		return 1;
	}else if(cmd == "who"){
		cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n";
		for(int i = 1; i < 31; i++){
			if(!usersData[i].alive) continue;
			cout << i << '\t' << usersData[i].name << '\t' << usersData[i].ip4 << ':' << usersData[i].port;
			if(i == me){
				cout << '\t' << "<-me";
			}
			cout << '\n';
		}
		return 1;
	}else if(cmd == "name"){
		int n = space1;
		for(; n < line.size() && line[n] == ' '; n++);
		int space2 = n;
		for(; space2 < line.size() && line[space2] != ' '; space2++);
		string name = line.substr(n, space2 - n);
		for(int i = 1; i < 31; i++){
			if(name == string(usersData[i].name)){
				cerr << "*** User '" << name << "' already exists. ***" << '\n';
				return 1;
			}
		}
		strcpy(usersData[me].name, name.c_str());
		broadcast("*** User from " + string(usersData[me].ip4) + ":" + to_string(usersData[me].port) + " is named '" + name + "'. ***\n");
		return 1;
	}
	return 0;
}

void execute(vector<vector<string>> &cmd, int pipeTime, string &fullcmd, int me){
	int numOrdinaryPipe = cmd.size() - 1, fd = -1;
	int idOfUserPipeIn = 0, idOfUserPipeOut = 0;
	bool userIN = false, userOUT = false;
	//file redirection check
	bool fileRedirection = cmd[numOrdinaryPipe][0] == ">";
	bool demo = cmd[numOrdinaryPipe][0] == ">>";
	if(fileRedirection || demo) numOrdinaryPipe--;
	//memory for pipe
	int ordinaryPipe[numOrdinaryPipe * 2];
	memset(ordinaryPipe, -1, sizeof(ordinaryPipe));
	pid_t pid;
	for(int i = 0; i < numOrdinaryPipe + 1; i++){
		//
		bool IN = false, OUT = false, userPipeExsits = false;
		//user pipr
		int argc = cmd[i].size();
		if(argc > 1){
			if(cmd[i][argc - 1][0] == '>'){
				sscanf(cmd[i][argc - 1].substr(1).c_str(), "%d", &idOfUserPipeOut);
				OUT = true;
			}else if(cmd[i][argc - 1][0] == '<'){
				sscanf(cmd[i][argc - 1].substr(1).c_str(), "%d", &idOfUserPipeIn);
				IN = true;
			}
			if(argc > 2){
				if(cmd[i][argc - 2][0] == '>'){
					sscanf(cmd[i][argc - 2].substr(1).c_str(), "%d", &idOfUserPipeOut);
					OUT = true;
				}else if(cmd[i][argc - 2][0] == '<'){
					sscanf(cmd[i][argc - 2].substr(1).c_str(), "%d", &idOfUserPipeIn);
					IN = true;
				}
			}
		}
		if(IN && idOfUserPipeIn != 0){
			cmd[i].pop_back();
			if(!usersData[idOfUserPipeIn].alive){
				cerr << "*** Error: user #" << idOfUserPipeIn << " does not exist yet. ***" << '\n';
			}else if(userPipes[idOfUserPipeIn].fd[me] == -1){
				cerr << "*** Error: the pipe #" << idOfUserPipeIn << "->#" << me << " does not exist yet. ***" << '\n';
			}else{
				broadcast( "*** " + string(usersData[me].name) + " (#" + to_string(me) + ") just received from " + string(usersData[idOfUserPipeIn].name) + " (#" + to_string(idOfUserPipeIn) + ") by '" + fullcmd + "' ***\n");
				
				userIN = true;
			}
		}
		if(OUT && idOfUserPipeOut != 0){
			cmd[i].pop_back();
			if(!usersData[idOfUserPipeOut].alive){
				cerr << "*** Error: user #" << idOfUserPipeOut << " does not exist yet. ***" << '\n';
			}else if(userPipes[me].fd[idOfUserPipeOut] != -1){
				cerr << "*** Error: the pipe #" << me << "->#" << idOfUserPipeOut << " already exists. ***" << '\n';
				userPipeExsits = true;
			}else{
				broadcast("*** " + string(usersData[me].name) + " (#" + to_string(me) + ") just piped '" + fullcmd + "' to " + string(usersData[idOfUserPipeOut].name) + " (#" + to_string(idOfUserPipeOut) + ") ***\n");				
				
				userOUT = true;
			}
		}
again_fork:
		//construct pipe
		if(i < numOrdinaryPipe) pipe(ordinaryPipe + i * 2);
		while(waitpid(-1, &state, WNOHANG) > 0);
		//fork
		pid = fork();
		if(pid < 0){ //fork error
			usleep(1000);
			goto again_fork;
		}else if(pid == 0){ //child
			//user pipe
			bool A = false, B = false;
			if(IN && idOfUserPipeIn != 0){
				if(!usersData[idOfUserPipeIn].alive){
					dup2(open("/dev/null", O_RDONLY, 0), STDIN_FILENO);
				}else if(userPipes[idOfUserPipeIn].fd[me] == -1){
					dup2(open("/dev/null", O_RDONLY, 0), STDIN_FILENO);
				}else{
					A = true;
					int x = dup2(userPipes[idOfUserPipeIn].fd[me], STDIN_FILENO);
					if(x == -1){
						dup2(open("/dev/null", O_RDONLY, 0), STDIN_FILENO);
					}
					userPipes[idOfUserPipeIn].fd[me] = -1;
				}
			}
			if(OUT && idOfUserPipeOut != 0){
				if(!usersData[idOfUserPipeOut].alive){
					dup2(open("/dev/null", O_RDWR, 0), STDOUT_FILENO);
				}else if(userPipeExsits){
					dup2(open("/dev/null", O_RDWR, 0), STDOUT_FILENO);
				}else{
					B = true;
					char fifoName[30];
					sprintf(fifoName, "user_pipe/%d-%d", me, idOfUserPipeOut);
					mkfifo(fifoName, 0666);
					userPipes[me].fd[idOfUserPipeOut] = -2;
					kill(usersData[idOfUserPipeOut].pid, SIGUSR2);
					int ffd = open(fifoName, O_WRONLY);
					if(ffd == -1){
						dup2(open("/dev/null", O_RDWR, 0), STDOUT_FILENO);
					}else{
						dup2(ffd, STDOUT_FILENO);
						close(ffd);
					}
					
				}
			}
			//dup2
			if(!A){
				if(i != 0)
					dup2(ordinaryPipe[(i - 1) * 2 ], STDIN_FILENO);
				else if(numberedPipes.find(counter) != numberedPipes.end())
					dup2(numberedPipes[counter].numberedPipe[0], STDIN_FILENO);
			}
			if(!B){
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
			}
			//close
			for(int j = 1; j < 31; j++){
				for(int k = 1; k < 31; k++){
					if(userPipes[j].fd[k] != -1){
						close(userPipes[j].fd[k]);
					}
				}
			}
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
			if(userIN){
				waitpid(userPipes[idOfUserPipeIn].pid[me], &state, 0);
				userPipes[idOfUserPipeIn].pid[me] = 0;
				close(userPipes[idOfUserPipeIn].fd[me]);
				char fifoName[30];
				sprintf(fifoName, "user_pipe/%d-%d", idOfUserPipeIn, me);
				unlink(fifoName);
				
			}
		}
	}
	//wait child or load pid
	if(userOUT) 				userPipes[me].pid[idOfUserPipeOut] = pid;
	else if(pipeTime == -1) 	waitpid(pid, &state, 0);
	else 						numberedPipes[pipeTime].pids.push_back(pid);
	while(waitpid(-1, &state, WNOHANG) > 0);
	//close fd
	if(fileRedirection || demo) close(fd);
	//init fd
	NPOUT = STDOUT_FILENO;
	NPERR = STDERR_FILENO;
}

void dealNumberedPipes(string &line, int me){ //split, execute, load pipe and pid
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
			execute(cmd, pipeTime, line, me);
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
	//
	signal(SIGUSR1, sendMsg);
	signal(SIGUSR2, openFifo);
	//socket
	int connectfd, listenfd, val = 1, len = sizeof(val), n;
	struct sockaddr_in serverAddress, clientAddress;
	char buf[MAXLINE];
	socklen_t clientLen = sizeof(clientAddress);
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
	//share memory
	shmUsersData = shm_open("cps:UsersData", O_CREAT | O_RDWR, 0666);
	ftruncate(shmUsersData, 31 * sizeof(struct usersInfo));
	usersData = (usersInfo *)mmap(NULL, 31 * sizeof(struct usersInfo), PROT_READ | PROT_WRITE, MAP_SHARED, shmUsersData, 0);
	shmMsg = shm_open("cps:MSG", O_CREAT | O_RDWR, 0666);
	ftruncate(shmMsg, 15000);
	msg = (char *)mmap(NULL, 15000, PROT_READ | PROT_WRITE, MAP_SHARED, shmMsg, 0);
	shmUserPipe = shm_open("cps:UserPipe", O_CREAT | O_RDWR, 0666);
	ftruncate(shmUserPipe, 31 * sizeof(struct userPipe));
	userPipes = (userPipe *)mmap(NULL, 31 * sizeof(struct userPipe), PROT_READ | PROT_WRITE, MAP_SHARED, shmUserPipe, 0);
	for(int i = 0; i < 31; i++){
		for(int j = 0; j < 31; j++){
			userPipes[i].fd[j] = -1;
			userPipes[i].pid[j] = 0;
		}
	}
	for(int i = 0; i < 31; i++){
		usersData[i].alive = false;
		usersData[i].pid = 0;
		usersData[i].port = 0;
		memset(usersData[i].ip4, '\0', sizeof(usersData[i].ip4));
		memset(usersData[i].name, '\0', sizeof(usersData[i].name));
		strcpy(usersData[i].name, "(no name)");
	}
	memset(msg, '\0', sizeof(msg));
	//child signal
	signal(SIGCHLD, handler);
	for(;;){
		connectfd = accept(listenfd, (SA *) &clientAddress, &clientLen);
		if(connectfd == -1){
			perror("accept");
			continue;
		}
		int index = 1;
		while(usersData[index].alive) index++;
		pid_t pid;
again_fork:
		pid = fork();
		if(pid < 0){
			usleep(1000);
			goto again_fork;
		}else if(pid == 0){
			ME = index;
			close(listenfd);
			dup2Client(connectfd);
			close(connectfd);
			init();
			cout << "****************************************\n";
			cout << "** Welcome to the information server. **\n";
			cout << "****************************************\n";
			while(!usersData[index].alive) usleep(1000);
			broadcast("*** User '" + string(usersData[index].name) + "' entered from " + string(usersData[index].ip4) + ":" + to_string(usersData[index].port) + ". ***\n");
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
						e = PJ2buildInCommand(line, index);
						if(e == 1){
							cout << "% ";
							continue;
						}
						dealNumberedPipes(line, index);
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
						execute(cmd, -1, line, index);
						cout << "% ";
					}
					if(e == -1) break;
				}
			}
			char fifoName[30];
			for(int i = 1; i < 31; i++){
				close(userPipes[index].fd[i]);
				close(userPipes[i].fd[index]);
				sprintf(fifoName, "user_pipe/%d to %d", index, i);
				unlink(fifoName);
				sprintf(fifoName, "user_pipe/%d to %d", i, index);
				unlink(fifoName);
				userPipes[index].fd[i] = -1;
				userPipes[i].fd[index] = -1;
				userPipes[index].pid[i] = 0;
				userPipes[i].pid[index] = 0;
			}
			usersData[index].alive = false;
			broadcast("*** User '" + string(usersData[index].name) + "' left. ***\n");
			usersData[index].pid = 0;
			usersData[index].port = 0;
			memset(usersData[index].ip4, '\0', sizeof(usersData[index].ip4));
			memset(usersData[index].name, '\0', sizeof(usersData[index].name));
			strcpy(usersData[index].name, "(no name)");
			close(0);
			close(1);
			close(2);
			exit(0);
		}else{
			close(connectfd);
			usersData[index].pid = pid;
			inet_ntop(AF_INET, &(clientAddress.sin_addr), usersData[index].ip4, INET_ADDRSTRLEN);
			usersData[index].port = ntohs(clientAddress.sin_port);
			strcpy(usersData[index].name, "(no name)");
			usersData[index].alive = true;
		}
	}
}

int main(int argc, char**argv){
	signal(SIGINT, terminate);
	std::cout.setf(std::ios::unitbuf);
	std::cerr.setf(std::ios::unitbuf);
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