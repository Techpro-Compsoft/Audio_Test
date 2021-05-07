#include "stdafx.h"
#include "Options.h"

using namespace Avf;

Options::Options()
{
}
Options::~Options()
{
}
Options^ Options::Clone() {

	Options^ clone = gcnew Options();
	clone->ignore = this->ignore;
	clone->normalize = this->normalize;
	clone->parameters = this->parameters->Clone();
	return clone;
}
bool Options::Ignore::get() {
	return this->ignore;
}
void Options::Ignore::set(bool value) {
	this->ignore;
}
bool Options::Normalize::get() {
	return this->normalize;
}
void Options::Normalize::set(bool value) {
	this->normalize = value;
}
Parameters^ Options::Parameters::get() {
	return this->parameters;
}
