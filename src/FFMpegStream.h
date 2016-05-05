#pragma once

#include "LFSIOContext.h"

// STL
#include <string>

// LOVE
#include <common/Exception.h>
#include <common/Object.h>
#include <filesystem/File.h>

// FFMPEG
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
}

class FFMpegStream
{
public:
	enum StreamType
	{
		TYPE_VIDEO,
		TYPE_AUDIO,
	};

	FFMpegStream(love::filesystem::File *file, StreamType type);
	~FFMpegStream();

	bool readFrame(AVFrame *frame);
	double translateTimestamp(int64_t ts) const;

private:
	LFSIOContext ioContext;
	AVInputFormat *inputFormat;
	AVFormatContext *formatContext;
	AVCodecContext *codecContext;
	AVPacket packet;

	int targetStream;
	StreamType type;

	love::StrongRef<love::filesystem::File> file;

	bool readPacket();
};
