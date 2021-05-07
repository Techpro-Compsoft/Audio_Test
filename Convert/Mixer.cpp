#include "stdafx.h"
#include "Mixer.h"

using namespace Avf;

Mixer::Mixer(Options^ options, Dictionary<String^, 
	String^>^ inputs, Dictionary<String^, String^>^ outputs, 
	List<String^>^ sequence ) :FilterGraph()
{
	this->options = options;
	this->inputs = inputs;
	this->outputs = outputs;
	this->sequence = sequence;
}
Mixer::~Mixer()
{
}
void Mixer::Open()
{
	List<Source^>^ sources = gcnew List<Source^>();
	List<Sink^>^  sinks = gcnew List<Sink^>();

	for each(String^ label in  gcnew HashSet<String^>(sequence)) {

		if (!this->inputs->ContainsKey(label)) {
			THROW_MIXER_EXCEPTION("No input found for label '{0}'.", label);
		}
		sources->Add(gcnew Source(label, this->inputs[label], this->options->Ignore));
	}
	for each(KeyValuePair<String^, String^>^ pair in this->outputs) {
		sinks->Add(gcnew Sink(pair->Key, pair->Value, this->options->Parameters));
	}
	String^ specification = this->Serialize(sequence, gcnew List<String^>(outputs->Keys));
	TRACE("FilterGraph specification: {0}", specification);
	FilterGraph::Open(sources, sinks, specification);
}
String^ Mixer::Serialize(List<String^>^ inputs, List<String^>^ outputs) 
{
	String^ whitespace = " ";
	String^ comma = ",";
	String^ semicolon = ";";

	StringBuilder^ builder = gcnew StringBuilder();
	Dictionary<String^, int>^ lookup = gcnew Dictionary<String^, int>();

	for each(String^ label in inputs) {
		lookup[label] = lookup->ContainsKey(label) ? lookup[label] + 1 : 0;
	}
	for each(KeyValuePair<String^, int>^ pair in lookup) {
		if (pair->Value) {
			builder->AppendFormat("[{0}] asplit={1}", pair->Key, pair->Value + 1);
			builder->AppendFormat(whitespace);
			for (int i = 0; i <= pair->Value; i++) {
				builder->AppendFormat("[{0}{1}]", pair->Key, i);
				if (i == pair->Value) {
					builder->AppendFormat(semicolon);
				}
				builder->AppendFormat(whitespace);
			}
		}
	}
	Dictionary<String^, int>^  counter = gcnew Dictionary<String^, int>();

	for (int i = 0; i < inputs->Count; i++) {
		if (lookup[inputs[i]]) {
			if (!counter->ContainsKey(inputs[i])) {
				counter[inputs[i]] = 0;
			}
			builder->AppendFormat("[{0}{1}]", inputs[i], counter[inputs[i]]);
			counter[inputs[i]] += 1;
		}
		else {
			builder->AppendFormat("[{0}]", inputs[i]);
		}
		builder->AppendFormat(whitespace);
	}
	builder->AppendFormat("amix=inputs={0}:duration=longest", inputs->Count);

	if (this->options->Normalize) {
		builder->AppendFormat(comma);
		builder->AppendFormat("dynaudnorm");
	}
	
	if (outputs->Count > 1) {
		builder->AppendFormat(comma);
		builder->AppendFormat(whitespace);
		builder->AppendFormat("asplit={0}", outputs->Count);

		for each(String^ output in outputs) {
			builder->AppendFormat(whitespace);
			builder->AppendFormat("[{0}]", output);
		}
	}
	else {
		builder->Append(whitespace);
		builder->AppendFormat("[{0}]", outputs[0]);
	}


	return builder->ToString();
}

cli::array<String^>^ Mixer::Inputs::get() {

	return  this->inputs ? (gcnew List<String^>(this->inputs->Values))->ToArray() : gcnew cli::array<String^>(0);

}
cli::array<String^>^ Mixer::Outputs::get() {

	return  this->outputs ? (gcnew List<String^>(this->outputs->Values))->ToArray() : gcnew cli::array<String^>(0);

}
cli::array<String^>^ Mixer::Sequence::get() {

	return  this->outputs ? (gcnew List<String^>(this->sequence))->ToArray() : gcnew cli::array<String^>(0);

}
MixerException::MixerException(String^ message) :Exception(message)
{
}
MixerException::~MixerException()
{
}
