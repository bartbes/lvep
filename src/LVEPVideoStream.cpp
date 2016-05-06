#include "LVEPVideoStream.h"

#include <timer/Timer.h>

LVEPVideoStream::LVEPVideoStream(love::filesystem::File *file)
	: stream(new FFMpegStream(file, FFMpegStream::TYPE_VIDEO))
	, file(file)
	, dirty(false)
	, previousTime(0)
{
	frame = av_frame_alloc();
	if (!stream->readFrame(frame))
	{
		av_frame_free(&frame);
		delete stream;
		throw love::Exception("No first frame");
	}

	frontBuffer = allocateBuffer();
	backBuffer = allocateBuffer();

	// frameSync is a StrongRef, so it retains itself, so after set it has a reference
	// count of 2, rather than 1
	frameSync.set(new love::video::VideoStream::DeltaSync());
	frameSync->release();

	previousTime = love::timer::Timer::getTimeSinceEpoch();
}

LVEPVideoStream::~LVEPVideoStream()
{
	av_frame_free(&frame);
	delete frontBuffer;
	delete backBuffer;
	delete stream;
}

love::video::VideoStream::Frame *LVEPVideoStream::allocateBuffer()
{
	love::video::VideoStream::Frame *buffer;
	buffer = new love::video::VideoStream::Frame();
	buffer->yw = getWidth();
	buffer->yh = getHeight();

	// TODO: Format support (non yuv420p)
	// swrast to recode?
	buffer->cw = getWidth()/2;
	buffer->ch = getHeight()/2;

	buffer->yplane = new unsigned char[buffer->yw*buffer->yh];
	buffer->cbplane = new unsigned char[buffer->cw*buffer->ch];
	buffer->crplane = new unsigned char[buffer->cw*buffer->ch];

	return buffer;
}

int LVEPVideoStream::getWidth() const
{
	return frame->width;
}

int LVEPVideoStream::getHeight() const
{
	return frame->height;
}

const std::string &LVEPVideoStream::getFilename() const
{
	return file->getFilename();
}

void LVEPVideoStream::fillBackBuffer()
{
	// TODO: End-of-stream
	// TODO: Seeking
	double curTime = love::timer::Timer::getTimeSinceEpoch();
	double dt = curTime-previousTime;
	previousTime = curTime;

	frameSync->update(dt);
	double time = frameSync->getPosition();
	double pts = stream->translateTimestamp(frame->pkt_pts);

	if (time < pts)
		return;

	dirty = true;

	memcpy(backBuffer->yplane, frame->data[0], backBuffer->yw*backBuffer->yh);
	memcpy(backBuffer->cbplane, frame->data[1], backBuffer->cw*backBuffer->ch);
	memcpy(backBuffer->crplane, frame->data[2], backBuffer->cw*backBuffer->ch);

	stream->readFrame(frame);
}

const void *LVEPVideoStream::getFrontBuffer() const
{
	return (void*) frontBuffer;
}

size_t LVEPVideoStream::getSize() const
{
	return sizeof(love::video::VideoStream::Frame);
}

bool LVEPVideoStream::swapBuffers()
{
	if (!dirty)
		return false;

	dirty = false;
	love::video::VideoStream::Frame *temp = frontBuffer;
	frontBuffer = backBuffer;
	backBuffer = temp;
	return true;
}
