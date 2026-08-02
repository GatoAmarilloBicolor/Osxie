/**
 * This file is part of Osxie.
 *
 * Copyright (C) 2021 Osxie developers
 *
 * Osxie is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Osxie is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <osxieserver/message.hpp>
#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include <new>
#include <mutex>
#include <system_error>

OsxieServer::Address::Address() {
	_address.sun_family = AF_UNIX;
	memset(_address.sun_path, 0, sizeof(_address.sun_path));
	_size = sizeof(_address);
};

OsxieServer::Address::Address(const struct sockaddr_un& rawAddress, size_t addressLength) {
	_address = rawAddress;
	_size = addressLength;
};

struct sockaddr_un& OsxieServer::Address::raw() {
	return _address;
};

const struct sockaddr_un& OsxieServer::Address::raw() const {
	return _address;
};

size_t OsxieServer::Address::rawSize() const {
	return _size;
};

void OsxieServer::Address::setRawSize(size_t newRawSize) {
	_size = newRawSize;
};

OsxieServer::Message::Message(size_t bufferSpace, size_t descriptorSpace, std::function<void()> sendNotificationCallback):
	_sendNotificationCallback(sendNotificationCallback)
{
	size_t controlLen = CMSG_SPACE(sizeof(struct ucred)) + (descriptorSpace > 0 ? CMSG_SPACE(sizeof(int) * descriptorSpace) : 0);
	_controlHeader = static_cast<decltype(_controlHeader)>(malloc(controlLen));

	if (!_controlHeader) {
		throw std::bad_alloc();
	}

	_controlHeader->cmsg_len = CMSG_LEN(sizeof(struct ucred));
	_controlHeader->cmsg_level = SOL_SOCKET;
	_controlHeader->cmsg_type = SCM_CREDENTIALS;
	struct ucred ourCreds;
	ourCreds.pid = getpid();
	ourCreds.uid = getuid();
	ourCreds.gid = getgid();
	// the cmsg man page says memcpy should be used with CMSG_DATA instead of direct pointer access
	memcpy(CMSG_DATA(_controlHeader), &ourCreds, sizeof(ourCreds));

	if (descriptorSpace > 0) {
		auto fdHeader = reinterpret_cast<decltype(_controlHeader)>(reinterpret_cast<char*>(_controlHeader) + CMSG_SPACE(sizeof(struct ucred)));
		fdHeader->cmsg_len = CMSG_LEN(sizeof(int) * descriptorSpace);
		fdHeader->cmsg_level = SOL_SOCKET;
		fdHeader->cmsg_type = SCM_RIGHTS;
		for (size_t i = 0; i < descriptorSpace; ++i) {
			const int fd = -1;
			memcpy(CMSG_DATA(fdHeader) + (sizeof(int) * i), &fd, sizeof(int));
		}
	}

	_buffer.resize(bufferSpace);

	_header.msg_name = &_socketAddress.raw();
	_header.msg_namelen = _socketAddress.rawSize();
	_header.msg_iov = &_dataDescriptor;
	_header.msg_iovlen = 1;
	_header.msg_control = _controlHeader;
	_header.msg_controllen = controlLen;
	_header.msg_flags = 0;

	_dataDescriptor.iov_base = _buffer.data();
	_dataDescriptor.iov_len = _buffer.size();
};

void OsxieServer::Message::_initWithOther(Message&& other) {
	_sendNotificationCallback = std::move(other._sendNotificationCallback);
	other._sendNotificationCallback = nullptr;

	_buffer = std::move(other._buffer);
	_socketAddress = std::move(other._socketAddress);
	_controlHeader = std::move(other._controlHeader);

	other._controlHeader = nullptr;

	_header = std::move(other._header);
	_header.msg_name = &_socketAddress.raw();
	_header.msg_iov = &_dataDescriptor;

	_dataDescriptor.iov_base = _buffer.data();
	_dataDescriptor.iov_len = _buffer.size();
};

void OsxieServer::Message::_cleanupSelf() {
	if (_controlHeader) {
		auto fdHeader = _descriptorHeader();
		if (fdHeader) {
			for (size_t i = 0; i < _descriptorSpace(); ++i) {
				int fd = -1;
				memcpy(&fd, CMSG_DATA(fdHeader) + (sizeof(int) * i), sizeof(int));
				if (fd != -1) {
					close(fd);
				}
			}
		}
		free(_controlHeader);
		_controlHeader = nullptr;
	}
};

struct cmsghdr* OsxieServer::Message::_credentialsHeader() {
	return const_cast<struct cmsghdr*>(const_cast<const Message*>(this)->_credentialsHeader());
};

const struct cmsghdr* OsxieServer::Message::_credentialsHeader() const {
	const struct cmsghdr* hdr = CMSG_FIRSTHDR(&_header);

	while (hdr) {
		if (hdr->cmsg_level == SOL_SOCKET && hdr->cmsg_type == SCM_CREDENTIALS) {
			return hdr;
		}

		hdr = CMSG_NXTHDR(const_cast<struct msghdr*>(&_header), const_cast<struct cmsghdr*>(hdr));
	}

	return nullptr;
};

struct cmsghdr* OsxieServer::Message::_descriptorHeader() {
	return const_cast<struct cmsghdr*>(const_cast<const Message*>(this)->_descriptorHeader());
};

const struct cmsghdr* OsxieServer::Message::_descriptorHeader() const {
	const struct cmsghdr* hdr = CMSG_FIRSTHDR(&_header);

	while (hdr) {
		if (hdr->cmsg_level == SOL_SOCKET && hdr->cmsg_type == SCM_RIGHTS) {
			return hdr;
		}

		hdr = CMSG_NXTHDR(const_cast<struct msghdr*>(&_header), const_cast<struct cmsghdr*>(hdr));
	}

	return nullptr;
};

size_t OsxieServer::Message::_descriptorSpace() const {
	auto hdr = _descriptorHeader();

	if (!hdr) {
		return 0;
	}

	return (hdr->cmsg_len - (reinterpret_cast<uintptr_t>(CMSG_DATA(hdr)) - reinterpret_cast<uintptr_t>(hdr))) / sizeof(int);
};

OsxieServer::Message::~Message() {
	_cleanupSelf();
};

OsxieServer::Message::Message(Message&& other) {
	_initWithOther(std::move(other));
};

OsxieServer::Message& OsxieServer::Message::operator=(Message&& other) {
	_cleanupSelf();
	_initWithOther(std::move(other));
	return *this;
};

struct msghdr& OsxieServer::Message::rawHeader() {
	return _header;
};

const struct msghdr& OsxieServer::Message::rawHeader() const {
	return _header;
};

std::vector<uint8_t>& OsxieServer::Message::data() {
	return _buffer;
};

const std::vector<uint8_t>& OsxieServer::Message::data() const {
	return _buffer;
};

OsxieServer::Address OsxieServer::Message::address() const {
	return _socketAddress;
};

void OsxieServer::Message::setAddress(Address address) {
	_socketAddress = address;
	_header.msg_namelen = _socketAddress.rawSize();
};

std::vector<int> OsxieServer::Message::descriptors() const {
	std::vector<int> descriptors;
	auto fdHeader = _descriptorHeader();

	if (fdHeader) {
		for (size_t i = 0; i < _descriptorSpace(); ++i) {
			int fd = -1;
			memcpy(&fd, CMSG_DATA(fdHeader) + (sizeof(int) * i), sizeof(int));
			if (fd != -1) {
				descriptors.push_back(fd);
			}
		}
	}

	return descriptors;
};

void OsxieServer::Message::pushDescriptor(int descriptor) {
	if (auto fdHeader = _descriptorHeader()) {
		for (size_t i = 0; i < _descriptorSpace(); ++i) {
			int fd = -1;
			memcpy(&fd, CMSG_DATA(fdHeader) + (sizeof(int) * i), sizeof(int));
			if (fd != -1) {
				continue;
			}

			memcpy(CMSG_DATA(fdHeader) + (sizeof(int) * i), &descriptor, sizeof(int));
			return;
		}
	}

	auto oldSpace = _descriptorSpace();
	_ensureDescriptorHeader(oldSpace + 1);
	memcpy(CMSG_DATA(_descriptorHeader()) + (sizeof(int) * oldSpace), &descriptor, sizeof(int));
};

int OsxieServer::Message::extractDescriptor(int descriptor) {
	if (auto fdHeader = _descriptorHeader()) {
		for (size_t i = 0; i < _descriptorSpace(); ++i) {
			int fd = -1;
			memcpy(&fd, CMSG_DATA(fdHeader) + (sizeof(int) * i), sizeof(int));
			if (fd != descriptor) {
				continue;
			}

			fd = -1;
			memcpy(CMSG_DATA(fdHeader) + (sizeof(int) * i), &fd, sizeof(int));
			return descriptor;
		}
	}

	return -1;
};

int OsxieServer::Message::extractDescriptorAtIndex(size_t index) {
	if (auto fdHeader = _descriptorHeader()) {
		for (size_t i = 0, presentIndex = 0; i < _descriptorSpace(); ++i) {
			int fd = -1;
			memcpy(&fd, CMSG_DATA(fdHeader) + (sizeof(int) * i), sizeof(int));
			if (fd == -1) {
				continue;
			}

			if (presentIndex++ != index) {
				continue;
			}

			int tmp = -1;
			memcpy(CMSG_DATA(fdHeader) + (sizeof(int) * i), &tmp, sizeof(int));
			return fd;
		}
	}

	return -1;
};

void OsxieServer::Message::replaceDescriptors(const std::vector<int>& newDescriptors) {
	_ensureDescriptorHeader(newDescriptors.size());
	memcpy(CMSG_DATA(_descriptorHeader()), newDescriptors.data(), sizeof(int) * newDescriptors.size());
};

bool OsxieServer::Message::copyCredentialsOut(struct ucred& outputCredentials) const {
	auto credHeader = _credentialsHeader();

	if (credHeader) {
		memcpy(&outputCredentials, CMSG_DATA(credHeader), sizeof(outputCredentials));
		return true;
	} else {
		return false;
	}
};

void OsxieServer::Message::copyCredentialsIn(const struct ucred& inputCredentials) {
	_ensureCredentialsHeader();
	memcpy(CMSG_DATA(_credentialsHeader()), &inputCredentials, sizeof(inputCredentials));
};

void OsxieServer::Message::_ensureCredentialsHeader() {
	if (_credentialsHeader()) {
		return;
	}

	auto fdHeader = _descriptorHeader();
	auto descSpace = _descriptorSpace();
	size_t controlLen = CMSG_SPACE(sizeof(struct ucred)) + (descSpace > 0 ? CMSG_SPACE(sizeof(int) * descSpace) : 0);
	auto tmp = static_cast<decltype(_controlHeader)>(malloc(controlLen));
	auto old = _controlHeader;
	struct cmsghdr* credHeader = nullptr;

	if (!tmp) {
		throw std::bad_alloc();
	}

	_controlHeader = tmp;
	_header.msg_control = _controlHeader;

	credHeader = _controlHeader;

	if (descSpace > 0) {
		auto newFdHeader = reinterpret_cast<decltype(_controlHeader)>(reinterpret_cast<char*>(_controlHeader) + CMSG_SPACE(sizeof(struct ucred)));
		memcpy(newFdHeader, fdHeader, CMSG_SPACE(sizeof(int) * descSpace));
	}

	free(old);

	_header.msg_controllen = controlLen;

	credHeader->cmsg_len = CMSG_LEN(sizeof(struct ucred));
	credHeader->cmsg_level = SOL_SOCKET;
	credHeader->cmsg_type = SCM_CREDENTIALS;

	struct ucred creds;
	creds.pid = getpid();
	creds.uid = getuid();
	creds.gid = getgid();
	memcpy(CMSG_DATA(credHeader), &creds, sizeof(struct ucred));
};

void OsxieServer::Message::_ensureDescriptorHeader(size_t newSpace) {
	if (_descriptorSpace() == newSpace) {
		return;
	}

	auto credHeader = _credentialsHeader();
	size_t oldSpace = _descriptorSpace();
	size_t controlLen = (credHeader ? CMSG_SPACE(sizeof(struct ucred)) : 0) + (newSpace == 0 ? 0 : CMSG_SPACE(sizeof(int) * newSpace));

	if (controlLen == 0) {
		free(_controlHeader);
		_controlHeader = nullptr;
		_header.msg_control = nullptr;
		_header.msg_controllen = 0;
		return;
	}

	auto tmp = static_cast<decltype(_controlHeader)>(malloc(controlLen));
	auto old = _controlHeader;
	struct cmsghdr* fdHeader = nullptr;

	if (!tmp) {
		throw std::bad_alloc();
	}

	_controlHeader = tmp;
	_header.msg_control = _controlHeader;

	if (credHeader) {
		memcpy(_controlHeader, credHeader, CMSG_SPACE(sizeof(struct ucred)));
		fdHeader = reinterpret_cast<decltype(_controlHeader)>(reinterpret_cast<char*>(_controlHeader) + CMSG_SPACE(sizeof(struct ucred)));
	} else {
		fdHeader = _controlHeader;
	}

	free(old);

	_header.msg_controllen = controlLen;

	if (newSpace > 0) {
		fdHeader->cmsg_len = CMSG_LEN(sizeof(int) * newSpace);
		fdHeader->cmsg_level = SOL_SOCKET;
		fdHeader->cmsg_type = SCM_RIGHTS;

		for (size_t i = oldSpace; i < newSpace; ++i) {
			int fd = -1;
			memcpy(CMSG_DATA(fdHeader) + (sizeof(int) * i), &fd, sizeof(int));
		}
	}
};

pid_t OsxieServer::Message::pid() const {
	struct ucred creds;

	if (copyCredentialsOut(creds)) {
		return creds.pid;
	} else {
		return -1;
	}
};

void OsxieServer::Message::setPID(pid_t pid) {
	_ensureCredentialsHeader();
	struct ucred creds;
	copyCredentialsOut(creds);
	creds.pid = pid;
	copyCredentialsIn(creds);
};

uid_t OsxieServer::Message::uid() const {
	struct ucred creds;

	if (copyCredentialsOut(creds)) {
		return creds.uid;
	} else {
		return -1;
	}
};

void OsxieServer::Message::setUID(uid_t uid) {
	_ensureCredentialsHeader();
	struct ucred creds;
	copyCredentialsOut(creds);
	creds.uid = uid;
	copyCredentialsIn(creds);
};

gid_t OsxieServer::Message::gid() const {
	struct ucred creds;

	if (copyCredentialsOut(creds)) {
		return creds.gid;
	} else {
		return -1;
	}
};

void OsxieServer::Message::setGID(gid_t gid) {
	_ensureCredentialsHeader();
	struct ucred creds;
	copyCredentialsOut(creds);
	creds.gid = gid;
	copyCredentialsIn(creds);
};

std::function<void()> OsxieServer::Message::sendNotificationCallback() const {
	return _sendNotificationCallback;
};

void OsxieServer::Message::setSendNotificationCallback(std::function<void()> sendNotificationCallback) {
	_sendNotificationCallback = sendNotificationCallback;
};

//
// message queue
//

void OsxieServer::MessageQueue::push(Message&& message) {
	std::unique_lock lock(_lock);
	_messages.push_back(std::move(message));

	if (_messages.size() > 0) {
		auto callback = _messageArrivalNotificationCallback;
		lock.unlock();
		if (callback) {
			callback();
		}
	}
};

std::optional<OsxieServer::Message> OsxieServer::MessageQueue::pop() {
	std::scoped_lock lock(_lock);
	if (_messages.size() > 0) {
		Message message = std::move(_messages.front());
		_messages.pop_front();
		return message;
	} else {
		return std::nullopt;
	}
};

bool OsxieServer::MessageQueue::sendMany(int socket) {
	bool canSendMore = true;
	std::scoped_lock lock(_lock);
	struct mmsghdr mmsgs[16];
	size_t len = 0;
	int ret = 0;

	while (ret >= 0) {
		len = 0;
		for (size_t i = 0; i < _messages.size() && i < sizeof(mmsgs) / sizeof(*mmsgs); ++i) {
			mmsgs[i].msg_hdr = _messages[i].rawHeader();
			mmsgs[i].msg_len = 0;

			// some systems report EINVAL when sending to an address filled with null bytes.
			auto address = reinterpret_cast<sockaddr_un*>(mmsgs[i].msg_hdr.msg_name);
			bool isAddressEmpty = true;
			for (size_t j = 0; j < sizeof(address->sun_path); ++j) {
				if (address->sun_path[j] != '\0') {
					isAddressEmpty = false;
					break;
				}
			}
			if (isAddressEmpty) {
				mmsgs[i].msg_hdr.msg_name = nullptr;
				mmsgs[i].msg_hdr.msg_namelen = 0;
			}

			++len;
		}

		if (len == 0) {
			break;
		}

		ret = sendmmsg(socket, mmsgs, len, MSG_DONTWAIT);

		if (ret < 0) {
			if (errno == EAGAIN) {
				canSendMore = false;
				break;
			} else if (errno == EINTR) {
				ret = 0;
			} else if (errno == EPIPE || errno == ECONNREFUSED) {
				// this means that peer of the first message we tried to send has died.
				// we'll just drop that message and try the others.
				_messages.pop_front();
				ret = 0;
			} else {
				throw std::system_error(errno, std::generic_category(), "Failed to send messages through socket");
			}
		}

		for (size_t i = 0; i < ret; ++i) {
			auto cb = _messages.front().sendNotificationCallback();
			_messages.pop_front();
			if (cb) {
				cb();
			}
		}
	}

	return canSendMore;
};

bool OsxieServer::MessageQueue::receiveMany(int socket) {
	bool canReadMore = true;
	std::unique_lock lock(_lock);
	struct mmsghdr mmsgs[16];
	int ret = 0;

	while (ret >= 0) {
		std::array<Message, sizeof(mmsgs) / sizeof(*mmsgs)> messages;

		for (size_t i = 0; i < messages.size(); ++i) {
			mmsgs[i].msg_hdr = messages[i].rawHeader();
			mmsgs[i].msg_len = 0;
		}

		ret = recvmmsg(socket, mmsgs, sizeof(mmsgs) / sizeof(*mmsgs), MSG_CMSG_CLOEXEC | MSG_DONTWAIT, nullptr);

		if (ret < 0) {
			if (errno == EAGAIN) {
				canReadMore = false;
				break;
			} else if (errno == EINTR) {
				ret = 0;
			} else {
				throw std::system_error(errno, std::generic_category(), "Failed to receive messages through socket");
			}
		}

		for (size_t i = 0; i < ret; ++i) {
			messages[i].rawHeader() = mmsgs[i].msg_hdr;
			messages[i].data().resize(mmsgs[i].msg_len);
			messages[i].setAddress(Address(*(const struct sockaddr_un*)mmsgs[i].msg_hdr.msg_name, mmsgs[i].msg_hdr.msg_namelen));
			_messages.push_back(std::move(messages[i]));
		}
	}

	if (_messages.size() > 0) {
		auto callback = _messageArrivalNotificationCallback;
		lock.unlock();
		if (callback) {
			callback();
		}
	}

	return canReadMore;
};

void OsxieServer::MessageQueue::setMessageArrivalNotificationCallback(std::function<void()> messageArrivalNotificationCallback) {
	std::unique_lock lock(_lock);
	_messageArrivalNotificationCallback = messageArrivalNotificationCallback;
};

bool OsxieServer::MessageQueue::empty() const {
	std::unique_lock lock(_lock);
	return _messages.empty();
};
