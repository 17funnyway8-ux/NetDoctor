#pragma once
#include "../include/PluginInterface.h"
#include "ConfigManager.h"
#include "NetworkChecker.h"
#include "DiagnosisEngine.h"
#include <chrono>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class CNetDoctorPlugin : public ITMPlugin {
public:
    static CNetDoctorPlugin& Instance();
    ~CNetDoctorPlugin();
    IPluginItem* GetItem(int index) override;
    void DataRequired() override;
    const wchar_t* GetInfo(PluginInfoIndex index) override;
    const wchar_t* GetTooltipInfo() override;
    OptionReturn ShowOptionsDialog(void* hParent) override;
    void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    NetDoctorState GetStateSnapshot() const;
private:
    CNetDoctorPlugin();
    void LoadConfig();
    void UpdateDataIfNeeded(bool force = false);
    void StartWorkerCheck();
    void ApplyWorkerResult(NetDoctorState&& state);
    void BuildTooltip();
private:
    std::vector<std::unique_ptr<IPluginItem>> m_items;
    ConfigManager m_config;
    NetworkChecker m_checker;
    DiagnosisEngine m_diagnosis;
    mutable std::mutex m_state_mutex;
    NetDoctorState m_state;
    std::wstring m_config_dir;
    std::wstring m_tooltip{L"NetDoctor\nWaiting for first check..."};
    std::chrono::steady_clock::time_point m_last_check{};
    std::atomic_bool m_checking{false};
    std::atomic_bool m_has_checked{false};
    std::atomic_bool m_shutdown{false};
    std::thread m_worker_thread;
};
