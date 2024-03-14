﻿#include "stdafx.h"
#include <Windows.h>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <locale>
#include <initializer_list>
#include <string.h>
#include "DrawTextManager.h"
#include "PluginManager.h"
#include "Common.h"
#include "TrafficMonitor.h"

#define PLUGIN_UNSUPPORT_VERSION 0      //不被支持的插件版本

CPluginManager::CPluginManager()
{
}

CPluginManager::~CPluginManager()
{
    //卸载插件
    for (const auto& m : V_PluginManageUnit)
        FreeLibrary(m.m_plugin_module);
}

static wstring WcharArrayToWString(const wchar_t* str)
{
    if (str == nullptr)
        return wstring();
    else
        return wstring(str);
}

void CPluginManager::LoadPlugins()
{
    //从plugins目录下加载插件
    wstring plugin_dir;
    plugin_dir = CCommon::GetModuleDir() + L"plugins";
    vector<wstring> plugin_files;
    CCommon::GetFiles((plugin_dir + L"\\*.dll").c_str(), plugin_files);		//获取Plugins目录下所有的dll文件的文件名
    for (const auto& file : plugin_files)
    {
        //插件信息
        V_PluginManageUnit.push_back(PluginManageUnit());                       //存入新的空PluginManageUnit
        PluginManageUnit& plugin_manage_unit{ V_PluginManageUnit.back() };      //得到刚存入的空PluginManageUnit引用，以便接下来存入数据。
        //插件dll的路径
        plugin_manage_unit.m_file_path = plugin_dir + file;
        //插件文件名
        std::wstring file_name{ file };
        if (!file_name.empty() && (file_name[0] == L'\\' || file_name[0] == L'/'))
            file_name = file_name.substr(1);
        //如果插件被禁用，则不加载插件
        if (theApp.m_cfg_data.plugin_disabled.Contains(file_name))
        {
            plugin_manage_unit.state = PluginState::PS_DISABLE;
            continue;
        }
        //载入dll
        plugin_manage_unit.m_plugin_module = LoadLibrary(plugin_manage_unit.m_file_path.c_str());
        if (NULL == plugin_manage_unit.m_plugin_module)
        {
            plugin_manage_unit.state = PluginState::PS_MUDULE_LOAD_FAILED;
            plugin_manage_unit.error_code = GetLastError();
            continue;
        }
        // hook插件导入的User32.dll下的DrawText系列函数
        ReplacePluginDrawTextFunction(plugin_manage_unit.m_plugin_module);
        //获取函数的入口地址
        pfTMPluginGetInstance TMPluginGetInstance = (pfTMPluginGetInstance)::GetProcAddress(plugin_manage_unit.m_plugin_module, "TMPluginGetInstance");
        if (TMPluginGetInstance == NULL)
        {
            plugin_manage_unit.state = PluginState::PS_FUNCTION_GET_FAILED;
            plugin_manage_unit.error_code = GetLastError();
            continue;
        }
        //创建插件对象
        plugin_manage_unit.plugin = TMPluginGetInstance();
        if (plugin_manage_unit.plugin == nullptr)
            continue;
        //检查插件版本
        int version = plugin_manage_unit.plugin->GetAPIVersion();
        if (version <= PLUGIN_UNSUPPORT_VERSION)
        {
            plugin_manage_unit.state = PluginState::PS_VERSION_NOT_SUPPORT;
            continue;
        }
        //向插件传递插件自己使用的配置文件的路径
        std::wstring config_dir = theApp.m_config_dir;
        config_dir += L"plugins\\";
        if (version >= 2)
        {
            CreateDirectory(config_dir.c_str(), NULL);       //如果plugins不存在，则创建它
            plugin_manage_unit.plugin->OnExtenedInfo(ITMPlugin::EI_CONFIG_DIR, config_dir.c_str());
        }

        //获取插件信息(名称、描述、版本等)
        for (int i{}; i < ITMPlugin::TMI_MAX; i++)
        {
            ITMPlugin::PluginInfoIndex index{ static_cast<ITMPlugin::PluginInfoIndex>(i) };
            plugin_manage_unit.M_properties[index] = WcharArrayToWString(plugin_manage_unit.plugin->GetInfo(index));
        }

        //获取插件支持的所有显示项目(插件可能会通过用户设置的形式来增加、修改、删除其支持的所有显示项目)
        int index = 0;
        while (true)
        {
            IPluginItem* item = plugin_manage_unit.plugin->GetItem(index);
            if (item == nullptr)
                break;
            plugin_manage_unit.V_PI_PluginItems.push_back(item);
            V_PI_PluginItems.push_back(item);                               //将所有ITMPlugin的所有IPluginItem都存入一个vector
            M_IPlguinItem_to_ITMPlugin[item] = plugin_manage_unit.plugin;   //记录每个IPlguinItem属于哪个ITMPlugin
            index++;
        }
    }
    // hook MFC导入的User32.dll下的DrawText系列函数
    ReplaceMfcDrawTextFunction();

    //初始化所有任务栏显示项目
    for (const auto& display_item : gS_AllBuiltinDisplayItems)
    {
        S_all_display_items_with_plugins.insert(display_item);
    }
    for (const auto& display_item : V_PI_PluginItems)
    {
        S_all_display_items_with_plugins.insert(display_item);
    }
}

const std::vector<IPluginItem*>& CPluginManager::GetAllIPluginItems() const
{
    return V_PI_PluginItems;
}

const std::vector<CPluginManager::PluginManageUnit>& CPluginManager::GetAllPluginManageUnit() const
{
    return V_PluginManageUnit;
}

IPluginItem* CPluginManager::GetIPluginItemById(const std::wstring& item_id)
{
    for (const auto& item : V_PI_PluginItems)
    {
        if (item->GetItemId() == item_id)
            return item;
    }
    return nullptr;
}

IPluginItem* CPluginManager::GetIPluginItemByIndex(size_t index)
{
    if (index < V_PI_PluginItems.size())
        return V_PI_PluginItems[index];
    return nullptr;
}

int CPluginManager::GetIPlguinItemIndex(IPluginItem* item) const
{
    for (auto iter = V_PI_PluginItems.begin(); iter != V_PI_PluginItems.end(); ++iter)
    {
        if (*iter == item)
            return iter - V_PI_PluginItems.begin();
    }
    return -1;
}

ITMPlugin* CPluginManager::GetITMPluginByIPlguinItem(IPluginItem* pItem)
{
    if (pItem == nullptr)
        return nullptr;
    return M_IPlguinItem_to_ITMPlugin[pItem];
}

int CPluginManager::GetIPluginIndex(ITMPlugin* plugin) const
{
    for (auto iter = V_PluginManageUnit.begin(); iter != V_PluginManageUnit.end(); ++iter)
    {
        if (iter->plugin == plugin)
            return iter - V_PluginManageUnit.begin();
    }
    return -1;
}

void CPluginManager::EnumPlugin(std::function<void(ITMPlugin*)> func) const
{
    for (const auto& item : V_PluginManageUnit)
    {
        if (item.plugin != nullptr)
        {
            func(item.plugin);
        }
    }
}

void CPluginManager::EnumPluginItem(std::function<void(IPluginItem*)> func) const
{
    for (const auto& item : V_PI_PluginItems)
    {
        if (item != nullptr)
        {
            func(item);
        }
    }
}

const std::set<CommonDisplayItem>& CPluginManager::AllDisplayItemsWithPlugins()
{
    return S_all_display_items_with_plugins;
}

int CPluginManager::GetItemWidth(IPluginItem* pItem, CDC* pDC)
{
    int width = 0;
    ITMPlugin* plugin = GetITMPluginByIPlguinItem(pItem);
    if (plugin != nullptr && plugin->GetAPIVersion() >= 3)
    {
        width = pItem->GetItemWidthEx(pDC->GetSafeHdc());       //优先使用GetItemWidthEx接口获取宽度
    }
    if (width == 0)
    {
        width = theApp.DPI(pItem->GetItemWidth());
    }
    return width;
}

std::wstring CPluginManager::PluginManageUnit::Property(ITMPlugin::PluginInfoIndex index) const
{
    auto iter = M_properties.find(index);
    if (iter != M_properties.end())
        return iter->second;
    return wstring();
}

template <class T>
void inline CopyData(void* from, T* to)
{
    ::memcpy(to, from, sizeof(T));
}

auto inline GetCLocale() noexcept
    -> const std::locale&
{
    const static std::locale restlt{"C"};
    return restlt;
};

/**
 * @brief 将输入的dll名称转大写
 *
 * @param dll_name 输入的dll名称
 * @param expected_length 期望的dll名称长度，不匹配则无操作
 * @return std::string 大写的dll名称
 */
std::string UpperDllName(const char* dll_name, std::size_t expected_length)
{
    std::string result{};
    result.resize(expected_length);
    const char* dll_name_last = &dll_name[expected_length];
    std::transform(dll_name, dll_name_last, result.begin(),
                   [](char c) -> char
                   { return std::toupper(c, GetCLocale()); });
    return result;
}

std::string UpperDllName(const char* dll_name)
{
    std::string result{};
    for (std::size_t i = 0; i < 256; ++i)
    {
        auto current_char = dll_name[i];
        if (current_char == '\0')
        {
            result += '\0';
            break;
        }
        result += std::toupper(current_char, GetCLocale());
    }
    return result;
}

class DllName
{
public:
    class FunctionNameImpl
    {
        using Handler = std::function<void(void**, void*)>;
        friend DllName;

    public:
        /**
         * @brief Construct a new Function Name Impl object
         *
         * @param function_name 函数名称
         * @param on_function_find_handler 当查找到函数时执行的方法，
         类型为std::function<void(void** pp_found_function_address, void* p_args)>
         */
        FunctionNameImpl(const std::string& function_name, Handler on_function_find_handler)
            : m_function_name{function_name}, m_on_function_find_handler{on_function_find_handler}, m_p_function{}
        {
        }
        ~FunctionNameImpl() = default;

        void OnFunctionFind(void** pp_found_fuction, void* p_args) const
        {
            m_on_function_find_handler(pp_found_fuction, p_args);
        }

        auto NameString() const noexcept
            -> const std::string&
        {
            return m_function_name;
        }

    private:
        void* m_p_function;
        Handler m_on_function_find_handler;
        std::string m_function_name;
    };
    using FunctionName = FunctionNameImpl;

private:
    std::unordered_map<void*, FunctionName> m_function_name_map{};
    std::string m_dll_name;

public:
    DllName(const char* dll_name, std::size_t expected_length, std::initializer_list<FunctionName> function_names_in_dll)
        : m_dll_name{UpperDllName(dll_name, expected_length)}
    {
        auto h_dll = ::LoadLibraryA(dll_name);
        if (h_dll)
        {
            for (auto function_name : function_names_in_dll)
            {
                auto* function_pointer =
                    reinterpret_cast<void*>(::GetProcAddress(h_dll, function_name.NameString().c_str()));

                auto exist_it = m_function_name_map.find(function_pointer);
                if (exist_it == m_function_name_map.end())
                {
                    m_function_name_map.emplace(std::move(std::make_pair(function_pointer, function_name)));
                }
            }
            ::FreeLibrary(h_dll);
        }
    }
    ~DllName() = default;

    auto NameString() const noexcept
        -> const std::string&
    {
        return m_dll_name;
    }

    auto MatchFunctionPointer(void* found_pointer) const noexcept
        -> const FunctionName*
    {
        auto it = m_function_name_map.find(found_pointer);
        if (it != m_function_name_map.end())
        {
            return &it->second;
        }
        return nullptr;
    }
};

using FunctionName = DllName::FunctionNameImpl;

class DllMap
{
private:
    std::unordered_map<std::string, DllName> m_dll_name_map;

public:
    DllMap(std::initializer_list<DllName> dll_names)
    {
        for (auto&& dll_name : dll_names)
        {
            auto exist_dll_name = m_dll_name_map.find(dll_name.NameString());
            if (exist_dll_name == m_dll_name_map.end())
            {
                m_dll_name_map.emplace(std::make_pair(dll_name.NameString(), dll_name));
            }
        }
    }
    ~DllMap() = default;

    auto MatchDllName(const char* any_case_string) const
        -> const DllName*
    {
        std::string capital_dll_name = UpperDllName(any_case_string);
        auto it = m_dll_name_map.find(capital_dll_name);
        if (it != m_dll_name_map.end())
        {
            return &it->second;
        }
        return nullptr;
    }
};

#define ARR_ARRLENGTH(c_array) c_array, GetArrayLength(c_array)

void CPluginManager::ReplacePluginDrawTextFunction(HMODULE plgin_dll_module) noexcept
{
    auto* p_plugin_dll_module = reinterpret_cast<BYTE*>(plgin_dll_module);

    auto p_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(plgin_dll_module);

    auto p_image_nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(
        p_plugin_dll_module + p_dos_header->e_lfanew);

    auto p_optional_header = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(
        &p_image_nt_headers->OptionalHeader);

    auto import_table_offset =
        p_optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    auto p_current_import_desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        p_plugin_dll_module + import_table_offset);

    //不区分大小写，需要输入的dll中导入表里User32.dll的4个函数
    static const char user32_dll_name[] = "User32.dll";
    static const DllName dll_names{ARR_ARRLENGTH(user32_dll_name),
                                   {{"DrawTextA", User32DrawTextManager::A::GetReplaceOperation()},
                                    {"DrawTextW", User32DrawTextManager::W::GetReplaceOperation()},
                                    {"DrawTextExA", User32DrawTextManager::ExA::GetReplaceOperation()},
                                    {"DrawTextExW", User32DrawTextManager::ExW::GetReplaceOperation()}}};
    static const DllMap dll_map{{dll_names}};

    while (p_current_import_desc->FirstThunk != NULL)
    {
        const auto* p_import_dll_name = reinterpret_cast<char*>(
            p_plugin_dll_module + p_current_import_desc->Name);

        auto* p_matched_dll_name = dll_map.MatchDllName(p_import_dll_name);
        if (p_matched_dll_name != nullptr)
        {
            //搜索IAT表
            auto p_current_chunk = reinterpret_cast<PIMAGE_THUNK_DATA>(
                p_plugin_dll_module + p_current_import_desc->FirstThunk);

            while (p_current_chunk->u1.AddressOfData != NULL)
            {
                static_assert(sizeof(p_current_chunk->u1.Function) == sizeof(void*), "Length not match!");
                auto p_target_pointer = reinterpret_cast<void**>(&p_current_chunk->u1.Function);

                auto p_matched_function =
                    p_matched_dll_name->MatchFunctionPointer(*p_target_pointer);
                if (p_matched_function != nullptr)
                {
                    p_matched_function->OnFunctionFind(p_target_pointer, nullptr);
                }
                ++p_current_chunk;
            }
        }
        ++p_current_import_desc;
    }
}

void CPluginManager::ReplaceMfcDrawTextFunction() noexcept
{
    HMODULE mfc_module = ::LoadLibraryA("mfc140u.dll");
    if (mfc_module != NULL)
    {
        ReplacePluginDrawTextFunction(mfc_module);
        ::FreeLibrary(mfc_module);
    }
}
