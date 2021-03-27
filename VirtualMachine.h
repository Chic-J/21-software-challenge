#pragma once
#ifndef VirtualMachine_H
#define VirtualMachine_H

#include<list>
#include<cstring>
#include "Server.h"

using namespace std;


class VirtualMachine {
private:
	std::string
		ID,
		type,
		bestServerType,
		serverTypeForBig;
	int
		numOfCpu,
		memory,
		dayBegin,
		dayEnd,
		placeAt,
		weighted,
		serverIndex;
	Server* server;
	bool isDouble;


public:


	std::string GetID() const { return ID; }
	void SetID(std::string val) { ID = val; }

	std::string GetType() const { return type; }
	void SetType(std::string val) { type = val; }

	std::string GetBestServerType() const { return bestServerType; }
	void SetBestServerType(std::string val) { bestServerType = val; }

	std::string GetServerTypeForBig() const { return serverTypeForBig; }
	void SetServerTypeForBig(std::string val) { serverTypeForBig = val; }

	int GetNumOfCpu() const { return numOfCpu; }
	void SetNumOfCpu(int val) { numOfCpu = val; }

	int GetMemory() const { return memory; }
	void SetMemory(int val) { memory = val; }

	int GetDayBegin() const { return dayBegin; }
	void SetDayBegin(int val) { dayBegin = val; }

	int GetDayEnd() const { return dayEnd; }
	void SetDayEnd(int val) { dayEnd = val; }

	int GetPlaceAt() const { return placeAt; }
	void SetPlaceAt(int val) { placeAt = val; }

	int GetWeighted() const { return weighted; }
	void SetWeighted(int val) { weighted = val; }

	int GetServerIndex() const { return serverIndex; }
	void SetServerIndex(int val) { serverIndex = val; }

	bool GetIsDouble() const { return isDouble; }
	void SetIsDouble(bool val) { isDouble = val; }

	Server* GetServer() { return server; }
	void SetServer(Server* val) { server = val; }

};

#endif