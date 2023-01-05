#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <map>
#define f first
#define s second

using boost::asio::ip::tcp;
using namespace std;

class session : public std::enable_shared_from_this<session>{
	public:
		session(tcp::socket socket) : socket_(std::move(socket)){}

		void start(){
			do_read();
		}

	private:
		void do_read(){
			auto self(shared_from_this());
			socket_.async_read_some(
				boost::asio::buffer(data_, max_length), 
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						pid_t pid;
again_fork:
						while(waitpid(-1, &state, WNOHANG) > 0);
						pid = fork();
						if(pid < 0){
							usleep(1000);
							goto again_fork;
						}else if(pid == 0){
							data_[length] = '\0';
//							cout << '\n' << data_ << '\n' << flush;
							//env
							fillEnv(string(data_));
							memset(data_, '\0', sizeof(data_));
							setEnv();
							//dup2
							dup2Client(socket_.native_handle());
							//close
							socket_.close();
							if(env["REQUEST_URI"] != "/panel.cgi"){
								cout << "HTTP/1.1 403 Forbiden\r\n" << flush;
								exit(0);
							}
							//exec
							cout << "HTTP/1.1 200 OK\r\n" << flush;
							cout << "Server: http_server\r\n" << flush;
							//cout << "Transfer-Encoding: chunked\r\n" << flush;
							path = "." + path;
							char **argv = stoc({path});
							int e = execv(path.c_str(), argv);
							if(e == -1){
								exit(1);
							}
							exit(0);
						}else{
							socket_.close();
							while(waitpid(-1, &state, WNOHANG) > 0);
						}
					}
				}
			);
		}
		
		void dup2Client(int fd){
			dup2(fd, 0);
			dup2(fd, 1);
			dup2(fd, 2);
		}
		
		void cleanEnv(){
			env["REQUEST_METHOD"] = "";
			env["REQUEST_URI"] = "";
			env["QUERY_STRING"] = "";
			env["SERVER_PROTOCOL"] = "";
			env["HTTP_HOST"] = "";
			env["SERVER_ADDR"] = "";
			env["SERVER_PORT"] = "";
			env["REMOTE_ADDR"] = "";
			env["REMOTE_PORT"] = "";
		}

		void setEnv(){
			for(auto e : env){
				setenv(e.f.c_str(), e.s.c_str(), 1);
			}
		}

		void splitArgument(vector<string> &dst, string &src){
			dst.clear();
			string arg;
			unsigned int cur = 0, charType = 0;
			for(; cur < src.size(); cur++){
				if(src[cur] == ' '){
					if(charType == 1){
						dst.push_back(arg);
						arg.clear();
						charType = 0;
					}
				}else if(src[cur] == '\r'){
					continue;
				}else{
					if(charType == 0) charType = 1;
					arg.push_back(src[cur]);
				}
			}
			if(arg.size() != 0){
				dst.push_back(arg);
			}
		}

		void splitline(vector<vector<string>> &dst, string src){
			dst.clear();
			vector<string> tmp;
			unsigned int begin = 0, cur = 0;
			for(; cur < src.size(); cur++){
				if(src[cur] == '\n'){
					tmp.push_back(src.substr(begin, cur - begin));
					begin = cur + 1;
				}
			}
			dst.resize(tmp.size());
			int i = 0;
			for(string cmd : tmp){
				splitArgument(dst[i], cmd);
				i++;
			}
		}

		char **stoc(vector<string> src){ //vector<string> to char[][]
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

		void fillEnv(string src){
			vector<vector<string>> cmd;
			splitline(cmd, src);
			if(cmd.size() < 2){
				cerr << "cmd.size() < 2\n";
				return;
			}
			env["REQUEST_METHOD"] = cmd[0][0];
			env["REQUEST_URI"] = cmd[0][1];
			int i = cmd[0][1].find('?');
			if(i == -1){
				env["QUERY_STRING"] = "";
				path = cmd[0][1];
			}else{
				env["QUERY_STRING"] = cmd[0][1].substr(i + 1);
				path = cmd[0][1].substr(0, i);
			}
			env["SERVER_PROTOCOL"] = cmd[0][2];
			env["HTTP_HOST"] = cmd[1][1];
			env["SERVER_ADDR"] = socket_.local_endpoint().address().to_string();
			env["SERVER_PORT"] = to_string(socket_.local_endpoint().port());
			env["REMOTE_ADDR"] = socket_.remote_endpoint().address().to_string();
			env["REMOTE_PORT"] = to_string(socket_.remote_endpoint().port());
		}
		
		tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];
		map<string, string> env;
		string path;
		int state;
};

class server{
	public:
		server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)){
			do_accept();
		}

	private:
		void do_accept(){
			acceptor_.async_accept(
				[this](boost::system::error_code ec, tcp::socket socket){
					if (!ec){
						std::make_shared<session>(std::move(socket))->start();
					}
					do_accept();
				}
			);
		}
		tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]){
	try{
		if (argc != 2){
			std::cerr << "Usage: async_tcp_echo_server <port>\n";
			return 1;
		}
		boost::asio::io_context io_context;
		server s(io_context, std::atoi(argv[1]));
		io_context.run();
	}catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}