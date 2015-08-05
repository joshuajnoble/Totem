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

class GuardAVCondexContext
{
private:
	FFmpegFactory *m_ffmpeg;
	AVCodecContext *handle;

	// Not copyable
	GuardAVCondexContext(const GuardAVCondexContext& other) {}
	GuardAVCondexContext& operator = (const GuardAVCondexContext& other);

public:
	GuardAVCondexContext(AVCodecContext *fp, FFmpegFactory *ffmpeg) : handle(fp), m_ffmpeg(ffmpeg) {}
	~GuardAVCondexContext() { Cleanup(); }

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
			m_ffmpeg->codec.avcodec_close(this->handle);
			m_ffmpeg->utils.av_free(this->handle);
			this->handle = NULL;
		}
	}
};

class GuardAVFormatContext
{
private:
	FFmpegFactory *m_ffmpeg;
	AVFormatContext *handle;

	// Not copyable
	GuardAVFormatContext(const GuardAVFormatContext& other) {}
	GuardAVFormatContext& operator = (const GuardAVFormatContext& other);

public:
	GuardAVFormatContext(AVFormatContext *fp, FFmpegFactory *ffmpeg) : handle(fp), m_ffmpeg(ffmpeg) {}
	~GuardAVFormatContext() { Cleanup(); }

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
			m_ffmpeg->format.avformat_close_input(&this->handle);
			this->handle = NULL;
		}
	}
};