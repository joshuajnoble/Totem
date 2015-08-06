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

	void OtherThread();

public:
	H264NetworkReceiver();
	~H264NetworkReceiver();
	void Start(FrameCallback callback);
	void Close();
};