#include "timestamp_sei.h"

namespace timestamp_sei {

	SeiPayload::SeiPayload()
		: gopId(0)
		, pictrueId(0)
		, timestamp(0){}

	//解析SeiPayload
	std::shared_ptr<SeiPayload> parseSeiPayload(const std::string& str)
	{
		std::shared_ptr<SeiPayload> pSei(new SeiPayload);
		size_t offset = 0;
		//字符串定位，查找
		size_t find = std::string::npos;
		do {
			find = str.find(',', offset);
			if (offset >= str.length())
				break;
			if (find == std::string::npos)
				find = str.length();

			auto sub = str.substr(offset, find - offset);
			offset = find + 1;

			auto kvpos = sub.find('=');
			if (kvpos == std::string::npos)
				continue;
			auto key = sub.substr(0, kvpos);
			auto value = sub.substr(kvpos + 1, sub.length() - (kvpos + 1));
			if (key.compare("version") == 0)
				pSei->version = value;
			else if (key.compare("group") == 0)
				pSei->groupName = value;
			else if (key.compare("media") == 0)
				pSei->mediaName = value;
			else if (key.compare("gop_id") == 0)
				pSei->gopId = std::stoull(value);
			else if (key.compare("picture_id") == 0)
				pSei->pictrueId = std::stoull(value);
			else if (key.compare("timestamp") == 0)
				pSei->timestamp = std::stoull(value);
		} while (true);

		return pSei;
	}

	//将SeiPayload 序列化字符串
	std::string stringfySeiPayload(const SeiPayload& sei)
	{
		return  "version=" + sei.version + "," +
			"group=" + sei.groupName + "," +
			"media=" + sei.mediaName + "," +
			"gop_id=" + std::to_string(sei.gopId) + "," +
			"picture_id=" + std::to_string(sei.pictrueId) + ","
			"timestamp=" + std::to_string(sei.timestamp);
	}



	enum CodecID
	{
		H264 = 2,
		HEVC = 3
	};

	//nalu 头信息
	unsigned char S_NaluHeader[] = { 0x00, 0x00, 0x00, 0x01 };
	
	//自定义SEI的UUID(unregister uuid)
	// python code
	// import _uuid
	// _uuid._uuid5(_uuid.NAMESPACE_DNS, 'www.iseetech.com.cn')
	//-------------------------------------------------------------
	// pip install uuid
	// python code 
	// import uuid
	// uuid.uuid5(uuid.NAMESPACE_DNS, 'www.iseetech.com.cn')
	// UUID('a2 11 1a 9d 5b 0b 5f e6 a2 e3 10 16 8b 71 b2 0c')
	unsigned char S_Uuid[] = {

		0xa2, 0x11, 0x1a, 0x9d,
		0x5b, 0x0b, 0x5f, 0xe6,
		0xa2, 0xe3, 0x10, 0x16,
		0x8b, 0x71, 0xb2, 0x0c };
	
	//RBSP trailer
	unsigned char S_Rbsp = 0x80 ;
	
	enum {NaluHeaderLen = 4, UuidLen = 16, RbspLen = 1};

	//SeiTimestamp 模板实例化
	template<CodecID CID> class SeiTT;
	template<> class SeiTT<H264>
	{
	public:
		enum { HeaderLen = 2 };
		static unsigned char Header[HeaderLen];
	};
	unsigned char SeiTT<H264>::Header[] = { 0x06, 0x05 };

	template<> class SeiTT<HEVC>
	{
	public:
		enum { HeaderLen = 3 };
		static unsigned char Header[HeaderLen];
	};
	unsigned char SeiTT<HEVC>::Header[] = { 0x4e, 0x01, 0x05 };
	//!结构体对象-》内存字节转化
	template <CodecID cid>
	std::vector<unsigned char> seiToNalu(const SeiPayload& sei)
	{
		std::string content = stringfySeiPayload(sei);
		//size
		unsigned int payloadSize = UuidLen + (unsigned int)content.size() + 1;	//这个1表示字符串的结束符'\0'
		//size size 即使用多少个字节存储 payload
		unsigned int payloadSizeSize = payloadSize /0xff + 1;

		std::vector<unsigned char> buf;
		//设置buf的大小
		buf.reserve(NaluHeaderLen + SeiTT<cid>::HeaderLen + payloadSizeSize + payloadSize + RbspLen);
		//insert start_code
		buf.insert(buf.end(), S_NaluHeader, S_NaluHeader + NaluHeaderLen);
		//sei_header
		buf.insert(buf.end(), SeiTT<cid>::Header, SeiTT<cid>::Header + SeiTT<cid>::HeaderLen);
		//0xff
		buf.insert(buf.end(), payloadSizeSize - 1, 0xff);//
		buf.push_back((unsigned char)(payloadSize % 0xff ));//remainder
		//uuid
		buf.insert(buf.end(), S_Uuid, S_Uuid+UuidLen);
		//content
		buf.insert(buf.end(), content.begin(), content.end());
		buf.push_back(0x00);
		//trail
		buf.push_back(S_Rbsp);
		return buf;
	}
	//!内存字节-》结构体对象转化
	template<CodecID cid>
	std::shared_ptr<SeiPayload> naluToSei(const unsigned char* pNalu, unsigned int naluSize)
	{
		if (naluSize < NaluHeaderLen + SeiTT<cid>::HeaderLen + UuidLen)
			return nullptr;

		for (int i = 0; i < NaluHeaderLen; ++i)
			if (*pNalu++ != S_NaluHeader[i])
				return nullptr;

		naluSize -= NaluHeaderLen;

		for (int i = 0; i < SeiTT<cid>::HeaderLen; ++i)
			if (*pNalu++ != SeiTT<cid>::Header[i])
				return nullptr;
	
		naluSize -= SeiTT<cid>::HeaderLen;
		//计算SEI长度
		//后置先用后算
		unsigned int payloadSize = 0;
		do {
			payloadSize += *pNalu;
			naluSize -= 1;
		} while (*pNalu++ == 0xff);

		//如果长度不够
		if (naluSize < payloadSize + 1 || pNalu[payloadSize] != S_Rbsp)
			return nullptr;

		for (int i = 0; i < UuidLen; ++i)
		{
			if (*pNalu++ != S_Uuid[i])
				return nullptr;
		}
		
		return parseSeiPayload(std::string((char*)pNalu, payloadSize - UuidLen));
	}

	template<CodecID cid>
	int findSeiLoc(unsigned char const* pData, unsigned int len)
	{
		
	}

	std::shared_ptr<SeiPayload> h264GetSeiPayload(unsigned char const* pData, unsigned int len)
	{
		return naluToSei<H264>(pData, len);
	}
	std::shared_ptr<SeiPayload> hevcGetSeiPayload(unsigned char const* pData, unsigned int len)
	{
		return naluToSei<HEVC>(pData, len);
	}
	std::vector<unsigned char> h264GetSeiNalu(const SeiPayload& payload)
	{
		return seiToNalu<H264>(payload);
	}
	std::vector<unsigned char> hevcGetSeiNalu(const SeiPayload& payload)
	{
		return seiToNalu<HEVC>(payload);
	}
	//!查找 h264 sei 位置
	//!返回查找位置
	int h264FindSeiLoc(unsigned char const * pData, unsigned int len)
	{
		//
		if (len < 6)
			return -2;

		//检测开头  3或4个字节
		//1:00 00 01
		//2:00 00 00 01

		if (!((pData[0] == 0x0 && pData[1] == 0x0 && pData[2] == 0x1) ||
			(pData[0] == 0x0 && pData[1] == 0x0 && pData[2] == 0x0 && pData[3] == 0x1)))
			return -3;
		// 1或2个字节
		// 搜索起始位置
		int index = -1;
		for (auto i = 0U; i < len - 6; i++)
		{
			int naluHeaderLen = 0;
			if (pData[i + 0] == 0x0 && pData[i + 1] == 0x0 && pData[i + 2] == 0x1)
				naluHeaderLen = 3;
			else if (pData[i + 0] == 0x0 && pData[i + 1] == 0x0 && pData[i + 2] == 0x0 && pData[i + 3] == 0x1)
				naluHeaderLen = 4;
			if (naluHeaderLen <= 0)
				continue;
			//0x1f=0000 1111
			int naluType = pData[i + naluHeaderLen] & 0x1f;
			if (pData[i + naluHeaderLen] == SeiTT<H264>::Header[0] && 
				pData[i + naluHeaderLen + 1] == SeiTT<H264>::Header[1])
			{
				index = i;
				break;
			}
			else if (naluType != 0x07 && naluType != 0x08) //07:sps 08:pps
				break;
			else
			{
				i += (naluHeaderLen);
			}
			
		}
		return index;
	}

	int hevcFindSeiLoc(unsigned char const * pData, unsigned int len)
	{
		if (len < 7)
			return -2;

		if (!((pData[0] == 0x0 && pData[1] == 0x0 && pData[2] == 0x1) ||
			(pData[0] == 0x0 && pData[1] == 0x0 && pData[2] == 0x0 && pData[3] == 0x1)))
			return -3;

		int index = -1;
		for (auto i = 0U; i < len - 7; i++)
		{
			int naluHeaderLen = 0;
			if (pData[i + 0] == 0x0 && pData[i + 1] == 0x0 && pData[i + 2] == 0x1)
				naluHeaderLen = 3;
			else if (pData[i + 0] == 0x0 && pData[i + 1] == 0x0 && pData[i + 2] == 0x0 && pData[i + 3] == 0x1)
				naluHeaderLen = 4;
			if (naluHeaderLen <= 0)
				continue;

			int naluType = (pData[i + naluHeaderLen] & 0x7E) >> 1;
			//printf("[%x %x %x]\n" , pData[i + naluHeaderLen], pData[i + naluHeaderLen + 1], pData[i + naluHeaderLen + 2]);
			if (pData[i + naluHeaderLen] == SeiTT<HEVC>::Header[0] && 
				pData[i + naluHeaderLen + 1] == SeiTT<HEVC>::Header[1] && 
				pData[i + naluHeaderLen + 2] == SeiTT<HEVC>::Header[2])
			{
				index = i;
				break;
			}
			else if (naluType != 0x20 && naluType != 0x21 && naluType != 0x22) //20:vps 21:sps 22:pps
				break;
			else
			{
				i += (naluHeaderLen);
			}
		}
		return index;
	}
}
