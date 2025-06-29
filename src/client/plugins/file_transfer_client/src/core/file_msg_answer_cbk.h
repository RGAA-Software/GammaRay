#pragma once

#include "tc_message_new/msg_answer_cbk.h"

namespace tc {
	class FileMsgAnswerCbkStructure : public MsgAnswerCallbackStructure {
	public:
		void Add(const std::shared_ptr<tc::Message>& msg, OnMsgParseRespCallbackFuncType parse_msg_callbck) override;
	};
}