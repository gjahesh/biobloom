/*
 * WindowedFileParser.h
 *
 *  Created on: Jul 18, 2012
 *      Author: cjustin
 */

#ifndef WINDOWEDFILEPARSER_H_
#define WINDOWEDFILEPARSER_H_
#include <vector>
#include <boost/unordered/unordered_map.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "DataLayer/FastaReader.h"
#include "Common/ReadsProcessor.h"
#include <deque>

using namespace std;
using namespace boost;

class WindowedFileParser {
public:
	//constructor/destructor
	explicit WindowedFileParser(const string &fileName, int16_t windowSize);
	const vector<string> getHeaders() const;
	void setLocationByHeader( const string &header);
	const size_t getSequenceSize( const string &header);
	const deque<char> &getNextDeq();
	const string &getNextSeq();
	const bool notEndOfSeqeunce();

	virtual ~WindowedFileParser();

private:
	struct FastaIndexValue {
		size_t index;
		size_t size;
		size_t start;
		size_t bpPerLine;
		size_t charsPerLine;
	};

	unordered_map<string, FastaIndexValue> fastaIndex;
	ifstream fastaFileHandle;
	unsigned short windowSize;
	vector<string> headers;
	string currentHeader;
	size_t currentCharNumber;
	size_t currentLinePos;
//	deque<char> window;
	string window;
	string currentString;
	ReadsProcessor proc;
	bool sequenceNotEnd;

	string bufferString; //so reallocation does not need to occur

	//helper methods
	void initializeIndex(string const &fileName);
//	unordered_map<string, fastaIndexValue> generateInternalIndex();
//	unordered_map<string, fastaIndexValue> loadInternalIndex(
//			ifstream indexFile);

};

#endif /* WINDOWEDFILEPARSER_H_ */