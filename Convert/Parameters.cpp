#include "stdafx.h"
#include "Parameters.h"

using namespace Avf;

Parameters::Parameters()
{
}
Parameters::~Parameters()
{
}
Parameters^ Parameters::Read(String^ url) {
	Parameters^ parameters = nullptr;
	AVFormatContext* format = nullptr;
	std::string input = Interop::convert(url);
	int ret = avformat_open_input(&format, input.c_str(), nullptr, nullptr);

	if (ret >= 0) {
		ret = avformat_find_stream_info(format, nullptr);

		if (ret >= 0) {
			AVCodec *codec = nullptr;
			int n = ret = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
			if (ret >= 0) {
				AVStream* stream = format->streams[n];

				if (stream->duration != AV_NOPTS_VALUE) {

					parameters = gcnew Parameters();
					parameters->Duration = Util::Time(stream->duration, stream->time_base);
					parameters->BitRate = stream->codecpar->bit_rate;
					parameters->Channels = stream->codecpar->channels;
					parameters->ChannelLayout = stream->codecpar->channel_layout;
					parameters->SampleRate = stream->codecpar->sample_rate;

					if (format->metadata) {
						AVDictionaryEntry *entry = nullptr;
						while (entry = av_dict_get(format->metadata, "", entry, AV_DICT_IGNORE_SUFFIX)) {
							parameters->Metadata[Interop::convert(entry->key)] = Interop::convert(entry->value);
						}
					}
				}
			}
		}
		avformat_close_input(&format);
	}
	return parameters;
}

Parameters^ Parameters::Clone() {
	Parameters^  clone = gcnew Parameters();
	clone->bitrate = this->bitrate;
	clone->channels = this->channels;
	clone->sample_rate = this->sample_rate;
	clone->duration = this->duration;
	clone->quality = this->quality;
	clone->channel_layout = this->channel_layout;
	clone->bitrate_mode = this->bitrate_mode;
	clone->metadata = gcnew Dictionary<String^, String^>(this->metadata);
	return clone;
}
Int64 Parameters::BitRate::get()
{
	return this->bitrate;
}
void Parameters::BitRate::set(Int64 value)
{
	this->bitrate = value;
}
Avf::BitRateMode Parameters::BitRateMode::get()
{
	return this->bitrate_mode;
}
void Parameters::BitRateMode::set(Avf::BitRateMode value)
{
	this->bitrate_mode = value;
}
int Parameters::Channels::get()
{
	return this->channels;
}
void Parameters::Channels::set(int value)
{
	this->channels = value;
}
Int64 Parameters::ChannelLayout::get()
{
	return this->channel_layout;
}
void Parameters::ChannelLayout::set(Int64 channel_layout)
{
	switch (channel_layout) {
	case (Int64)Avf::ChannelLayout::Mono:
	{
		this->channels = 1;
	}
	break;
	case (Int64)Avf::ChannelLayout::Stereo:
	{
		this->channels = 2;
	}
	break;
	default:break;
	}
	this->channel_layout = channel_layout;
}

double Parameters::Duration::get() {
	return this->duration;
}
void Parameters::Duration::set(double value) {
	this->duration = value;
}

Dictionary<String^, String^>^  Parameters::Metadata::get() {
	return this->metadata;
}
double Parameters::Quality::get()
{
	return this->quality;
}
void Parameters::Quality::set(double value)
{
	this->quality = value;
}
int Parameters::SampleRate::get()
{
	return this->sample_rate;
}
void Parameters::SampleRate::set(int value)
{
	this->sample_rate = value;
}
