#pragma once
namespace Avf {
	 public ref class Source
	{
	public:
		Source(String^ label, String^ url);
		Source(String^ label, String^ url, bool ignore);
		virtual ~Source(void);
	public:
		virtual void Close(void);
		virtual void Configure(void);
		virtual void Insert(AVFilterGraph* graph);
		virtual void Open(void);
		virtual AVFilterInOut* Output(void);
		virtual void Read(void);
	public:
		virtual property bool Eof {
			bool get();
		}
		virtual property String^  Label {
			String^ get();
		}
		virtual property double Length {
			double get();
		}
		virtual property double Position {
			double get();
		}
		virtual property Avf::Parameters^  Parameters{
			 Avf::Parameters^ get();
		}
		virtual property String^  Url {
			String^ get();
		}
	private:
		bool eof = false;
		bool ignore = true;
		double length = 0;
		double position = 0;
		int stream = -1;
		String^ label = nullptr;
		String^ url = nullptr;
		AVFormatContext *format = nullptr;
		AVCodec *codec = nullptr;
		AVCodecContext *decoder = nullptr;
		AVFrame *frame = nullptr;
		AVFilterContext *filter = nullptr;
		AVPacket *packet = nullptr;
		Avf::Parameters^ parameters = gcnew Avf::Parameters();
	};
	 public ref class SourceException :
		public Exception
	{
	public:
		SourceException(String^ input, String^ message);
		virtual ~SourceException();
		virtual property String^  Url {
			String^ get();
		}
	private:
		String^ url = nullptr;
	};
}

#define THROW_SOURCE_EXCEPTION(url,fmt,...) \
	throw gcnew SourceException(url,String::Format(fmt,__VA_ARGS__))
