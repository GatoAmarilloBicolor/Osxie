#ifndef _OSXIESERVER_UTILITY_HPP_
#define _OSXIESERVER_UTILITY_HPP_

namespace DarlingServer {
	/**
	 * A RAII wrapper for POSIX file descriptors.
	 */
	class FD {
	private:
		int _fd;
	public:
		FD();
		FD(int fd);
		~FD();

		FD(const FD&) = delete;
		FD& operator=(const FD&) = delete;

		FD(FD&& other);
		FD& operator=(FD&& other);

		int fd() const;
		int extract();

		explicit operator bool();
	};
};

#endif // _OSXIESERVER_UTILITY_HPP_
