#include <iostream>
#include <fstream>
#include <thread>

#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>

//#include <netdb.h>
//#include <cstdlib>

#define BACKLOG_SIZE 5
#define BUFFER_SIZE 256

void error(const char* msg) {
	std::cerr << msg << std::endl;
	exit(1);
}


std::string read_filename(int client_fd) {
	std::string filename;

	short filename_len = 0;
	if (read(client_fd, &filename_len, sizeof(filename_len)) <= 0)
		return filename;

	if (filename_len == 0)
		return filename;

	char fname[filename_len+1];
	bzero(fname, filename_len+1);
	if (read(client_fd, fname, filename_len) <= 0)
		return filename;

	filename = std::string(fname);
	return filename;
}


bool file_exists(const std::string &filename) {
	return access(filename.c_str(), 0) == 0;
}


void process_client(int client_fd, std::string path) {

	std::string filename = read_filename(client_fd);
//	filename.insert(0, std::to_string(rand()));
	std::cout << "download: " << filename << std::endl;

	try {
		if (filename.empty())
			throw "empty filename";
		if (file_exists(filename))
			throw "file already exists";

		std::ofstream fout(filename, std::ios::out | std::ios::binary);

		char buffer[BUFFER_SIZE];
		while(true) {
			bzero(buffer, BUFFER_SIZE);
			int read_len = read(client_fd, buffer, BUFFER_SIZE-1);
			if (read_len <= 0)
				break;

			fout.write(buffer, read_len);
			std::this_thread::yield();
		}
		fout.close();
		std::cout << "downloading '" << filename << "' is finished" << std::endl;
	} catch (const char* msg) {
		std::cerr << msg << std::endl;
	}
	close(client_fd);
}


std::string get_path() {
	std::string path;
	while (true) {
		std::cout << "enter path: ";
		std::cin >> path;

		struct stat info;
		if (stat(path.c_str(), &info) != 0) {
			std::cout << "this path doesnt exist: " << path << std::endl;
		} else if (!S_ISDIR(info.st_mode)) {
			std::cout << "path: " << path << " - is not a directory" << std::endl;
		} else {
			break;
		}
	}
	return path;
}


int main(int argc, char* argv[]) {

	if (argc < 2) {
		error("port is not set\nusage: ./storage port_num");
	}
	int port_num = atoi(argv[1]);

	std::string path = get_path();

	//
	int listen_sockfd;
	struct sockaddr_in serv_addr;

	listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sockfd < 0) {
		error("cant create socket");
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_num);

	if (bind(listen_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		error("cant bind socket");
	}

	if (listen(listen_sockfd, BACKLOG_SIZE) != 0) {
		error("cant listen socket");
	}

	std::cout << "storage started" << std::endl;

	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	while (true) {
		int client_fd = accept(listen_sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
		if (client_fd < 0) {
			continue;
		}

//		std::cout << "client connected" << std::endl;

		std::thread client_thread(process_client, client_fd, path);
		client_thread.detach();
		std::this_thread::yield();
	}

	close(listen_sockfd);

	return 0;
}
