#include <osxieserver/utility.hpp>
#include <unistd.h>

OsxieServer::FD::FD():
	_fd(-1)
	{};

OsxieServer::FD::FD(int fd):
	_fd(fd)
	{};

OsxieServer::FD::~FD() {
	if (_fd != -1) {
		close(_fd);
	}
};

OsxieServer::FD::FD(FD&& other):
	_fd(other._fd)
{
	other._fd = -1;
};

OsxieServer::FD& OsxieServer::FD::operator=(FD&& other) {
	if (_fd != -1) {
		close(_fd);
	}
	_fd = other._fd;
	other._fd = -1;
	return *this;
};

int OsxieServer::FD::fd() const {
	return _fd;
};

int OsxieServer::FD::extract() {
	auto fd = _fd;
	_fd = -1;
	return fd;
};

OsxieServer::FD::operator bool() {
	return _fd != -1;
};
