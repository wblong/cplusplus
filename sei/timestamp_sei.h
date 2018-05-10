#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace timestamp_sei
{
	struct SeiPayload
	{
		SeiPayload();
		unsigned long long pictrueId;
		unsigned long long gopId;
		unsigned long long timestamp;
		std::string version;
		std::string groupName;
		std::string mediaName;
	};

	std::shared_ptr<SeiPayload> h264GetSeiPayload(unsigned char const* pData, unsigned int len);

	std::shared_ptr<SeiPayload> hevcGetSeiPayload(unsigned char const* pData, unsigned int len);

	std::vector<unsigned char> h264GetSeiNalu(const SeiPayload& payload);

	std::vector<unsigned char> hevcGetSeiNalu(const SeiPayload& payload);

	/** 
	* @brief find sei nalu from orgion nalu 
	* @author xty
	* @date  2018/4/25
	* @param pData: nalu
	* @param len: size of nalu
	* @return >=0 the location of sei, -1 not found, -2 length no enough, -3 not start with 0001 or 001
	* @attention 
	* @Modified
	**/
	int h264FindSeiLoc(unsigned char const* pData, unsigned int len);

	/**
	* @brief find sei nalu from orgion nalu
	* @author xty
	* @date  2018/4/25
	* @param pData: nalu
	* @param len: size of nalu
	* @return >=0 the location of sei, -1 not found, -2 length no enough, -3 not start with 0001 or 001
	* @attention
	* @Modified
	**/
	int hevcFindSeiLoc(unsigned char const* pData, unsigned int len);
}