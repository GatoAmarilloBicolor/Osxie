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

#ifndef _OSXIESERVER_CALL_HPP_
#define _OSXIESERVER_CALL_HPP_

#include <osxieserver/rpc.h>
#include <osxieserver/rpc.internal.h>

#include <osxieserver/message.hpp>
#include <osxieserver/registry.hpp>
#include <osxieserver/logging.hpp>

#include <memory>

#include <unistd.h>

namespace OsxieServer {
	class CallWithReply;

	class Call {
	public:
		enum class Number: unsigned int {
			Invalid = dserver_callnum_invalid,
			DSERVER_ENUM_VALUES
		};

		static inline const char* callNumberToString(Number number) {
			return dserver_callnum_to_string(static_cast<dserver_callnum_t>(number));
		};

	protected:
		std::weak_ptr<Thread> _thread;
		Address _replyAddress;
		dserver_rpc_callhdr_t _header;

		static OsxieServer::Log rpcReplyLog;

		static void sendReply(Message&& reply);

	public:
		Call(std::shared_ptr<Thread> thread, Address replyAddress, dserver_rpc_callhdr_t* callHeader);
		virtual ~Call();

		static std::shared_ptr<Call> callFromMessage(Message&& requestMessage);

		virtual Number number() const = 0;
		std::shared_ptr<Thread> thread() const;

		virtual void processCall() = 0;

		virtual void sendBasicReply(int resultCode);
		// FIXME: this should actually be in a "BSD" subclass with BSD traps inheriting from it
		virtual void sendBSDReply(int resultCode, uint32_t returnValue);

		virtual bool isXNUTrap() const;
		virtual bool isBSDTrap() const;

		DSERVER_CLASS_DECLS;
	};

	DSERVER_CLASS_DEFS;
};

#endif // _OSXIESERVER_CALL_HPP_
