#pragma once

#ifndef Server_H
#define Server_H

#include <list>
#include <cstring>

using namespace std;


class Server {
private:
	int index,
		numOfCpuA,
		numOfCpuB,
		memOfCpuA,
		memOfCpuB,
		hardwareCost,
		dayCost,
		emptyDay,
		weighted;
	int workDay = 0;
	std::string
		type;
	std::list<std::string>
		VMListOfCpuA,
		VMListOfCpuB;

public:


	int GetIndex() const { return index; }
	void SetIndex(int val) { index = val; }

	int GetNumOfCpuA() const { return numOfCpuA; }
	void SetNumOfCpuA(int val) { numOfCpuA = val; }

	int GetNumOfCpuB() const { return numOfCpuB; }
	void SetNumOfCpuB(int val) { numOfCpuB = val; }

	int GetMemOfCpuA() const { return memOfCpuA; }
	void SetMemOfCpuA(int val) { memOfCpuA = val; }

	int GetMemOfCpuB() const { return memOfCpuB; }
	void SetMemOfCpuB(int val) { memOfCpuB = val; }

	int GetHardwareCost() const { return hardwareCost; }
	void SetHardwareCost(int val) { hardwareCost = val; }

	int GetDayCost() const { return dayCost; }
	void SetDayCost(int val) { dayCost = val; }

	int GetWorkDay() const { return workDay; }
	void SetWorkDay(int val) { workDay = val; }

	int GetEmptyDay() const { return emptyDay; }
	void SetEmptyDay(int val) { emptyDay = val; }

	int GetWeighted() const { return weighted; }
	void SetWeighted(int val) { weighted = val; }

	std::string GetType() const { return type; }
	void SetType(std::string val) { type = val; }

	std::list<std::string> GetVMListOfCpuA() const { return VMListOfCpuA; }
	void SetVMListOfCpuA(std::string val) { VMListOfCpuA.push_back(val); } //change

	std::list<std::string> GetVMListOfCpuB() const { return VMListOfCpuB; }
	void SetVMListOfCpuB(std::string val) { VMListOfCpuB.push_back(val); }

	void DelVMListOfCpuA(std::string val) { VMListOfCpuA.remove(val); }
	void DelVMListOfCpuB(std::string val) { VMListOfCpuB.remove(val); }
};

#endif
