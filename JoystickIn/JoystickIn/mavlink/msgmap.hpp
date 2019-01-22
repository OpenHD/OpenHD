
#pragma once

#include <endian.h>
#include <type_traits>

namespace mavlink {

/**
 * Serialization helper wrapper for mavlink_message_t
 */
class MsgMap {
public:

	explicit MsgMap(mavlink_message_t *p) :
		msg(p), cmsg(p), pos(0)
	{ }

	explicit MsgMap(mavlink_message_t &p) :
		msg(&p), cmsg(&p), pos(0)
	{ }

	explicit MsgMap(const mavlink_message_t *p) :
		msg(nullptr), cmsg(p), pos(0)
	{ }

	inline void reset()
	{
		pos = 0;
	}

	inline void reset(uint32_t msgid, uint8_t len)
	{
		assert(msg);

		msg->msgid = msgid;	// necessary for finalize
		msg->len = len;		// needed only for deserialization w/o finalize
		pos = 0;
	}

	template<typename _T>
	void operator<< (const _T data);

	template<class _T, size_t _Size>
	void operator<< (const std::array<_T, _Size> &data);

	template<typename _T>
	void operator>> (_T &data);

	template<class _T, size_t _Size>
	void operator>> (std::array<_T, _Size> &data);

private:
	mavlink_message_t *msg;		// for serialization
	const mavlink_message_t *cmsg;	// for deserialization
	size_t pos;

	template<typename _T, typename _Tin>
	inline void msg_swap_memcpy(_T &buf, _Tin data);

	template<typename _T, typename _Tout>
	inline void cmsg_memcpy_bzero_swap_set_data(_T &buf, _Tout &data);
};

} // namespace mavlink

// implementation

template<typename _T, typename _Tin>
void mavlink::MsgMap::msg_swap_memcpy(_T &buf, _Tin data)
{
	if (std::is_floating_point<_Tin>::value) {
		buf = *static_cast<const _T *>(static_cast<const void *>(&data));
	} else {
		buf = data;
	}

	// htoleXX functions may be empty macros,
	// switch will be optimized-out
	switch (sizeof(_T)) {
	case 2:
		buf = htole16(buf);
		break;

	case 4:
		buf = htole32(buf);
		break;

	case 8:
		buf = htole64(buf);
		break;

	default:
		assert(false);
	}

	memcpy(&_MAV_PAYLOAD_NON_CONST(msg)[pos], &buf, sizeof(buf));
}

template<typename _T>
void mavlink::MsgMap::operator<< (const _T data)
{
	assert(msg);
	assert(pos + sizeof(_T) <= MAVLINK_MAX_PAYLOAD_LEN);

	switch (sizeof(_T)) {
	case 1:
		_MAV_PAYLOAD_NON_CONST(msg)[pos] = data;
		break;

	case 2:
		uint16_t data_le16;
		msg_swap_memcpy(data_le16, data);
		break;

	case 4:
		uint32_t data_le32;
		msg_swap_memcpy(data_le32, data);
		break;

	case 8:
		uint64_t data_le64;
		msg_swap_memcpy(data_le64, data);
		break;

	default:
		assert(false);
	}

	//std::cout << "M< s: " << sizeof(_T) << " p: " << pos << " d: " << data << std::endl;

	pos += sizeof(_T);
}

template<class _T, size_t _Size>
void mavlink::MsgMap::operator<< (const std::array<_T, _Size> &data)
{
	for (auto &v : data) {
		*this << v;
	}
}

template<typename _T, typename _Tout>
void mavlink::MsgMap::cmsg_memcpy_bzero_swap_set_data(_T &buf, _Tout &data)
{
	memcpy(&buf, &_MAV_PAYLOAD(cmsg)[pos], sizeof(_T));

	// if message is trimmed - bzero tail
	if (pos + sizeof(_T) > cmsg->len) {
		union {
			_T d;
			uint8_t u8[sizeof(_T)];
		} bz;

		size_t toclean = (pos + sizeof(_T)) - cmsg->len;
		size_t start_pos = sizeof(_T) - toclean;

		//std::cout << "B> bzero s: " << sizeof(_T) << " c: " << toclean << " p: " << start_pos << std::endl;

		bz.d = buf;
		memset(&bz.u8[start_pos], 0, toclean);
		buf = bz.d;
	}

	// leXXtoh functions may be empty macros,
	// switch will be optimized-out
	switch (sizeof(_T)) {
	case 2:
		buf = le16toh(buf);
		break;

	case 4:
		buf = le32toh(buf);
		break;

	case 8:
		buf = le64toh(buf);
		break;

	default:
		assert(false);
	}

	if (std::is_floating_point<_Tout>::value) {
		data = *static_cast<_Tout *>(static_cast<void *>(&buf));
	} else {
		data = buf;
	}
}

template<typename _T>
void mavlink::MsgMap::operator>> (_T &data)
{
	assert(cmsg);
	assert(pos + sizeof(_T) <= MAVLINK_MAX_PAYLOAD_LEN);

	// message is trimmed - fill with zeroes
	if (pos >= cmsg->len) {
		data = 0;
		pos += sizeof(_T);
		return;
	}

	switch (sizeof(_T)) {
	case 1:
		data = _MAV_PAYLOAD(cmsg)[pos];
		break;

	case 2:
		uint16_t data_le16;
		cmsg_memcpy_bzero_swap_set_data(data_le16, data);
		break;

	case 4:
		uint32_t data_le32;
		cmsg_memcpy_bzero_swap_set_data(data_le32, data);
		break;

	case 8:
		uint64_t data_le64;
		cmsg_memcpy_bzero_swap_set_data(data_le64, data);
		break;

	default:
		assert(false);
	}

	//std::cout << "M> s: " << sizeof(_T) << " p: " << pos << " d: " << data << std::endl;

	pos += sizeof(_T);
}

template<class _T, size_t _Size>
void mavlink::MsgMap::operator>> (std::array<_T, _Size> &data)
{
	for (auto &v : data) {
		*this >> v;
	}
}

