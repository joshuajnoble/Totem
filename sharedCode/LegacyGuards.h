#pragma once

#include "FFmpegImports.h"
#include "stdio.h"

class GuardFileHandle
{
private:
	FILE *handle;

	// Not copyable
	GuardFileHandle(const GuardFileHandle& other) {}
	GuardFileHandle& operator = (const GuardFileHandle& other);

public:
	GuardFileHandle(FILE *fp) : handle(fp) {}
	~GuardFileHandle() { Cleanup(); }

	// Abondon the resource (presumably so the calling code can handle cleanup).
	void Release()
	{
		this->handle = NULL;
	}

	// Force the cleanup (so you don't have to wait to go out of scope).
	void Cleanup()
	{
		if (this->handle)
		{
			fclose(handle);
			handle = NULL;
		}
	}
};

class GuardCodecContext
{
private:
	FFmpegFactory *m_ffmpeg;
	AVCodecContext *handle;

	// Not copyable
	GuardCodecContext(const GuardCodecContext& other) {}
	GuardCodecContext& operator = (const GuardCodecContext& other);

public:
	GuardCodecContext(AVCodecContext *fp, FFmpegFactory *ffmpeg) : handle(fp), m_ffmpeg(ffmpeg) {}
	~GuardCodecContext() { Cleanup(); }

	// Abondon the resource (presumably so the calling code can handle cleanup).
	void Release()
	{
		this->handle = NULL;
	}

	// Force the cleanup (so you don't have to wait to go out of scope).
	void Cleanup()
	{
		if (this->handle)
		{
			m_ffmpeg->codec.avcodec_close(handle);
			m_ffmpeg->utils.av_free(handle);
			handle = NULL;
		}
	}
};