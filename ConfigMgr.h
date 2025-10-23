#pragma once
#include "const.h"
struct Sectioninfo {
	Sectioninfo()
	{
		
	}
	~Sectioninfo()
	{
		_section_datas.clear();
	}
	Sectioninfo& operator=(const Sectioninfo& src) {
		if (this != &src) {  
			this->_section_datas = src._section_datas;
		}
		return *this;
	}
	Sectioninfo(const Sectioninfo& src) {
		_section_datas = src._section_datas;
	}
	std::map<std::string, std::string> _section_datas;
	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";
		}
        return _section_datas[key];
	}
};
class ConfigMgr
{
public:
	~ConfigMgr()
	{
		_config_maps.clear();
	}
	Sectioninfo& operator[](const std::string& section) {
		auto it = _config_maps.find(section);
		if (it == _config_maps.end()) {
			auto result = _config_maps.emplace(section, Sectioninfo());
			return result.first->second;
		}
		return it->second;
	}
	
	static ConfigMgr& GetInstance()
	{
		static ConfigMgr cfg_mgr;
		return cfg_mgr;
	}

	ConfigMgr(const ConfigMgr& src) {
		_config_maps = src._config_maps;
	}
	ConfigMgr& operator=(const ConfigMgr& src) {
		if (this != &src) {
			this->_config_maps = src._config_maps;
		}
		return *this;
	}
	ConfigMgr();
private:
    std::map<std::string, Sectioninfo> _config_maps;
	
};

