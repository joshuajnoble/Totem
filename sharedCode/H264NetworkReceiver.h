#pragma once

#include "FFmpegImports.h"

#include <string>

class H264NetworkReceiver
{
private:
	static DWORD WINAPI ThreadMarshaller(LPVOID);

	FFmpegFactory m_ffmpeg;
	bool initialized, closed;
	FrameCallback callback;
	HANDLE closeHandle, thread;

	AVFormatContext *ifmt_ctx;
	AVPacket pkt;
	int videoindex;
	std::string remoteIp;
	int remoteVideoPort;
	bool hasReceviedPackets;

	void OtherThread();

public:
	H264NetworkReceiver();
	~H264NetworkReceiver();
	void Start(const std::string& remoteIp, int remotePort, FrameCallback callback);
	void Close();

	bool isConnected();
};