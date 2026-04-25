#pragma once
#include "../include/PluginInterface.h"
#include <string>

class CNetDoctorPlugin;

class CNetDoctorItemBase : public IPluginItem {
public:
    CNetDoctorItemBase(CNetDoctorPlugin& plugin, std::wstring name, std::wstring id, std::wstring label, std::wstring sample);
    const wchar_t* GetItemName() const override { return m_name.c_str(); }
    const wchar_t* GetItemId() const override { return m_id.c_str(); }
    const wchar_t* GetItemLableText() const override { return m_label.c_str(); }
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override { return m_sample.c_str(); }
protected:
    virtual std::wstring BuildValue() const = 0;
    CNetDoctorPlugin& m_plugin;
    std::wstring m_name, m_id, m_label, m_sample;
    mutable std::wstring m_value;
};
class CNetSummaryItem : public CNetDoctorItemBase { public: CNetSummaryItem(CNetDoctorPlugin& p); private: std::wstring BuildValue() const override; };
class CDnsStatusItem : public CNetDoctorItemBase { public: CDnsStatusItem(CNetDoctorPlugin& p); private: std::wstring BuildValue() const override; };
class CCnStatusItem : public CNetDoctorItemBase { public: CCnStatusItem(CNetDoctorPlugin& p); private: std::wstring BuildValue() const override; };
class CIntlStatusItem : public CNetDoctorItemBase { public: CIntlStatusItem(CNetDoctorPlugin& p); private: std::wstring BuildValue() const override; };
class CProxyStatusItem : public CNetDoctorItemBase { public: CProxyStatusItem(CNetDoctorPlugin& p); private: std::wstring BuildValue() const override; };
class CPublicIpItem : public CNetDoctorItemBase { public: CPublicIpItem(CNetDoctorPlugin& p); private: std::wstring BuildValue() const override; };
