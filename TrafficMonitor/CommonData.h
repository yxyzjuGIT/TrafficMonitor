﻿//此文件包含全局结构体、枚举类型的定义
#pragma once
#include "stdafx.h"
#include "CommonTypes.h"
#include "TaskbarItemOrderHelper.h"
#include "Layout.h"

//APP全局性设置数据
struct AppSettingData
{
    bool m_show_taskbar_wnd{ false };          //显示任务栏窗口
    bool m_hide_main_window;                    //隐藏主窗口
    //bool m_tbar_show_cpu_memory;              //任务栏窗口显示CPU和内存利用率

    //网络连接设置
    bool m_auto_select{ false };                //自动选择连接
    bool m_select_all{ false };                 //统计所有连接的网速
    string m_connection_name;                   //当前选择网络的名称
    //bool m_show_internet_ip{ false };         //是否在“连接详情”对话框中显示外网IP地址

    //通知图标设置
    int  m_dft_notify_icon = 0;                 //默认的通知图标(用于区分win10的深色和浅色模式)
    int  m_notify_icon_selected{};              //要显示的通知区图标
    bool m_notify_icon_auto_adapt{ false };     //通知区图标是否自动适应Win10深浅色模式

    bool m_use_log_scale{ false };              //“历史流量统计”对话框中绘制表示历史流量数值的矩形时是否使用对数比例
    HistoryTrafficViewType m_view_type{};
    bool m_sunday_first{ true };                //是否将周日作为一周的第一天
    StringSet plugin_disabled;                  //已禁用的插件

    int  taskbar_left_space_win11{};            //Windows11下，任务栏窗口显示在左侧时的边距
};

//选项设置中“主窗口设置”和“任务栏窗口设置”中公共的数据(不使用此结构体创建对象)
struct PublicSettingData
{
    bool show_tool_tip{ true };                     //显示鼠标提示

    CLayout   layout;

    //数值属性设置
    struct LayoutItemValueAttributes    layout_item_value_attributes;

    MemoryDisplay memory_display{ MemoryDisplay::USAGE_PERCENTAGE };    //内存显示方式

    //鼠标双击设置
    DoubleClickAction   double_click_action;    //鼠标双击动作
    wstring             double_click_exe;       //鼠标双击动作为打开指定应用程序时，打开的程序路径
};

//#define MAIN_WND_COLOR_NUM 9      //主窗口颜色数量
//选项设置中“主窗口设置”的数据
struct MainWndSettingData : public PublicSettingData
{
    //(1)选项对话框中的主窗口设置(当前版本情况：只支持全局性设置)
    bool swap_up_down           { false };      //交换上传和下载显示的位置
    bool m_always_on_top        { false };      //窗口置顶
    bool m_lock_window_pos      { false };      //锁定窗口位置
    bool m_mouse_penetrate      { false };      //鼠标穿透
    bool m_alow_out_of_border   { false };      //是否允许悬浮窗超出屏幕边界
    bool hide_main_wnd_when_fullscreen;         //有程序全屏运行时隐藏悬浮窗
    //(2)鼠标右键中的部分设置
    bool m_show_more_info{ false };             //显示更多信息
    int  m_transparency         { 100 };        //窗口透明度
    wstring m_skin_name;                        //选择的皮肤的名称

    int  m_position_x;                          //窗口位置的x坐标
    int  m_position_y;                          //窗口位置的y坐标
};

//#define TASKBAR_COLOR_NUM 18      //任务栏窗口颜色数量


//选项设置中“任务栏窗口设置”的数据
struct TaskBarSettingData : public PublicSettingData
{
    //缺省颜色
    int dft_back_color = 0;                                 //默认背景颜色
    int dft_transparent_color = 0;                          //默认透明色
    int dft_status_bar_color = 0x005A5A5A;                  //默认CPU/内存 状态条颜色
    int dft_text_colors = 0x00ffffffU;                      //默认文字颜色
    //颜色主题
    bool auto_adapt_light_theme{ true };                    //是否自动适应浅色主题
    int dark_default_style{ 0 };                            //深色主题时使用的预设方案
    int light_default_style{ -1 };                          //浅色主题时使用的预设方案
    bool auto_set_background_color{ false };                //根据任务栏颜色自动设置背景色
    bool auto_save_taskbar_color_settings_to_preset{};      //当启用“自动适应Windows10深色/浅色主题”时，是否在颜色设置有更改时自动将当前颜色设置保存到对应的预设
    bool IsTaskbarTransparent() const;
    void SetTaskabrTransparent(bool transparent);

    CTaskbarItemOrderHelper     item_order;
    unsigned int m_tbar_display_item{ TDI_UP | TDI_DOWN };  //任务栏窗口显示的项目
    StringSet plugin_display_item;                          //任务窗口显示的插件项目

    bool        value_right_align{ false };                 //数值是否右对齐
    int         digits_number{ 4 };                         //数据位数
    bool        horizontal_arrange{ true };                 //水平排列
    bool        tbar_wnd_on_left{ false };                  //如果为true，则任务栏窗口显示在任务栏的左侧（或上方）
    bool        tbar_wnd_snap{ false };     	            //如果为true，则在Win11中任务栏窗口贴靠中间任务栏，否则靠近边缘
    int         item_space{};                               //项目间距
    int         window_offset_top{};                        //任务栏窗口顶部边距
    int         vertical_margin{};                          //项目垂直间距

    //任务栏窗口独有设置选项
    bool b_show_resource_figure{ true };                    //是否显示资源占用图(CPU/内存)
    bool b_show_netspeed_figure{ false };                   //是否显示网速占用图(上传、下载、上传下载总和)
    bool b_show_graph_dashed_box{ true };                   //是否显示虚线框
    bool cm_graph_type{ false };                            //占用图类型(如果为false，默认原样式，柱状图显示占用率; 如为true，滚动显示占用率)
    int netspeed_figure_max_value;                          //网速占用图的最大值
    int netspeed_figure_max_value_unit{};                   //网速占用图最大值的单位（0: KB, 1: MB）
    unsigned __int64 GetNetspeedFigureMaxValueInBytes() const;  //获取网速占用图的最大值（以字节为单位）

    void ValidItemSpace();
    void ValidWindowOffsetTop();
    void ValidVerticalMargin();

    bool disable_d2d{ false };                              //是否禁用d2d绘图
    DWORD update_layered_window_error_code{0};              // 使用D2D1渲染时，UpdateLayeredWindowIndirect失败的错误代码，会在关闭任务栏窗口时被重置为0
};

//选项设置中“常规设置”的数据
struct GeneralSettingData
{
    bool check_update_when_start{ true };
    int  update_source{};                   //更新源。0: GitHub; 1: Gitee
    bool auto_run{ false };
    bool show_notify_icon{ true };          //显示通知区域图标
//通知消息
    bool traffic_tip_enable{ false };       //是否启用流量超出时提示
    int traffic_tip_value;                  //要提示的流量临界值
    int traffic_tip_unit{};                 //要提示的流量值的单位（0: MB, 1: GB）

    struct NotifyTipSettings                //超过某个值时弹出提示的设置
    {
        bool enable;                        //是否启用提示
        int tip_value;                      //要弹出提示的临界值
    };
    NotifyTipSettings memory_usage_tip;     //用内存使用率超出提示
    NotifyTipSettings cpu_temp_tip;         //CPU温度超出提示
    NotifyTipSettings gpu_temp_tip;         //显卡温度超出提示
    NotifyTipSettings hdd_temp_tip;         //硬盘温度超出提示
    NotifyTipSettings mainboard_temp_tip;   //主板温度超出提示

    //语言
    Language language;

    bool show_all_interface{ true };
    bool m_get_cpu_usage_by_cpu_times{ false }; //获取CPU利用率的方式，如果为true则是使用GetSystemTimes，否则使用Pdh(性能计数器)

    bool portable_mode{ false };                //便携模式，如果为true，则程序所有数据都保存到exe所在目录下，否则保存到Appdata\Romaing目录下
    int monitor_time_span{ 1000 };              //监控的时间间隔

    //要监控的硬件及其具体的名称
    unsigned int hardware_monitor_item{};       //要监控哪些硬件
    std::wstring hard_disk_name;                //要监控的硬盘名称
    std::wstring cpu_core_name;                 //要监控的CPU核心的名称
    bool IsHardwareEnable(HardwareItem item_type) const
    {
        return hardware_monitor_item & item_type;
    }
    void SetHardwareEnable(HardwareItem item_type, bool enable)
    {
        if (enable)
            hardware_monitor_item |= item_type;
        else
            hardware_monitor_item &= ~item_type;
    }

    StringSet connections_hide;     //用于保存哪些网络要从“选择网络连接”子菜单项中隐藏
};

//通过构造函数传递一个bool变量的引用，在构造时将其置为true，析构时置为false
class CFlagLocker
{
public:
    CFlagLocker(bool& flag)
        : m_flag(flag)
    {
        m_flag = true;
    }

    ~CFlagLocker()
    {
        m_flag = false;
    }

private:
    bool& m_flag;
};
