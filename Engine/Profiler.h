#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <sstream>
#include <vector>

#include <cassert>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std::chrono;
typedef duration<steady_clock::rep, steady_clock::period> DurationType;
typedef std::pair<std::string, DurationType> NameAndDuration;

#include "Utils/FileIO.h"

#define PROFILE_SCOPE(label, detail) ProfileMoment(label, detail);
#define PROFILE_SCOPE(label) ProfileMoment(label, "");

struct ProfileData
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	std::string label;
};

struct Entry {
	time_point<steady_clock> Start;
	DurationType Duration;
	std::string Name;
	std::string Detail;
};


#define MILLISECONDS 1000
#define MICROSECONDS 1000000
#define TIME_FORMAT MILLISECONDS


static std::string EscapeString(const char* src) {
	std::string os;
	while (*src) {
		char c = *src;
		switch (c) {
		case '"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			os += '\\';
			os += c;
			break;
		default:
			if (c >= 32 && c < 126) {
				os += c;
			}
		}
		++src;
	}
	return os;
}

class Profiler
{
public:
	Profiler() 
	{
		QueryPerformanceFrequency(&freq);

		Stack.reserve(8);
		Entries.reserve(128);
		StartTime = steady_clock::now();
	}
	~Profiler()
	{
		std::stringstream stream;
		Write(stream);
		FileIO::SaveTextFile("profiler_capture.json", stream.str());
	}

	size_t Start(const std::string& label)
	{
		ProfileData data;
		data.label = label;
		QueryPerformanceCounter(&data.start);

		m_data.push_back(data);
		return m_data.size() - 1;
	}

	void Stop(size_t index)
	{
		auto& data = m_data[index];
		QueryPerformanceCounter(&data.end);
	}

	void Begin(const std::string& name, const std::string& detail) {
		Entry e = { steady_clock::now(), {}, name, detail };
		Stack.emplace_back(e);
	}

	void End() {
		assert(!Stack.empty() && "Must call Begin first");
		auto& e = Stack.back();
		e.Duration = steady_clock::now() - e.Start;

		// only include sections longer than 500us
		if (duration_cast<microseconds>(e.Duration).count() > 500)
			Entries.emplace_back(e);

		// track total time taken by each "name", but only the topmost levels of them;
		// e.g. if there's a template instantiation that instantiates other templates
		// from within, we only want to add the topmost one.
		// "topmost" happens to be the ones that don't have any currently open
		// entries above itself.
		if (std::find_if(
			++Stack.rbegin(), Stack.rend(),
			[&](const Entry& val) { return val.Name == e.Name; }) == Stack.rend()) {
			TotalPerName[e.Name] += e.Duration;
			CountPerName[e.Name]++;
		}

		Stack.pop_back();
	}

	void Write(std::stringstream& os) {
		assert(Stack.empty() &&
			"All profiler sections should be ended when calling Write");

		os << "{ \"traceEvents\": [\n";

		// emit all events for the main flame graph
		for (const auto& e : Entries) {
			auto startUs = duration_cast<microseconds>(e.Start - StartTime).count();
			auto durUs = duration_cast<microseconds>(e.Duration).count();
			os << "{ \"pid\":1, \"tid\":0, \"ph\":\"X\", \"ts\":" << startUs
				<< ", \"dur\":" << durUs << ", \"name\":\""
				<< EscapeString(e.Name.c_str()) << "\", \"args\":{ \"detail\":\""
				<< EscapeString(e.Detail.c_str()) << "\"} },\n";
		}

		// emit totals by section name as additional "thread" events, sorted from
		// longest one
		int tid = 1;
		std::vector<NameAndDuration> sortedTotals;
		sortedTotals.reserve(TotalPerName.size());
		for (const auto& e : TotalPerName) {
			sortedTotals.push_back(e);
		}
		std::sort(sortedTotals.begin(), sortedTotals.end(), [](const NameAndDuration& a, const NameAndDuration& b) { return a.second > b.second; });
		for (const auto& e : sortedTotals) {
			auto durUs = duration_cast<microseconds>(e.second).count();
			os << "{ \"pid\":1, \"tid\":" << tid << ", \"ph\":\"X\", \"ts\":" << 0
				<< ", \"dur\":" << durUs << ", \"name\":\"Total "
				<< EscapeString(e.first.c_str()) << "\", \"args\":{ \"count\":"
				<< CountPerName[e.first] << ", \"avg ms\":"
				<< (durUs / CountPerName[e.first] / 1000)
				<< "} },\n";
			++tid;
		}

		// emit metadata event with process name
		os << "{ \"cat\":\"\", \"pid\":1, \"tid\":0, \"ts\":0, \"ph\":\"M\", "
			"\"name\":\"process_name\", \"args\":{ \"name\":\"clang\" } }\n";
		os << "] }\n";
	}

	std::vector<Entry> Stack;
	std::vector<Entry> Entries;
	std::unordered_map<std::string, DurationType> TotalPerName;
	std::unordered_map<std::string, size_t> CountPerName;
	time_point<steady_clock> StartTime;

	size_t GetStartTime(size_t index)
	{
		auto& data = m_data[index];
		return (data.start.QuadPart / TIME_FORMAT);
	}

	size_t GetEndTime(size_t index)
	{
		auto& data = m_data[index];
		return (data.end.QuadPart / TIME_FORMAT);
	}

	size_t GetDuration(size_t index)
	{
		auto& data = m_data[index];
		return ((data.end.QuadPart - data.start.QuadPart) * TIME_FORMAT / freq.QuadPart);
	}

	static Profiler& Instance() { return m_instance; }
	std::vector<ProfileData>& GetData() { return m_data; }

private:
	LARGE_INTEGER freq;

	std::vector<ProfileData> m_data;

	static Profiler m_instance;
};

Profiler Profiler::m_instance = Profiler();

struct ProfileMoment
{
	ProfileMoment(const std::string& label, const std::string& detail)
	{
		Profiler::Instance().Begin(label, detail);
	}

	~ProfileMoment()
	{
		Profiler::Instance().End();
	}
};

