#include "stdafx.h"
#include "FilterGraph.h"

using namespace Avf;

FilterGraph::FilterGraph()
{
}

FilterGraph::~FilterGraph()
{
}
void FilterGraph::Close(void) {
	for each(Source^ source in this->sources) {
		source->Close();
	}
	this->sources->Clear();

	for each(Sink^ sink in this->sinks) {
		sink->Close();
	}
	this->sinks->Clear();

	if (this->graph) {
		AVFilterGraph* graph = this->graph;
		this->graph = nullptr;
		avfilter_graph_free(&graph);
	}
}
void FilterGraph::Open(List<Source^>^ sources, List<Sink^>^ sinks, String^ description)
{
	this->sources = sources;
	this->sinks = sinks;
	this->description = description;

	for each(Source^ source in this->sources) {
		source->Open();	
	}
	for each(Sink^ sink in this->sinks) {
		sink->Open();
	}

	this->graph = avfilter_graph_alloc();
	if (!this->graph) {
		THROW_FILTERGRAPH_EXCEPTION(this->description,"Cannot allocate filtergraph.");
	}

	for each(Source^ source in this->sources) {
		source->Insert(this->graph);
	}
	for each(Sink^ sink in this->sinks) {
		sink->Insert(this->graph);
	}

	AVFilterInOut * outputs = nullptr;
	for each(Source^ source in this->sources) {
		AVFilterInOut* output = source->Output();
		output->next = outputs;
		outputs = output;
	}

	AVFilterInOut * inputs = nullptr;
	for each(Sink^ sink in this->sinks) {
		AVFilterInOut* input = sink->Input();
		input->next = inputs;
		inputs = input;
	}

	std::string desc = Interop::convert(this->description);
	int ret = avfilter_graph_parse_ptr(this->graph, desc.c_str(), &inputs, &outputs, nullptr);
	if (ret < 0) {
		THROW_FILTERGRAPH_EXCEPTION(this->description,"Cannot parse filtergraph description.");
	}

	ret = avfilter_graph_config(this->graph, NULL);
	if (ret < 0) {
		THROW_FILTERGRAPH_EXCEPTION(this->description,"Cannot configure filtergraph.");
	}

	for each(Source^ source in this->sources) {
		source->Configure();
	}
	for each(Sink^ sink in this->sinks) {
		sink->Configure();
	}
}
bool FilterGraph::Next(void) {
	bool eof = true;
	Source^ minimum = nullptr;
	for each(Source^ source in this->sources) {
		if (!source->Eof) {
			if (!minimum) {
				minimum = source;
			}
			else if(minimum->Position > source->Position){
				minimum = source;
			}
		}
	}
	if (minimum) {

		double position = minimum->Position;

		while (minimum->Position <= position) {
			minimum->Read();
			TRACE("Reading {0} of {1} for input.({2})", minimum->Position, minimum->Length, minimum->Url);
		}
	}
	for each(Sink^ sink in this->sinks) {
		if (!sink->Eof) {
			sink->Write();
			eof = eof && sink->Eof;
		}
	}
	return !eof;
}
double FilterGraph::Progress::get() {
	double position = 0;
	double length = 0;
	for each(Source^ source in this->sources) {
		if (length < source->Length) {
			length = source->Length;
			position = source->Position;
		}
	}
	return length ? (position/ length) : 0;
}
FilterGraphException::FilterGraphException(String^ description, String^ message) :Exception(message)
{
	this->description = description;
}
FilterGraphException::~FilterGraphException()
{
}
String^ FilterGraphException::Description::get() {
	return this->description;
}
