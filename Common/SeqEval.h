/*
 * SeqEval.h
 *
 * Algorithms for bloomfilter based evaluation on sequences
 *
 *  Created on: Mar 10, 2015
 *      Author: cjustin
 *
 * Todo: try to expand and transfer methods from BioBloomClassifier
 */

#ifndef SEQEVAL_H_
#define SEQEVAL_H_

#include <string>
#include "boost/unordered/unordered_map.hpp"
#include "DataLayer/FastaReader.h"
#include "Common/Options.h"

using namespace std;
using namespace boost;

namespace SeqEval {
/*
 * Evaluation algorithm with no hashValue storage (optimize speed for single queries)
 */
inline bool evalSingle(const FastqRecord &rec, unsigned kmerSize, const BloomFilter &filter,
		double threshold, size_t antiThreshold)
{
	ReadsProcessor proc(kmerSize);
	size_t currentLoc = 0;
	double score = 0;
	unsigned antiScore = 0;
	unsigned streak = 0;
	while (rec.seq.length() >= currentLoc + kmerSize) {
		const unsigned char* currentKmer = proc.prepSeq(rec.seq, currentLoc);
		if (streak == 0) {
			if (currentKmer != NULL) {
				if (filter.contains(currentKmer)) {
					score += 0.5;
					++streak;
					if (threshold <= score) {
						return true;
					}
				}
				else if (antiThreshold <= ++antiScore) {
					return false;
				}
				++currentLoc;
			} else {
				if (currentLoc > kmerSize) {
					currentLoc += kmerSize + 1;
					antiScore += kmerSize + 1;
				} else {
					++antiScore;
					++currentLoc;
				}
				if (antiThreshold <= antiScore) {
					return false;
				}
			}
		} else {
			if (currentKmer != NULL) {
				if (filter.contains(currentKmer)) {
					++streak;
					score += 1 - 1 / (2 * streak);
					++currentLoc;

					if (threshold <= score) {
						return true;
					}
					continue;
				}
				else if (antiThreshold <= ++antiScore) {
					return false;
				}
			} else {
				currentLoc += kmerSize + 1;
				antiScore += kmerSize + 1;
			}
			if (streak < opt::streakThreshold) {
				++currentLoc;
			} else {
				currentLoc += kmerSize;
				antiScore += kmerSize;
			}
			if (antiThreshold <= antiScore) {
				return false;
			}
			streak = 0;
		}
	}
	return false;
}

/*
 * Evaluation algorithm with hashValue storage (minimize redundant work)
 */
inline bool evalSingle(const FastqRecord &rec, unsigned kmerSize, const BloomFilter &filter,
		double threshold, double antiThreshold, unsigned hashNum,
		vector<vector<size_t> > &hashValues)
{
	ReadsProcessor proc(kmerSize);
	size_t currentLoc = 0;
	double score = 0;
	unsigned antiScore = 0;
	unsigned streak = 0;
	while (rec.seq.length() >= currentLoc + kmerSize) {
		const unsigned char* currentSeq = proc.prepSeq(rec.seq, currentLoc);
		if (streak == 0) {
			if (currentSeq != NULL) {
				hashValues[currentLoc] = multiHash(currentSeq, hashNum, kmerSize);
				if (filter.contains(hashValues[currentLoc])) {
					score += 0.5;
					++streak;
					if (threshold <= score) {
						return true;
					}
				}
				else if (antiThreshold <= ++antiScore) {
					return false;
				}
				++currentLoc;
			} else {
				if (currentLoc > kmerSize) {
					currentLoc += kmerSize + 1;
					antiScore += kmerSize + 1;
				} else {
					++antiScore;
					++currentLoc;
				}
				if (antiThreshold <= antiScore) {
					return false;
				}
			}
		} else {
			if (currentSeq != NULL) {
				hashValues[currentLoc] = multiHash(currentSeq, hashNum, kmerSize);
				if (filter.contains(hashValues[currentLoc])) {
					++streak;
					score += 1 - 1 / (2 * streak);
					++currentLoc;

					if (threshold <= score) {
						return true;
					}
					continue;
				}
				else if (antiThreshold <= ++antiScore) {
					return false;
				}
			} else {
				currentLoc += kmerSize + 1;
				antiScore += kmerSize + 1;
			}
			if (streak < opt::streakThreshold) {
				++currentLoc;
			} else {
				currentLoc += kmerSize;
				antiScore += kmerSize;
			}
			if (antiThreshold <= antiScore) {
				return false;
			}
			streak = 0;
		}
	}
	return false;
}

/*
 * Evaluation algorithm with no hashValue storage (optimize speed for single queries)
 * Returns score and does not have a stopping threshold
 */
inline double evalSingleExhaust(const FastqRecord &rec, unsigned kmerSize,
		const BloomFilter &filter)
{
	ReadsProcessor proc(kmerSize);
	size_t currentLoc = 0;
	double score = 0;
	unsigned streak = 0;
	while (rec.seq.length() >= currentLoc + kmerSize) {
		const unsigned char* currentKmer = proc.prepSeq(rec.seq, currentLoc);
		if (streak == 0) {
			if (currentKmer != NULL) {
				if (filter.contains(currentKmer)) {
					score += 0.5;
					++streak;
				}
				++currentLoc;
			} else {
				currentLoc += kmerSize + 1;
			}
		} else {
			if (currentKmer != NULL) {
				if (filter.contains(currentKmer)) {
					++streak;
					score += 1 - 1 / (2 * streak);
					++currentLoc;
					continue;
				}
			} else {
				currentLoc += kmerSize + 1;
			}
			if (streak < opt::streakThreshold) {
				++currentLoc;
			} else {
				currentLoc += kmerSize;
			}
			streak = 0;
		}
	}
	return false;
}

///*
// * Evaluation algorithm with no hashValue storage (optimize speed for single queries)
// * Returns length when stopping threshold is met
// * Returns the length of the read where threshold was met
// */
//inline unsigned evalSingleLength(const FastqRecord &rec, unsigned kmerSize,
//		const BloomFilter &filter, double threshold, double antiThreshold)
//{
//	ReadsProcessor proc(kmerSize);
//	size_t currentLoc = 0;
//	double score = 0;
//	unsigned antiScore = 0;
//	unsigned streak = 0;
//	while (rec.seq.length() >= currentLoc + kmerSize) {
//		const unsigned char* currentKmer = proc.prepSeq(rec.seq, currentLoc);
//		if (streak == 0) {
//			if (currentKmer != NULL) {
//				if (filter.contains(currentKmer)) {
//					score += 0.5;
//					++streak;
//					if (threshold <= score) {
//						return currentLoc;
//					}
//				} else if (antiThreshold <= ++antiScore) {
//					return rec.seq.length() - kmerSize;
//				}
//				++currentLoc;
//			} else {
//				if (currentLoc > kmerSize) {
//					currentLoc += kmerSize + 1;
//					antiScore += kmerSize + 1;
//				} else {
//					++antiScore;
//					++currentLoc;
//				}
//				if (antiThreshold <= antiScore) {
//					return rec.seq.length() - kmerSize;
//				}
//			}
//		} else {
//			if (currentKmer != NULL) {
//				if (filter.contains(currentKmer)) {
//					++streak;
//					score += 1 - 1 / (2 * streak);
//
//					if (threshold <= score) {
//						return currentLoc;
//					}
//					++currentLoc;
//					continue;
//				} else if (antiThreshold <= ++antiScore) {
//					return rec.seq.length() - kmerSize;
//				}
//			} else {
//				currentLoc += kmerSize + 1;
//				antiScore += kmerSize + 1;
//			}
//			if (streak < opt::streakThreshold) {
//				++currentLoc;
//			} else {
//				currentLoc += kmerSize;
//				antiScore += kmerSize;
//			}
//			if (antiThreshold <= antiScore) {
//				return rec.seq.length() - kmerSize;
//			}
//			streak = 0;
//		}
//	}
//	return rec.seq.length() - kmerSize;
//}

/*
 * Core evaluation algorithm, with ability start evaluating sequence midway
 * Evaluation algorithm with hashValue storage (minimize redundant work)
 * Also stores if position has already been visited to minimize work
 * Takes in last position visited and score and updates them accordingly
 */
inline bool eval(const FastqRecord &rec, unsigned kmerSize,
		const BloomFilter &filter, double threshold, double antiThreshold,
		vector<bool> &visited, vector<vector<size_t> > &hashValues,
		unsigned &currentLoc, double &score, ReadsProcessor &proc)
{
	unsigned antiScore = 0;
	unsigned streak = 0;
	while (rec.seq.length() >= currentLoc + kmerSize) {

		//prepare hash values for filter

		//check if hash value is already generated
		if (hashValues[currentLoc].size() == 0) {
			if (!visited[currentLoc]) {
				const unsigned char* currentSeq = proc.prepSeq(rec.seq,
						currentLoc);
				if (currentSeq != NULL) {
					hashValues[currentLoc] = multiHash(currentSeq, filter.getHashNum(),
							kmerSize);
				}
				visited[currentLoc] = true;
			}
		}

		if (streak == 0) {
			if (hashValues[currentLoc].size() > 0) {
				if (filter.contains(hashValues[currentLoc])) {
					score += 0.5;
					++streak;
					if (threshold <= score) {
						++currentLoc;
						return true;
					}
				}
				else if (antiThreshold <= ++antiScore) {
					++currentLoc;
					return false;
				}
				++currentLoc;
			} else {
				if (currentLoc > kmerSize) {
					currentLoc += kmerSize + 1;
					antiScore += kmerSize + 1;
				} else {
					++antiScore;
					++currentLoc;
				}
				if (antiThreshold <= antiScore) {
					return false;
				}
			}
		} else {
			if (hashValues[currentLoc].size() > 0) {
				if (filter.contains(hashValues[currentLoc])) {
					++streak;
					score += 1 - 1 / (2 * streak);
					++currentLoc;

					if (threshold <= score) {
						return true;
					}
					continue;
				}
				else if (antiThreshold <= ++antiScore) {
					++currentLoc;
					return false;
				}
			} else {
				//if has non atcg character
				currentLoc += kmerSize + 1;
				antiScore += kmerSize + 1;
			}
			if (streak < opt::streakThreshold) {
				++currentLoc;
			} else {
				currentLoc += kmerSize;
				antiScore += kmerSize;
			}
			if (antiThreshold <= antiScore) {
				return false;
			}
			streak = 0;
		}
	}
	return false;
}

}
;

#endif /* SEQEVAL_H_ */
