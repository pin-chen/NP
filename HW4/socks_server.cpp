#include <cstdlib>
#include <iostream>
#include <fstream>
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
#define uc unsigned char
#define ui unsigned int

using boost::asio::ip::tcp;
using namespace std;

struct socks4Info{
	int vn;
	int cd;
	string srcIP;
	string srcPort;
	string dstIP;
	string dstPort;
	string cmd;
	string reply;
};

class session : public std::enable_shared_from_this<session>{
	public:
		session(tcp::socket socket, boost::asio::io_context& io_context) : socket1_(std::move(socket)), socket2_(io_context), io_context_(io_context){}

		void start(){
			do_read_request();
		}

	private:
		void do_read_request(){
			auto self(shared_from_this());
			memset(data_, '\0', sizeof(data_));
			socket1_.async_read_some(
				boost::asio::buffer(data_, max_length), 
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						parse(int(length));
						firewall();
						cout << "<S_IP>: " << request.srcIP << '\n' << flush;
						cout << "<S_PORT>: " << request.srcPort << '\n' << flush;
						cout << "<D_IP>: " << request.dstIP << '\n' << flush;
						cout << "<D_PORT>: " << request.dstPort << '\n' << flush;
						cout << "<Command>: " << request.cmd << '\n' << flush;
						cout << "<Reply>: " << request.reply << '\n' << flush;
						reply[0] = 0;
						if(request.reply == "Accept"){
							reply[1] = 90;
							if(request.cd == 1){
								do_connect();
							}else{
								do_bind();
							}
						}else{
							reply[1] = 91;
							do_write_reply();
							exit(0);
						}
					}
				}
			);
		}
		
		void do_write_reply(){
			auto self(shared_from_this());
			boost::asio::async_write(
				socket1_, boost::asio::buffer(reply, 8),
				[this, self](boost::system::error_code ec, size_t length){
					if(!ec){
						
					}
				}
			);
		}
		
		void do_connect(){
			auto self(shared_from_this());
			tcp::resolver resolver_(io_context_);
			tcp::resolver::results_type endpoint_ = resolver_.resolve(request.dstIP, request.dstPort);
			boost::asio::async_connect(
				socket2_, endpoint_,
				[this, self](boost::system::error_code ec, tcp::endpoint ed){
					if(!ec){
						do_write_reply();
						do_read1_data();
						do_read2_data();
					}else{
						reply[1] = 91;
						do_write_reply();
						socket1_.close();
						socket2_.close();
					}
				}
			);
		}
		
		void do_bind(){
			tcp::acceptor acceptor_(io_context_, tcp::endpoint(tcp::v4(), 0));
			acceptor_.listen();
			ui port = acceptor_.local_endpoint().port();
			//cerr << "**************\n";
			//cerr << "port: " << port << '\n' << flush;
			reply[2] = (port >> 8) & 0x000000FF;
			reply[3] = port & 0x000000FF;
			do_write_reply();
			acceptor_.accept(socket2_);
			do_write_reply();
			do_read1_data();
			do_read2_data();
		}
		
		void do_read1_data(){
			auto self(shared_from_this());
			memset(data_, '\0', sizeof(data_));
			socket1_.async_read_some(
				boost::asio::buffer(data_, max_length), 
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						do_write2_data(length);
					}else{
						socket1_.close();
						socket2_.close();
						exit(0);
					}
				}
			);
		}
		
		void do_read2_data(){
			auto self(shared_from_this());
			memset(data_, '\0', sizeof(data_));
			socket2_.async_read_some(
				boost::asio::buffer(data_, max_length), 
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						do_write1_data(length);
					}else{
						socket1_.close();
						socket2_.close();
						exit(0);
					}
				}
			);
		}
		
		void do_write1_data(size_t length){
			auto self(shared_from_this());
			boost::asio::async_write(
				socket1_, boost::asio::buffer(data_, length),
				[this, self](boost::system::error_code ec, size_t length){
					if(!ec){
						do_read2_data();
					}
				}
			);
		}
		
		void do_write2_data(size_t length){
			auto self(shared_from_this());
			boost::asio::async_write(
				socket2_, boost::asio::buffer(data_, length),
				[this, self](boost::system::error_code ec, size_t length){
					if(!ec){
						do_read1_data();
					}
				}
			);
		}
		
		void firewall(){
			if(request.reply != "Firewall") return;
			request.reply = "Reject";
			ifstream in("./socks.conf");
			if(!in.is_open()) return;
			string rule = "";
			while(getline(in, rule)){
				vector<string> tmp;
				splitArgument(tmp, rule, ' ');
				if(tmp[0] != "permit") continue;
				if(tmp[1] == "c" && request.cd == 2) continue;
				if(tmp[1] == "b" && request.cd == 1) continue;
				vector<string> ipX;
				vector<string> ipY;
				splitArgument(ipX, tmp[2], '.');
				splitArgument(ipY, request.dstIP, '.');
				bool permit = true;
				for(int i = 0; i < 4; i++){
					if(ipX[i] == "*") continue;
					if(ipX[i] != ipY[i]){
						permit = false;
						break;
					}
				}
				if(permit){
					request.reply = "Accept";
					break;
				}
			}
		}
		
		void splitArgument(vector<string> &dst, string &src, char c){
			dst.clear();
			string arg;
			ui cur = 0, charType = 0;
			for(; cur < src.size(); cur++){
				if(src[cur] == c){
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
		
		void parse(int length){
			request.reply = "Firewall";
			for(int i = 2; i < 8; i++) reply[i] = 0;//reply[i] = data_[i];
			if(length < 9){
				request.reply = "Reject";
				return;
			} //cerr << "request error\n";
			request.vn = data_[0];
			if(request.vn != 4) request.reply = "Reject";
			request.cd = data_[1];
			request.cmd = request.cd == 1 ? "CONNECT" : "BIND";
			request.dstPort = to_string((ui) (data_[2] << 8) | data_[3]);
			if(data_[4] == 0 && data_[5] == 0 && data_[6] == 0 && data_[7] != 0){
				int index = 8;
				while(data_[index] != 0) index++;
				index++;
				string domain = "";
				while(data_[index] != 0) domain.push_back(data_[index++]);
				tcp::resolver resolver_(io_context_);
				tcp::endpoint endpoint_ = resolver_.resolve(domain, request.dstPort)->endpoint();
				request.dstIP = endpoint_.address().to_string();
			}else{
				char ip[20];
				snprintf(ip, 20, "%d.%d.%d.%d", data_[4], data_[5], data_[6], data_[7]);
				request.dstIP = string(ip);
			}
			request.srcIP = socket1_.remote_endpoint().address().to_string();
			request.srcPort = to_string(socket1_.remote_endpoint().port());
		}
		
		tcp::socket socket1_;
		tcp::socket socket2_;
		boost::asio::io_context& io_context_;
		struct socks4Info request;
		uc reply[8];
		enum { max_length = 10240 };
		uc data_[max_length];
};

class server{
	public:
		server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), io_context_(io_context), signal_(io_context, SIGCHLD){
			acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
			handler();
			do_accept();
		}

	private:
	
		void handler(){
			signal_.async_wait(
				[this](boost::system::error_code ec, int signo){
					if(acceptor_.is_open()){
						int state;
						while(waitpid(-1, &state, WNOHANG) > 0);
						handler();
					}
				}
			);
		}
		
		void do_accept(){
			acceptor_.async_accept(
				[this](boost::system::error_code ec, tcp::socket socket){
					if (!ec){
						io_context_.notify_fork(boost::asio::io_context::fork_prepare);
						pid_t pid;
again_fork:
						pid = fork();
						if(pid < 0){
							usleep(1000);
							goto again_fork;
						}else if(pid == 0){
							io_context_.notify_fork(boost::asio::io_context::fork_child);
							acceptor_.close();
							signal_.cancel();
							std::make_shared<session>(std::move(socket), io_context_)->start();
						}else{
							io_context_.notify_fork(boost::asio::io_context::fork_parent);
							socket.close();
						}
					}
					do_accept();
				}
			);
		}
		tcp::acceptor acceptor_;
		boost::asio::io_context& io_context_;
		boost::asio::signal_set signal_;
};

int main(int argc, char* argv[]){
	try{
		if (argc != 2){
			std::cerr << "Usage: ./socks_server <port>\n";
			return 1;
		}
		boost::asio::io_context io_context;
		server s(io_context, std::atoi(argv[1]));
		io_context.run();
	}catch (std::exception& e){
		//std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}