#include "CF18SyncTrigger.h"
#include "httplib.h"  // cpp-httplib库 https://github.com/yhirose/cpp-httplib
#include "nlohmann/json.hpp"

httplib::Client * cliCf18 = nullptr;
#define CHECK_HADNLE(device_handle) if (device_handle == nullptr) return false; 
CF18SDK_API bool CF18Open(std::string host, int port)
{
	cliCf18 = new httplib::Client(host, port);
	auto res = cliCf18->Get("/");
	if (res == nullptr) {
		return false;
	}
	return true;
}

CF18SDK_API bool CF18Close() {
	if (cliCf18 != nullptr) {
		delete cliCf18;
		cliCf18 = nullptr;
	}
	return true;
}

CF18SDK_API bool GetVersionData(std::string &arm_version, std::string &fpga_version)
{
	auto res = cliCf18->Get("/api/v1/version_info");
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		arm_version = subData["armVersion"].dump();
		fpga_version = subData["fpgaVersion"].dump();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetChannelSwitch(uint8_t mode, uint8_t channel, uint8_t work)
{
	nlohmann::json jsonData;
	jsonData["mode"] = mode;
	jsonData["channel"] = channel;
	jsonData["work"] = work;

	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/switch", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}

CF18SDK_API bool GetChnStatus(int chn, CmdChnStatus &chnIno)
{
	if (!cliCf18)
	{
		return false;
	}
	auto res = cliCf18->Get("/api/v1/chn_info?id="+std::to_string(chn));
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		chnIno.channel = subData["channel"].get<int>();
		chnIno.single = subData["single"].get<int>();
		chnIno.loopSwitch = subData["loopSwitch"].get<int>();
		chnIno.globalCtl = subData["globalCtl"].get<int>();
		chnIno.outputReset = subData["outputReset"].get<int>();
		chnIno.currentStatus = subData["currentStatus"].get<int>();
		chnIno.circleCnt = subData["circleCnt"].get<int>();
		chnIno.delayPs = subData["delayPs"].get<int>();
		chnIno.outputCnt = subData["outputCnt"].get<int>();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetChnStatus(CmdChnStatus chnIno)
{
	nlohmann::json jsonData;
	jsonData["channel"] = chnIno.channel;
	jsonData["single"] = chnIno.single;
	jsonData["loopSwitch"] = chnIno.loopSwitch;
	jsonData["globalCtl"] = chnIno.globalCtl;
	jsonData["outputReset"] = chnIno.outputReset;
	jsonData["currentStatus"] = chnIno.currentStatus;
	jsonData["circleCnt"] = chnIno.circleCnt;
	jsonData["delayPs"] = chnIno.delayPs;
	jsonData["outputCnt"] = chnIno.outputCnt;

	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/chn_info", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}


CF18SDK_API bool GetInputTrigger(CmdInputTrigger &inputTrigger)
{
	auto res = cliCf18->Get("/api/v1/input_trigger");
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		inputTrigger.polarityReversal = subData["polarityReversal"].get<int>();
		inputTrigger.edgeReset = subData["edgeReset"].get<int>();
		inputTrigger.ditheringTime = subData["ditheringTime"].get<int>();
		inputTrigger.risingEdgeCnt = subData["risingEdgeCnt"].get<int>();
		inputTrigger.fallingEdgeCnt = subData["fallingEdgeCnt"].get<int>();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetInputTrigger(CmdInputTrigger inputTrigger)
{
	nlohmann::json jsonData;
	jsonData["polarityReversal"] = inputTrigger.polarityReversal;
	jsonData["edgeReset"] = inputTrigger.edgeReset;
	jsonData["ditheringTime"] = inputTrigger.ditheringTime;
	jsonData["risingEdgeCnt"] = inputTrigger.risingEdgeCnt;
	jsonData["fallingEdgeCnt"] = inputTrigger.fallingEdgeCnt;
	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/input_trigger", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}

CF18SDK_API bool GetInputSync(CmdInputSync &inputSync)
{
	auto res = cliCf18->Get("/api/v1/input_sync");
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		inputSync.polarityReversal = subData["polarityReversal"].get<int>();
		inputSync.edgeReset = subData["edgeReset"].get<int>();
		inputSync.ditheringTime = subData["ditheringTime"].get<int>();
		inputSync.risingEdgeCnt = subData["risingEdgeCnt"].get<int>();
		inputSync.fallingEdgeCnt = subData["fallingEdgeCnt"].get<int>();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetInputSync(CmdInputSync inputSync)
{
	nlohmann::json jsonData;
	jsonData["polarityReversal"] = inputSync.polarityReversal;
	jsonData["edgeReset"] = inputSync.edgeReset;
	jsonData["ditheringTime"] = inputSync.ditheringTime;
	jsonData["risingEdgeCnt"] = inputSync.risingEdgeCnt;
	jsonData["fallingEdgeCnt"] = inputSync.fallingEdgeCnt;
	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/input_sync", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}

CF18SDK_API bool GetInputBcode(uint32_t &BcodeDitheringTime)
{
	auto res = cliCf18->Get("/api/v1/input_Bcode");
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		BcodeDitheringTime = subData["BcodeDitheringTime"].get<int>();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetInputBcode(uint32_t BcodeDitheringTime)
{
	nlohmann::json jsonData;
	jsonData["BcodeDitheringTime"] = BcodeDitheringTime;
	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/input_Bcode", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}

CF18SDK_API bool GetInternalSync(int chn, CmdInternalSync &internalSyncInfo)
{
	auto res = cliCf18->Get("/api/v1/internal_sync?id=" + std::to_string(chn));
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		internalSyncInfo.channel = subData["channel"].get<int>();
		internalSyncInfo.cycle = subData["cycle"].get<int>();
		internalSyncInfo.highLevelWidth = subData["highLevelWidth"].get<int>();
		internalSyncInfo.risingDelay = subData["risingDelay"].get<int>();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetInternalSync(CmdInternalSync internalSyncInfo)
{
	nlohmann::json jsonData;
	jsonData["channel"] = internalSyncInfo.channel;
	jsonData["cycle"] = internalSyncInfo.cycle;
	jsonData["highLevelWidth"] = internalSyncInfo.highLevelWidth;
	jsonData["risingDelay"] = internalSyncInfo.risingDelay;

	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/internal_sync", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}


CF18SDK_API bool GetInternalTrigger(int chn, CmdInternalTrigger &internalTrigInfo)
{
	auto res = cliCf18->Get("/api/v1/internal_trigger?id=" + std::to_string(chn));
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		internalTrigInfo.channel = subData["channel"].get<int>();
		internalTrigInfo.cycle = subData["cycle"].get<int>();
		internalTrigInfo.highLevelWidth = subData["highLevelWidth"].get<int>();
		internalTrigInfo.risingDelay = subData["risingDelay"].get<int>();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetInternalTrigger(CmdInternalTrigger internalTrigInfol)
{
	nlohmann::json jsonData;
	jsonData["channel"] = internalTrigInfol.channel;
	jsonData["cycle"] = internalTrigInfol.cycle;
	jsonData["highLevelWidth"] = internalTrigInfol.highLevelWidth;
	jsonData["risingDelay"] = internalTrigInfol.risingDelay;

	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/internal_trigger", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}

CF18SDK_API bool GetWaveData(std::string &wave)
{
	auto res = cliCf18->Get("/api/v1/wave");
	if (!res || res->status != 200) {
		return false;
	}
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(res->body);
		nlohmann::json subData = jsonData["data"];
		wave = subData.dump();
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	return true;
}

CF18SDK_API bool SetWaveData(std::string wave)
{
	nlohmann::json jsonData;
	try
	{
		jsonData = nlohmann::json::parse(wave);
	}
	catch (const nlohmann::json::exception&)
	{
		return false;
	}
	std::string jsonStr = jsonData.dump();
	auto res = cliCf18->Post("/api/v1/wave", jsonStr, "application/json");
	if (!res || res->status != 200) {
		return false;
	}
	return true;
}
