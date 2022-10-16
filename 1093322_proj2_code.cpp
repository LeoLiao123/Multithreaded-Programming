#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <pthread.h>
#include <unistd.h> 
#include <sys/types.h>  
#include <math.h> 
#include <iomanip> 
#include <time.h> 
#include <sstream>
using namespace std;
map<string,int>words, WordIndex;
vector<string> docID;
vector<vector<int>> vectorArr;
vector<double> averageCos;
pid_t tid;
int wait = 0, WordCount = 0;
void *childThread(void *arg) {
	double begin = clock(), end;
	int n = *(int *)arg;
	tid = gettid();
	wait++;
	while (1) {
		if (wait == 2) {
			cout << "[TID=" << tid << "] DocID:" << docID[n] << "[" << vectorArr[n][0];
			for (int i = 1; i < WordCount; i++)
				cout << "," << vectorArr[n][i];
			cout << "]" << endl;
			wait++;
			break;
		}
	}
	double average = 0.0;
	for (long unsigned int i = 0; i < docID.size(); i++) {
		if (i != (long unsigned int)n) {
			int upCount = 0;
			double underCount1 = 0.0, underCount2 = 0.0, sim = 0.0;
			for (int j = 0; j < WordCount; j++) {
				upCount += vectorArr[i][j] * vectorArr[n][j];
				underCount1 += vectorArr[i][j] * vectorArr[i][j];
				underCount2 += vectorArr[n][j] * vectorArr[n][j];
			}
			sim = upCount / (sqrt(underCount1)*sqrt(underCount2));
			cout << "[TID=" << tid << "] cosine(" << docID[n] << "," << docID[i] << ")=" << fixed << setprecision(4) << sim << endl;
			average += sim;
		}
	}
	average /= ((double)docID.size() - 1.0);
	averageCos.push_back(average);
	end = clock();
	cout << "[TID=" << tid << "] Avg_cosine: " << average << endl;
	cout << "[TID=" << tid << "] CPU time: " << fixed << setprecision(0) << (end - begin) << "ms" << endl;
	return NULL;
}
int main(int argc, char *argv[]) {
	double begin = clock(), end;
	ifstream file;
	vector<string> docArr;
	file.open(argv[1]);
	string str;
	int DocCount = 0;
	while (getline(file, str)) {
		if (str != "") {
			docID.push_back(str);
			getline(file, str);
			docArr.push_back(str);
			stringstream ss(str);
			string temp;
			while (ss >> temp) {
				if (words[temp] == 0) {
					words[temp] = 1;
					WordIndex[temp] = WordCount;
					WordCount++;
				}
			}
			DocCount++;
		}
		else
			break;
	}
	vectorArr.assign(DocCount + 1, vector<int>());
	for (int i = 0; i < DocCount; i++) {
		vector<int> tempVector(WordCount + 1);
		stringstream ss(docArr[i]);
		string temp;
		while (ss >> temp)
			tempVector[WordIndex[temp]]++;
		vectorArr[i] = tempVector;
	}
	pthread_t thread[DocCount];
	for (int i = 0; i < DocCount; i++) {
		pthread_create(&thread[i], NULL, childThread, &i);
		while (1) {
			if (wait==1) {
				cout << "[Main thread]: create TID:" << tid << ",DocID:" << docID[i] << endl;
				wait++;
				break;
			}
		}
		pthread_join(thread[i], NULL);
		wait = 0;
	}
	double max= -2;
	int KeyID;
	for (int i = 0; i < DocCount; i++) {
		if (max < averageCos[i]) {
			KeyID = i;
			max = averageCos[i];
		}
	}
	end = clock();
	cout << "[Main thread] KeyDocID:" << docID[KeyID] << " Highest Average Cosine:" << fixed << setprecision(4) << max << endl;
	cout << "[Main thread] CPU time: " << fixed << setprecision(0) << (end - begin) << "ms" << endl;
}