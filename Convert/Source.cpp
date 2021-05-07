#include "stdafx.h"
#include "Source.h"

using namespace Avf;

Source::Source(String^ label, String^ url, bool ignore)
{
	this->label = label;
	this->url = url;
	this->ignore = ignore;
}
Source::Source(String^ label, String^ url) :Source(label, url, true)
{
}
Source::~Source()
{
}
void Source::Close(void)
{
	this->eof = false;
	this->length = 0;
	this->position = 0;
	this->stream = -1;
	this->codec = nullptr;
	this->filter = nullptr;

	if (this->frame) {
		AVFrame* frame = this->frame;
		this->frame = nullptr;
		av_frame_free(&frame);
	}
	if (this->packet) {
		AVPacket* packet = this->packet;
		this->packet = nullptr;
		av_packet_free(&packet);
	}
	if (this->filter) {
		AVFilterContext* filter = this->filter;
		this->filter = nullptr;
		avfilter_free(filter);
	}
	if (this->decoder) {
		AVCodecContext* decoder = this->decoder;
		this->decoder = nullptr;
		avcodec_free_context(&decoder);
	}
	if (this->format) {
		AVFormatContext* format = this->format;
		this->format = nullptr;
		avformat_close_input(&format);
	}

}
void Source::Configure(void)
{
}
void Source::Insert(AVFilterGraph* graph)
{
	const AVFilter *abuffer = avfilter_get_by_name("abuffer");
	if (!abuffer) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot find filter 'abuffer' for input file.");
	}

	std::string label = Interop::convert(this->label);
	std::string parameters = Interop::format(
		"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llx",
		this->decoder->time_base.num, this->decoder->time_base.den,
		this->decoder->sample_rate,
		av_get_sample_fmt_name(this->decoder->sample_fmt),
		this->decoder->channel_layout);
	AVFilterContext *filter = nullptr;
	int ret = avfilter_graph_create_filter(&filter, abuffer, label.c_str(), parameters.c_str(), NULL, graph);
	this->filter = filter;
	if (ret < 0) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot create filter 'abuffer' for input.");
	}
}
void Source::Open(void)
{
	AVFormatContext* format = nullptr;
	std::string url = Interop::convert(this->url);
	int ret = avformat_open_input(&format, url.c_str(), nullptr, nullptr);
	this->format = format;
	if (ret < 0) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot open input.");
	}

	ret = avformat_find_stream_info(this->format, nullptr);
	if (ret < 0) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot find stream information for input.");
	}

	AVCodec *codec = nullptr;
	ret = this->stream = av_find_best_stream(this->format, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	this->codec = codec;
	if (ret < 0) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot find audio stream for input.)");
	}

	AVStream* stream = this->format->streams[this->stream];
	if (stream->duration == AV_NOPTS_VALUE) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot read duration of audio stream {0} of input", this->stream);
	}
	this->length = Util::Time(stream->duration, stream->time_base);

	this->parameters->BitRate = stream->codecpar->bit_rate;
	this->parameters->Channels = stream->codecpar->channels;
	this->parameters->ChannelLayout = stream->codecpar->channel_layout;
	this->parameters->SampleRate = stream->codecpar->sample_rate;

	if (this->format->metadata) {
		AVDictionaryEntry *entry = nullptr;
		while (entry = av_dict_get(this->format->metadata, "", entry, AV_DICT_IGNORE_SUFFIX)) {
			this->parameters->Metadata[Interop::convert(entry->key)] = Interop::convert(entry->value);
		}
	}

	this->decoder = avcodec_alloc_context3(this->codec);
	if (!this->decoder) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot allocate audio decoder for input.");
	}

	avcodec_parameters_to_context(this->decoder, this->format->streams[this->stream]->codecpar);

	ret = avcodec_open2(this->decoder, this->codec, nullptr);
	if (ret < 0) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot open audio decoder for input.");
	}
}
AVFilterInOut* Source::Output(void)
{
	std::string url = Interop::convert(this->url);
	std::string label = Interop::convert(this->label);

	AVFilterInOut* output = avfilter_inout_alloc();
	if (!output) {
		THROW_SOURCE_EXCEPTION(this->url, "Cannot allocate link for input.");
	}
	output->name = av_strdup(label.c_str());
	output->filter_ctx = this->filter;
	output->pad_idx = 0;
	output->next = nullptr;
	return output;
}
void Source::Read(void)
{
	if (!this->eof) {

		if (!this->frame) {
			this->frame = av_frame_alloc();
		}
		if (!this->packet) {
			this->packet = av_packet_alloc();
		}
		try {
			TRACE("Reading packet for input.({0})", this->url);

			int ret = av_read_frame(this->format, this->packet);

			if (ret < 0 && ret != AVERROR_EOF && !this->ignore) {
				THROW_SOURCE_EXCEPTION(this->url, "Cannot read packet for input.");
			}
			else {
				this->eof = (ret == AVERROR_EOF) || (ret < 0 && this->ignore);

				if (ret == AVERROR_EOF || (ret < 0 && this->ignore) || this->stream == packet->stream_index) {
					if (ret == AVERROR_EOF || (ret < 0 && this->ignore)) {
						ret = avcodec_send_packet(this->decoder, nullptr);
						TRACE("Sending eof to the decoder for input. ({0})", this->url);
					}
					else {
						TRACE("Sending packet to the decoder for input. ({0})", this->url);
						av_packet_rescale_ts(this->packet, this->format->streams[this->stream]->time_base, this->decoder->time_base);
						ret = avcodec_send_packet(this->decoder, this->packet);
					}
					av_packet_unref(this->packet);
					this->eof = ret < 0;
					if (ret < 0 && !this->ignore) {
						THROW_SOURCE_EXCEPTION(this->url, "Cannot send packet to the decoder for input.");
					}
					else {
						bool eof2 = ret < 0;
						do {
							TRACE("Receiving frame from the decoder for input. ({0})", this->url);
							ret = eof2 ? AVERROR_EOF : avcodec_receive_frame(this->decoder, this->frame);

							if (ret == AVERROR(EAGAIN) && !this->eof) {
								break;
							}
							else if (ret < 0 && ret != AVERROR_EOF && !this->ignore) {
								THROW_SOURCE_EXCEPTION(this->url, "Cannot receive frame from decoder for input.");
							}
							else {
								eof2 = (ret == AVERROR_EOF) || (ret < 0 && this->ignore);
								if (eof2) {
									ret = av_buffersrc_add_frame_flags(this->filter, nullptr, AV_BUFFERSRC_FLAG_PUSH);
									this->position = this->length;
									TRACE("Feeding eof to the source filter for input. ({0})", this->url);
								}
								else {
									if (frame->pts == AV_NOPTS_VALUE) {
										THROW_SOURCE_EXCEPTION(this->url, "Presentational timestamp is not available for input.");
									}
									else {
										double  position = Util::Time(frame->pts, this->decoder->time_base);
										if (this->position &&  position < this->position) {
											THROW_SOURCE_EXCEPTION(this->url, "Presentational timestamp is not monotonically increasing for input.");
										}
										if (this->length < position) {
											this->length = position;
										}
										this->position = position;
									}
									ret = av_buffersrc_add_frame_flags(this->filter, this->frame, AV_BUFFERSRC_FLAG_PUSH);
								}
								TRACE("Feeding frame to the source filter for input. ({0})", this->url);
								if (ret < 0) {
									THROW_SOURCE_EXCEPTION(this->url, "Cannot feed frame into source filter for input.");
								}
							}
							av_frame_unref(this->frame);
						} while (!eof2);
						this->eof = this->eof || eof2;
					}
				}
			}
		}
		finally{

			if (this->eof) this->position = this->length;

			av_packet_unref(this->packet);
			av_frame_unref(this->frame);

		}
	}
}
bool Source::Eof::get()
{
	return this->eof;
}
String^ Source::Label::get()
{
	return this->label;
}
double Source::Length::get()
{
	return this->length;
}
double Source::Position::get()
{
	return this->position;
}
Avf::Parameters^ Source::Parameters::get()
{
	return this->parameters;
}
String^ Source::Url::get()
{
	return this->url;
}
SourceException::SourceException(String^ url, String^ message) :Exception(message)
{
	this->url = url;
}
SourceException::~SourceException()
{
}
String^ SourceException::Url::get() {
	return this->url;
}
