
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <algorithm> 
#include <cstring>
#include "Server.h"
#include "VirtualMachine.h"
#include <cmath>


using namespace std;


std::unordered_map<std::string, Server> serverList;
std::unordered_map<std::string, VirtualMachine> VMList;

std::unordered_map<std::string, VirtualMachine> VMServed; //1
std::unordered_map<std::string, VirtualMachine*> VMServedAddr; //1


std::unordered_map<std::string, std::unordered_map<int, Server>> allServerBought;
std::unordered_map<std::string, std::unordered_map<int, Server*>> allServerBoughtAddr;


std::map<int, Server*> allServerBoughtOnlyAddr;
std::list<Server*> allServerBoughtOnlyAddrSeq;
std::list<Server*> allServerBoughtOnlyAddrList;
std::map<int, Server*> ServersNewBuyAddr;

std::list<std::string> migrationResult;
std::list<std::string> delList, VMIDNewDay;

//分割函数
vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if ("" == str) return res;
	//先将要切割的字符串从string类型转换为char*类型  
	char* strs = new char[str.length() + 1]; //不要忘了  
	strcpy(strs, str.c_str());

	char* d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char* p = strtok(strs, d);
	while (p) {
		string s = p; //分割得到的字符串转换为string类型  
		res.push_back(s); //存入结果数组  
		p = strtok(NULL, d);
	}

	return res;

}


void VMCopy(VirtualMachine& vmNew, VirtualMachine& vmOld) {

	vmNew.SetMemory(vmOld.GetMemory());
	vmNew.SetNumOfCpu(vmOld.GetNumOfCpu());
	vmNew.SetID(vmOld.GetID());
	vmNew.SetType(vmOld.GetType());
	vmNew.SetIsDouble(vmOld.GetIsDouble());
	vmNew.SetBestServerType(vmOld.GetBestServerType());
	vmNew.SetWeighted(vmOld.GetWeighted());
	vmNew.SetServerTypeForBig(vmOld.GetServerTypeForBig());
}

void serverCopy(Server& serverNew, Server& ServerOld) {
	serverNew.SetType(ServerOld.GetType());
	serverNew.SetNumOfCpuA(ServerOld.GetNumOfCpuA());
	serverNew.SetNumOfCpuB(ServerOld.GetNumOfCpuB());
	serverNew.SetMemOfCpuA(ServerOld.GetMemOfCpuA());
	serverNew.SetMemOfCpuB(ServerOld.GetMemOfCpuB());
	serverNew.SetHardwareCost(ServerOld.GetHardwareCost());
	serverNew.SetDayCost(ServerOld.GetDayCost());
	serverNew.SetWeighted(ServerOld.GetWeighted());
}


//匹配虚拟机的最佳服务器 方法一
string selectBFServer(VirtualMachine vm, std::unordered_map<string, Server> serverList) {
	int vmCpu = vm.GetNumOfCpu();
	int vmMem = vm.GetMemory();
	double weighted;
	//double tmp1, tmp2;
	double minWeight =1000000000;

	std::string bestType;
	std::string type = vm.GetType();

	bool isDouble = vm.GetIsDouble();
	for (auto it = serverList.begin(); it != serverList.end(); it++)
	{
		Server server = it->second;
		int serverCpu = server.GetNumOfCpuA() + server.GetNumOfCpuB();
		int serverMem = server.GetMemOfCpuA() + server.GetMemOfCpuB();
		int hardwareCost = server.GetHardwareCost();
		int dayCost = server.GetDayCost();
		double cpu_D = serverCpu;
		double mem_D = serverMem;

		if (isDouble && (vmMem > serverMem / 4 || vmCpu > serverCpu / 4)) continue;
		else if (!isDouble && (vmMem > serverMem/2 || vmCpu > serverCpu/2)) continue;
		
		weighted = 5000 * (vmCpu/cpu_D + vmMem/mem_D) + 0.0005* hardwareCost + 50 * dayCost;
		//weighted = double(85* cpu_D + 40 * mem_D  + 0.0005*hardwareCost + 15* dayCost);
	
		if (weighted < minWeight) {
			minWeight = weighted;
			bestType = server.GetType();
		}
	}
	return bestType;

}

string selectServerBigReq(VirtualMachine vm, std::unordered_map<string, Server> serverList)
{
	int vmCpu = vm.GetNumOfCpu();
	int vmMem = vm.GetMemory();
	double weighted;
	//double tmp1, tmp2;
	double minWeight = 1000000;

	std::string bestType;
	std::string type = vm.GetType();

	bool isDouble = vm.GetIsDouble();
	for (auto it = serverList.begin(); it != serverList.end(); it++)
	{
		Server server = it->second;
		int serverCpu = server.GetNumOfCpuA() + server.GetNumOfCpuB();
		int serverMem = server.GetMemOfCpuA() + server.GetMemOfCpuB();
		int hardwareCost = server.GetHardwareCost();
		int dayCost = server.GetDayCost();
		double cpu_D = serverCpu;
		double mem_D = serverMem;

		if (isDouble && (vmMem > serverMem / 4 || vmCpu > serverCpu / 4)) continue;
		else if (!isDouble && (vmMem > serverMem/2 || vmCpu > serverCpu/2)) continue;

		weighted =double( min(cpu_D/vmCpu , mem_D/vmMem) +  dayCost);
		

		if (weighted < minWeight) {
			minWeight = weighted;
			bestType = server.GetType();
		}
	}
	return bestType;
}


int VMweighted(int cpuNum, int mem) {
	return cpuNum * mem + cpuNum +mem;
}

int ServerWeighted(Server& server) {
	int cpu_num = server.GetNumOfCpuA() + server.GetNumOfCpuB();
	int mem = server.GetMemOfCpuA() + server.GetMemOfCpuB();
	int hardwareCost = server.GetHardwareCost();
	int dayCost = server.GetDayCost();

	return 85 * cpu_num + 40 * mem + 15 * dayCost;// +0.0005 * hardwareCost ;
}

bool placeForMigra(VirtualMachine* vm, int index, std::string& result) {
	int numOfCpu = (*vm).GetNumOfCpu(),
		memory = (*vm).GetMemory();
	std::string ID = (*vm).GetID();
	std::string serverType = (*(*vm).GetServer()).GetType();
	bool isDouble = (*vm).GetIsDouble();
	bool flag = false;

	for (auto it = allServerBoughtOnlyAddrSeq.begin(); it != allServerBoughtOnlyAddrSeq.end(); it++)
	{
		Server* server = *it;
		int index2 = (*server).GetIndex();
		if(index2 >= index) break;
		if((*server).GetType() == serverType) continue;
		if (isDouble) { //双节点
			if (((*server).GetNumOfCpuA() >= (numOfCpu / 2))
				&& ((*server).GetNumOfCpuB() >= (numOfCpu / 2))
				&& ((*server).GetMemOfCpuA() >= (memory / 2))
				&& ((*server).GetMemOfCpuB() >= (memory / 2))
				)
			{
				(*server).SetNumOfCpuA((*server).GetNumOfCpuA() - (numOfCpu / 2));
				(*server).SetNumOfCpuB((*server).GetNumOfCpuB() - (numOfCpu / 2));
				(*server).SetMemOfCpuA((*server).GetMemOfCpuA() - (memory / 2));
				(*server).SetMemOfCpuB((*server).GetMemOfCpuB() - (memory / 2));
				(*server).SetVMListOfCpuA(ID);
				(*server).SetVMListOfCpuB(ID);
				(*vm).SetServer(server);
				
				result = "(";
				result = result + ID + ", " + std::to_string(index2) + ")";

				(*vm).SetPlaceAt(2);
				flag = true;
				break;
			}
			else continue;
		}
		else {  //单节点
			if (((*server).GetNumOfCpuA() >= numOfCpu)
				&& ((*server).GetMemOfCpuA() >= memory)
				) {
				(*server).SetNumOfCpuA((*server).GetNumOfCpuA() - numOfCpu);
				(*server).SetMemOfCpuA((*server).GetMemOfCpuA() - memory);
				(*server).SetVMListOfCpuA(ID);
				(*vm).SetServer(server);
				(*vm).SetPlaceAt(0);

				result = "(";
				result = result + ID + ", " + std::to_string(index2) + ", A)";
				flag = true;
				break;
			}
			else if (((*server).GetNumOfCpuB() >= numOfCpu) && ((*server).GetMemOfCpuB() >= memory)) {
				(*server).SetNumOfCpuB((*server).GetNumOfCpuB() - numOfCpu);
				(*server).SetMemOfCpuB((*server).GetMemOfCpuB() - memory);
				(*server).SetVMListOfCpuB(ID);
				(*vm).SetServer(server);
				(*vm).SetPlaceAt(1);
				result = "(";
				result = result + ID + ", " + std::to_string(index2) + ", B)";
				flag = true;

				break;
			}
			else continue;
		}

		if (flag == true) break;
	}
	return flag;
}

bool placeVMWithBought(VirtualMachine& vm)
{
	int numOfCpu = (vm).GetNumOfCpu(),
		memory = (vm).GetMemory();
	std::string ID = (vm).GetID();
	bool isDouble = (vm).GetIsDouble();
	bool flag = false;

	for(auto it = allServerBoughtOnlyAddrList.begin(); it != allServerBoughtOnlyAddrList.end(); it++)
	{
			Server* server = *it;

			if (isDouble) { //双节点
				if (((*server).GetNumOfCpuA() >= (numOfCpu / 2))
					&& ((*server).GetNumOfCpuB() >= (numOfCpu / 2))
					&& ((*server).GetMemOfCpuA() >= (memory / 2))
					&& ((*server).GetMemOfCpuB() >= (memory / 2))
					)
				{
					(*server).SetNumOfCpuA((*server).GetNumOfCpuA() - (numOfCpu / 2));
					(*server).SetNumOfCpuB((*server).GetNumOfCpuB() - (numOfCpu / 2));
					(*server).SetMemOfCpuA((*server).GetMemOfCpuA() - (memory / 2));
					(*server).SetMemOfCpuB((*server).GetMemOfCpuB() - (memory / 2));
					(*server).SetVMListOfCpuA(ID);
					(*server).SetVMListOfCpuB(ID);
					(vm).SetServer(server);
					//(*vm).SetServerIndex(it->second.size());
					(vm).SetPlaceAt(2);

					flag = true;
					break;
				}
				else continue;
			}
			else {  //单节点
				if (((*server).GetNumOfCpuA() >= numOfCpu)
					&& ((*server).GetMemOfCpuA() >= memory)
					) {
					(*server).SetNumOfCpuA((*server).GetNumOfCpuA() - numOfCpu);
					(*server).SetMemOfCpuA((*server).GetMemOfCpuA() - memory);
					(*server).SetVMListOfCpuA(ID);
					(vm).SetServer(server);
					(vm).SetPlaceAt(0);
					flag = true;
					break;
				}
				else if (((*server).GetNumOfCpuB() >= numOfCpu) && ((*server).GetMemOfCpuB() >= memory)) {
					(*server).SetNumOfCpuB((*server).GetNumOfCpuB() - numOfCpu);
					(*server).SetMemOfCpuB((*server).GetMemOfCpuB() - memory);
					(*server).SetVMListOfCpuB(ID);
					(vm).SetServer(server);
					(vm).SetPlaceAt(1);
					flag = true;

					break;
				}
				else continue;
			}

		if (flag == true) break;
	}
	return flag;
}

void placeVMWithAll(VirtualMachine& vm,
	std::unordered_map<std::string, 
	std::unordered_map<int, Server*>>& ServersNewDayAddr,
	std::unordered_map<std::string, int> vmBigReqType
)
{
	int numOfCpu = (vm).GetNumOfCpu(),
		memory = (vm).GetMemory();
	std::string ID = (vm).GetID();
	std::string type = (vm).GetType();
	bool isDouble = (vm).GetIsDouble();
	bool flag = false;
	Server server;
	std::string selectType;
	if ((vmBigReqType.size()!=0) && vmBigReqType.count(type) > 0) {
		selectType = (vm).GetServerTypeForBig();
	}
	else selectType = (vm).GetBestServerType();

	Server tmp = serverList[selectType];
	serverCopy(server, tmp);


	if (isDouble && (server.GetNumOfCpuA() >= numOfCpu / 2) && (server.GetNumOfCpuB() >= numOfCpu / 2)
		&& (server.GetMemOfCpuA() >= memory / 2) && (server.GetMemOfCpuB() >= memory / 2)
		)
	{
		server.SetNumOfCpuA(server.GetNumOfCpuA() - numOfCpu / 2);
		server.SetNumOfCpuB(server.GetNumOfCpuB() - numOfCpu / 2);
		server.SetMemOfCpuA(server.GetMemOfCpuA() - memory / 2);
		server.SetMemOfCpuB(server.GetMemOfCpuB() - memory / 2);
		server.SetVMListOfCpuA(ID);
		server.SetVMListOfCpuB(ID);

		(vm).SetPlaceAt(2);
	}
	else {  //单节点
		if ((server.GetNumOfCpuA() >= numOfCpu) && (server.GetMemOfCpuA() >= memory)) {
			server.SetNumOfCpuA(server.GetNumOfCpuA() - numOfCpu);
			server.SetMemOfCpuA(server.GetMemOfCpuA() - memory);
			server.SetVMListOfCpuA(ID);

			(vm).SetPlaceAt(0);
		}
		else if ((server.GetNumOfCpuB() >= numOfCpu) && (server.GetMemOfCpuB() >= memory)) {
			server.SetNumOfCpuB(server.GetNumOfCpuB() - numOfCpu);
			server.SetMemOfCpuB(server.GetMemOfCpuB() - memory);
			server.SetVMListOfCpuB(ID);
			(vm).SetPlaceAt(1);
		}
	}

	if (allServerBought.find(selectType) == allServerBought.end()) {
		allServerBought[selectType][0] = server;
		allServerBoughtAddr[selectType][0] = &(allServerBought[selectType][0]);
		allServerBoughtOnlyAddrList.push_back(&(allServerBought[selectType][0]));
		allServerBoughtOnlyAddrSeq.push_back(&(allServerBought[selectType][0]));
		//------------------------------
		allServerBoughtOnlyAddr[allServerBoughtOnlyAddr.size()] = &(allServerBought[selectType][0]);
		ServersNewBuyAddr[ServersNewBuyAddr.size()] = &(allServerBought[selectType][0]);


		(vm).SetServer(&(allServerBought[selectType][0]));
		ServersNewDayAddr[selectType][0] = &(allServerBought[selectType][0]);



	}
	else 
	{
		int size = allServerBought[selectType].size();
		allServerBought[selectType][size] = server;
		allServerBoughtAddr[selectType][size] = &(allServerBought[selectType][size]);

		allServerBoughtOnlyAddr[allServerBoughtOnlyAddr.size()] = &(allServerBought[selectType][size]);
		allServerBoughtOnlyAddrList.push_back(&(allServerBought[selectType][size]));
		allServerBoughtOnlyAddrSeq.push_back(&(allServerBought[selectType][size]));
		ServersNewBuyAddr[ServersNewBuyAddr.size()] = &(allServerBought[selectType][size]);

		(vm).SetServer(&(allServerBought[selectType][size]));

		if (ServersNewDayAddr.find(selectType) == ServersNewDayAddr.end()) {
			ServersNewDayAddr[selectType][0] = &(allServerBought[selectType][size]);
		}
		else {
			int size1 = ServersNewDayAddr[selectType].size();
			ServersNewDayAddr[selectType][size1] = &(allServerBought[selectType][size]);
		}

	}


}




int migration(std::list<std::string>& migrationResult,
	std::map<int, Server*>& allServerBoughtOnlyAddr,
	std::map<int, Server*>& ServersNewBuyAddr
)
{
	int times = 0;
	int limit_size = (VMServed.size() * 5 / 1000) - 1;
	std::string result;

	if (times >= limit_size) return times;

	int	ServersNewBuySize = ServersNewBuyAddr.size();
	int allServerBoughtSize = allServerBoughtOnlyAddr.size();

	for (int i = 0; i < allServerBoughtSize; i++) {
		Server* server = allServerBoughtOnlyAddr[i];
		std::list<std::string> listA = (*server).GetVMListOfCpuA();
		int index0fServer = (*server).GetIndex();
		for (auto it_vm = listA.begin(); it_vm != listA.end(); it_vm++) {
			VirtualMachine* vm = VMServedAddr[(*it_vm)];
			if (placeForMigra(vm, index0fServer, result)) {
				times++;
				migrationResult.push_back(result);
				bool isDouble = (*vm).GetIsDouble();
				std::string ID = (*vm).GetID();
				int numOfCpu = (*vm).GetNumOfCpu(),
					memory = (*vm).GetMemory();
				if (isDouble) { //双节点
					(*server).SetNumOfCpuA((*server).GetNumOfCpuA() + (numOfCpu / 2));
					(*server).SetNumOfCpuB((*server).GetNumOfCpuB() + (numOfCpu / 2));
					(*server).SetMemOfCpuA((*server).GetMemOfCpuA() + (memory / 2));
					(*server).SetMemOfCpuB((*server).GetMemOfCpuB() + (memory / 2));
					(*server).DelVMListOfCpuA(ID);
					(*server).DelVMListOfCpuB(ID);
				}
				else 
				{  //单节点
					(*server).SetNumOfCpuA((*server).GetNumOfCpuA() + numOfCpu);
					(*server).SetMemOfCpuA((*server).GetMemOfCpuA() + memory);
					(*server).DelVMListOfCpuA(ID);
				}

				if (times >= limit_size) return times;
			}
			
		}
		

		std::list<std::string> listB = (*server).GetVMListOfCpuB();

		for (auto it_vm = listB.begin(); it_vm != listB.end(); it_vm++) {
			VirtualMachine* vm = VMServedAddr[(*it_vm)];
			if (placeForMigra(vm, index0fServer, result)) {
				times++;
				migrationResult.push_back(result);
				bool isDouble = (*vm).GetIsDouble();
				std::string ID = (*vm).GetID();
				int numOfCpu = (*vm).GetNumOfCpu(),
					memory = (*vm).GetMemory();
				if (isDouble) { //双节点
					(*server).SetNumOfCpuA((*server).GetNumOfCpuA() + (numOfCpu / 2));
					(*server).SetNumOfCpuB((*server).GetNumOfCpuB() + (numOfCpu / 2));
					(*server).SetMemOfCpuA((*server).GetMemOfCpuA() + (memory / 2));
					(*server).SetMemOfCpuB((*server).GetMemOfCpuB() + (memory / 2));
					(*server).DelVMListOfCpuA(ID);
					(*server).DelVMListOfCpuB(ID);
				}
				else
				{  //单节点
					(*server).SetNumOfCpuB((*server).GetNumOfCpuB() + numOfCpu);
					(*server).SetMemOfCpuB((*server).GetMemOfCpuB() + memory);
					(*server).DelVMListOfCpuB(ID);
				}

				if (times >= limit_size) return times;
			}

		}
	}

	return times;

}





/*************************************************************************/
/******************************问题求解***********************************/


int main()
{

	int index = 0;
	std::string days, infolen;


	getline(cin, infolen);
	//记录全部的servers
	for (int i = 0; i < stoi(infolen); i++) {

		std::string serverTXT;
		//std::string tmp1, tmp2, tmp3, tmp4;
		VirtualMachine vm;

		getline(cin, serverTXT);
		vector<std::string> serverInfo = split(serverTXT, ",");

		Server server;
		std::string type = serverInfo[0].substr(1);
		server.SetType(type);
		server.SetNumOfCpuA(stoi(serverInfo[1].substr(1)) / 2);
		server.SetNumOfCpuB(stoi(serverInfo[1].substr(1)) / 2);

		server.SetMemOfCpuA(stoi(serverInfo[2].substr(1)) / 2);
		server.SetMemOfCpuB(stoi(serverInfo[2].substr(1)) / 2);

		server.SetHardwareCost(stoi(serverInfo[3].substr(1)));
		server.SetDayCost(stoi(serverInfo[4].substr(1, serverInfo[4].size() - 2)));
		server.SetWeighted(ServerWeighted(server));

		serverList[type] = server;

	}

	getline(cin, infolen);

	//记录所有的VM
	for (int i = 0; i < stoi(infolen); i++)
	{
		std::string VMTXT;

		VirtualMachine vm;
		getline(cin, VMTXT);
		vector<std::string> VMInfo = split(VMTXT, ",");


		int isDouble = stoi(VMInfo[3].substr(1, VMInfo[3].size() - 1));
		std::string type = VMInfo[0].substr(1);

		int numOfCpu = stoi(VMInfo[1].substr(1)),
			mem = stoi(VMInfo[2].substr(1));

		vm.SetType(type);
		vm.SetMemory(mem);
		vm.SetNumOfCpu(numOfCpu);

		if (isDouble)
		{
			vm.SetIsDouble(true);
		}
		else
		{
			vm.SetIsDouble(false);
		}

		vm.SetBestServerType(selectBFServer(vm, serverList)); //匹配最好的服务器  
		vm.SetServerTypeForBig(selectServerBigReq(vm, serverList));
		vm.SetWeighted(VMweighted(numOfCpu, mem));
		VMList[type] = vm;

	}


	getline(cin, days);
	//cin.get();
	for (int day = 0; day < stoi(days); day++)
	{
		int migra_times;
		migrationResult.clear();
		if(delList.size() > 3000000)
			migra_times = migration(migrationResult, allServerBoughtOnlyAddr, ServersNewBuyAddr);
		else migra_times = 0;
		std::unordered_map<std::string, VirtualMachine> VMNewReq;  //1
		std::unordered_map<std::string, VirtualMachine*> VMNewReqAddr;  //1
		std::list<std::string> ServersTypeNewDay;
		std::map<int, std::string> VMNewReqAddrSeq;

		std::unordered_map<std::string, std::unordered_map<int, Server*>> ServersNewDayAddr; //1

		std::unordered_map<std::string, int> vmNumList;
		std::unordered_map<std::string, int> vmBigReqType;



		delList.clear();

		ServersNewBuyAddr.clear();

		std::string reqlen;
		getline(cin, reqlen);

		for (int i = 0; i < stoi(reqlen); i++) {

			vector<std::string> reqinfo;
			std::string tmp1;
			getline(cin, tmp1);
			vector<std::string> reqVec = split(tmp1, ",");

			if (reqVec.size() == 3) {

				VirtualMachine VMNew;
				std::string type = reqVec[1].substr(1);
				std::string ID = reqVec[2].substr(1, reqVec[2].size() - 2);

				vmNumList[type] = vmNumList[type] + 1;

				VMCopy(VMNew, VMList[type]);
				VMNew.SetID(ID);
				VMIDNewDay.push_back(ID);
				VMNewReq[ID] = VMNew;
				VMNewReqAddr[ID] = &(VMNewReq[ID]);
				VMNewReqAddrSeq[i] = ID;
			}
			else {
				delList.push_back(reqVec[1].substr(1, reqVec[1].size() - 2));
			}
			for (auto it = vmNumList.begin(); it != vmNumList.end(); it++) {
				if ((it->second )> 10)
					vmBigReqType[it->first] = 1;
			}
		}


		VMIDNewDay.sort([&](std::string a, std::string b) {
			return VMNewReq[a].GetWeighted() > VMNewReq[b].GetWeighted();
		});

		for(auto it = allServerBoughtOnlyAddr.begin(); it != allServerBoughtOnlyAddr.end(); it++) 
		{
			Server* server = it->second;
			(*server).SetWeighted(ServerWeighted(*server));
		}

		allServerBoughtOnlyAddrList.sort([&](Server* a, Server* b) {
			return (*a).GetWeighted() < (*b).GetWeighted();
		});

		int de_i = 0;
		for (std::list<std::string>::iterator it = VMIDNewDay.begin(); it != VMIDNewDay.end(); it++)
		{
			de_i++;
			std::string ID = *it;
			VirtualMachine* VM = VMNewReqAddr[ID];

			if (!placeVMWithBought(*VM))
				placeVMWithAll(*VM, ServersNewDayAddr, vmBigReqType);

			VMServed[ID] = *VM;
			VMServedAddr[ID] = &(VMServed[ID]);

		}

		//给每个新服务器赋index
		for (auto it = ServersNewDayAddr.begin(); it != ServersNewDayAddr.end(); it++) {

			for (auto list_it = it->second.begin(); list_it != it->second.end(); list_it++) {
				Server* server = list_it->second;
				(*list_it->second).SetIndex(index++);
			}
		}


		//delete
		for (auto it = delList.begin(); it != delList.end(); it++)
		{
			std::string ID = *it;
			VirtualMachine* vm = VMServedAddr[ID];

			int placeAt = (*vm).GetPlaceAt();
			int memory = (*vm).GetMemory();
			int numOfCpu = (*vm).GetNumOfCpu();
			Server* atServer = (*vm).GetServer();
			if (placeAt == 2) {
				(*atServer).DelVMListOfCpuA(ID);
				(*atServer).DelVMListOfCpuB(ID);
				(*atServer).SetMemOfCpuA((*atServer).GetMemOfCpuA() + memory / 2);
				(*atServer).SetMemOfCpuB((*atServer).GetMemOfCpuB() + memory / 2);
				(*atServer).SetNumOfCpuA((*atServer).GetNumOfCpuA() + numOfCpu / 2);
				(*atServer).SetNumOfCpuB((*atServer).GetNumOfCpuB() + numOfCpu / 2);
			}
			else if (placeAt == 1) {
				(*atServer).DelVMListOfCpuB(ID);
				(*atServer).SetMemOfCpuB((*atServer).GetMemOfCpuB() + memory);
				(*atServer).SetNumOfCpuB((*atServer).GetNumOfCpuB() + numOfCpu);
			}
			else if (placeAt == 0) {
				(*atServer).DelVMListOfCpuA(ID);
				(*atServer).SetMemOfCpuA((*atServer).GetMemOfCpuA() + memory);
				(*atServer).SetNumOfCpuA((*atServer).GetNumOfCpuA() + numOfCpu);
			}
			VMServedAddr.erase(ID);
			VMServed.erase(ID);

			
		}


		int newServerNum = ServersNewDayAddr.size();

		cout << "(purchase, " << newServerNum << ")" << endl;

		for (auto it = ServersNewDayAddr.begin(); it != ServersNewDayAddr.end(); it++) {
			std::string type = it->first;
			cout << "(" << type << ", " << ServersNewDayAddr[type].size() << ")" << endl;
		}

		cout << "(migration, " << migra_times << ")" << endl;
		//cout << "(migration, " <<  "0)" << endl;
		
		for (auto it = migrationResult.begin(); it != migrationResult.end(); it++) {
			cout << *it << endl;
		}
		


		for (auto it = VMNewReqAddrSeq.begin(); it != VMNewReqAddrSeq.end(); it++) {
			std::string ID = it->second;
			VirtualMachine* vm = VMNewReqAddr[ID];
			int index1 = (*((*vm).GetServer())).GetIndex();
			Server* server = (*vm).GetServer();
			int placeAt = (*vm).GetPlaceAt();
			if (placeAt == 0) {
				cout << "(" << index1 << ", A)" << endl;
			}
			else if (placeAt == 1) {
				cout << "(" << index1 << ", B)" << endl;

			}
			else {
				cout << "(" << index1 << ")" << endl;
			}
		}


		for (auto it = allServerBoughtAddr.begin(); it != allServerBoughtAddr.end(); it++) {
			for (auto it_2 = it->second.begin(); it_2 != it->second.end(); it_2++) {
				Server* server = it_2->second;
				if ((*server).GetVMListOfCpuA().size() != 0 || (*server).GetVMListOfCpuB().size() != 0) {
					(*server).SetWorkDay((*server).GetWorkDay() + 1);
				}
			}

		}
		VMNewReqAddrSeq.clear();
		VMNewReq.clear();  
		VMNewReqAddr.clear();  
		ServersNewDayAddr.clear(); 
		//delList.clear();
		VMIDNewDay.clear();
		vmNumList.clear();
		vmBigReqType.clear();

	}


	int totalCost = 0;
	for (auto it = allServerBought.begin(); it != allServerBought.end(); it++) {
		for (auto it_2 = it->second.begin(); it_2 != it->second.end(); it_2++) {
			Server server = it_2->second;
			totalCost += server.GetHardwareCost();
			totalCost += server.GetDayCost() * server.GetWorkDay();
		}

	}

	return 0;

}
