#pragma once


namespace Avf {
	public ref class Sink
	{
	public:
		Sink(String^ label, String^ url);
		Sink(String^ label, String^ url, Parameters^ parameters);
		virtual ~Sink(void);
	public:
		virtual void Close(void);
		virtual void Configure(void);
		virtual void Open(void);
		virtual AVFilterInOut* Input(void);
		virtual void Insert(AVFilterGraph* graph);
		virtual void Write(void);
	public:
		virtual property bool Eof {
			bool get();
		}
		virtual property String^  Label {
			String^ get();
		}
		virtual property Avf::Parameters^  Parameters{
			 Avf::Parameters^ get();
		}
		virtual property String^  Url {
			String^ get();
		}
	private:
		bool eof = false;
		bool header = false;
		double position = 0;
		String^ label = nullptr;
		String^  url = nullptr;
		AVCodec *codec = nullptr;
		AVCodecContext *encoder = nullptr;
		AVFrame		*frame = nullptr;
		AVFilterContext* filter = nullptr;
		AVFormatContext *format = nullptr;
		AVPacket *packet = nullptr;
		AVStream *stream = nullptr;
		Avf::Parameters^ parameters = gcnew Avf::Parameters();

	};
	public ref class SinkException :
		public Exception
	{
	public:
		SinkException(String^ input, String^ message);
		virtual ~SinkException();
		virtual property String^  Url {
			String^ get();
		}
	private:
		String^ url = nullptr;
	};
}

#define THROW_SINK_EXCEPTION(url,fmt,...) \
	throw gcnew SinkException(url,String::Format(fmt,__VA_ARGS__))
