﻿#pragma once
#include "CommonData.h"
#include "TinyXml2Helper.h"
#include "DrawCommon.h"

class CSkinFile
{
public:
    CSkinFile();
    ~CSkinFile();

    //皮肤信息
    struct SkinInfo
    {
        wstring                 skin_author;                //皮肤的作者
    };

    //皮肤布局
    struct Layout
    {
        int                     width{}, height{};                              //整个layout的宽度与高度
        COLORREF                PrefixColor{}, ValueColor{};                    //layout缺省的前缀颜色、数值颜色

        //////////////////////////////////////////////////////////////
        //      以后将支持下面这几项
        //////////////////////////////////////////////////////////////
        FontInfo                font_info;                                      //字体信息
//      int     text_height{};                                                  //皮肤文本的高度，即每个显示项的数值显示高度。
//      bool    no_label{};                                                     //是否不显示标签
        //保存皮肤配置文件中单个Layout配置的每一项的布局信息，不包含没有配置的项。   //切换皮肤(包括重新加载当前皮肤)时存放新数据，否则不改变数据。
        std::map<CommonDisplayItem, LayoutItem>             M_LayoutItems{};
        LayoutItem& GetItem(CommonDisplayItem display_item)
        {
            return M_LayoutItems[display_item];
        }
/*
        LayoutItem GetItem(CommonDisplayItem display_item) const
        {
            auto iter = M_LayoutItems.find(display_item);
            if (iter != M_LayoutItems.end())
                return iter->second;
            return LayoutItem();
        }
*/
    };

    //皮肤布局信息
    struct LayoutManager
    {
        int     text_height{};      //皮肤文本的高度，即每个显示项的数值显示高度。
        bool    no_label{};         //是否不显示标签
        Layout  layout_l;           //“显示更多信息”时的布局
        Layout  layout_s;           //不“显示更多信息”时的布局
    };

    //皮肤预览图信息
    struct PreviewInfo
    {
        struct Pos
        {
            int x{};
            int y{};
        };
        int width{};    //预览图的宽度
        int height{};   //预览图的高度
        Pos l_pos;      //“显示更多信息”时的窗口在预览图中的位置
        Pos s_pos;      //不“显示更多信息”时的窗口在预览图中的位置
    };

    const CImage&           GetBackgroundL()    const { return m_background_l;      }
    const CImage&           GetBackgroundS()    const { return m_background_s;      }
    const SkinInfo&         GetSkinInfo()       const { return m_skin_info;         }
    const LayoutManager&    GetLayoutManager()  const { return m_layout_manager;    }
    const PreviewInfo&      GetPreviewInfo()    const { return m_preview_info;      }

    //从文件载入皮肤信息
    void LoadCfgAndBGImage(const wstring& file_path);

    //绘制预览图
    //pDC: 绘图的CDC
    //rect: 绘图区域
    void DrawPreview(CDC* pDC, CRect rect);
    //绘制主界面
    void DrawInfo(CDC* pDC, CFont& font);
private:
    void LoadFromXml(const wstring& file_path);     //从xml文件读取皮肤数据

//  void InitLayoutItemAttributes(LayoutItem&   layout_item);
    void LoadLayoutItemFromXmlNode(CSkinFile::Layout& layout, LayoutItem& layout_item, tinyxml2::XMLElement* ele);
    void LoadLayoutFromXmlNode(CSkinFile::Layout& layout, tinyxml2::XMLElement* ele);

    static void DrawSkinText(CDrawCommon drawer, CRect rect, CString label, CString value, COLORREF label_color, COLORREF value_color, Alignment align);

private:
    SkinInfo        m_skin_info;
    LayoutManager   m_layout_manager;
    PreviewInfo     m_preview_info;
    std::map<std::string, std::string> m_plugin_map;    //插件名称与xml节点名称的映射关系。key是xml节点名称，value是插件ID
    CFont  m_font;                                      //用于皮肤预览的字体对象
    CImage m_background_s;
    CImage m_background_l;

};
