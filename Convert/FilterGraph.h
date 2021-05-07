#pragma once

namespace Avf{
	public ref class FilterGraph
	{
	public:
		FilterGraph();
		virtual ~FilterGraph();
	public:
		virtual void Close(void);
		virtual bool Next(void);
		virtual void Open(List<Source^>^ sources, List<Sink^>^ sinks, String^ specification);
	public:
		virtual property double Progress {
			double get();
		}
	private:
		AVFilterGraph* graph = nullptr;
		List<Sink^>^ sinks = gcnew List<Sink^>();
		List<Source^>^ sources = gcnew List<Source^>();
		String^ description = nullptr;
	};
	public ref class FilterGraphException :
		public Exception
	{
	public:
		FilterGraphException(String^ description, String^ message);
		virtual ~FilterGraphException();
	public:
		virtual property String^  Description {
			String^ get();
		}
	private:
		String^ description = nullptr;
	};
}
#define THROW_FILTERGRAPH_EXCEPTION(desc,fmt,...) \
	throw gcnew FilterGraphException(desc,String::Format(fmt,__VA_ARGS__))
