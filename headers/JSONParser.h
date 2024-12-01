#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <iostream>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class JsonParser
{
public:
	virtual bool DeSerialize(std::string json_string, json& json_obj)
	{
		bool isLegal = json::accept(json_string);
		if (!isLegal)
			return false;
		json_obj = json::parse(json_string.c_str());
		return true;
	}
	virtual std::string Serialize(const json& json_obj)
	{ 
		try { 
			return json_obj.dump();
		} catch (json::type_error& e) { 
			std::cerr << "JSON serialization error: " << e.what() << std::endl;
			return "";
		}
	}
	template<typename T>
	T GetValue(const json& json_obj, const std::string& key)
	{
		try { 
			return json_obj.at(key).get<T>();
		} catch (json::out_of_range& e) {
			std::cerr << "Key not found: " << key << std::endl;
			throw;
		} catch (json::type_error& e) {
			std::cerr << "Type error for key: " << key << std::endl; throw; 
		}
	}
	template<typename T> 
	void SetValue(json& json_obj, const std::string& key, const T& value)
	{ 
		try { 
			json_obj[key] = value;
		} catch (const std::exception& e) {
			std::cerr << "Failed to set value: " << e.what() << std::endl;
		}
	}
protected:

private:

};

#endif // !JSON_PARSER_H
