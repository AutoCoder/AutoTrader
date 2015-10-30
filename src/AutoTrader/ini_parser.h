/*
* Copyright (c) 2014 Jake Horsfield
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <map>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <fstream>
#include <stdexcept>

/*
* Allows this to work with compilers that do not support
* these functions (such as MinGW).
*/
namespace
{
	template <class T> std::string to_string(T val)
	{
		std::stringstream ss;
		ss << val;
		return ss.str();
	}

	int stoi(const std::string& str)
	{
		std::stringstream ss;
		int ret;
		ss << str;
		ss >> ret;
		return ret;
	}

	long stol(const std::string& str)
	{
		std::stringstream ss;
		long ret;
		ss << str;
		ss >> ret;
		return ret;
	}

	float stof(const std::string& str)
	{
		std::stringstream ss;
		float ret;
		ss << str;
		ss >> ret;
		return ret;
	}

	double stod(const std::string& str)
	{
		std::stringstream ss;
		double ret;
		ss << str;
		ss >> ret;
		return ret;
	}
}

class ini_parser
{
public:
	const std::string BOOL_TRUE;
	const std::string BOOL_FALSE;

	ini_parser(const std::string& filename)
		: BOOL_TRUE("BOOL_TRUE")
		, BOOL_FALSE("BOOL_FALSE")
		, filename(filename)
		, current_section("")
	{
		parse(filename);
	}

	void create_property(const std::string& name, const std::string& value, const std::string& section = "")
	{
		if (name.empty() || value.empty())
		{
			throw std::runtime_error("when creating a property, the name or value cannot be empty");
		}

		if (section.empty())
		{
			create_property_no_section(name, value);
		}
		else
		{
			create_property_in_section(name, value, section);
		}
	}

	/* Writes a new line at the bottom of the file, followed by the start of the section. */
	void create_section(const std::string& name)
	{
		if (name.empty())
		{
			throw std::runtime_error("when creating section, its name cannot be empty");
		}

		std::string line = "\n[" + name + "]";
		input.push_back(line);
		write_input_to_file();
	}

	int get_int(const std::string& name, const std::string& section = "") const
	{
		ensure_property_exists(section, name);
		return ::stoi(sections.at(section).at(name));
	}

	/*
	* The legal values for bools are BOOL_TRUE and BOOL_FALSE.
	* Anything other than these values are illegal.
	*/
	bool get_bool(const std::string& name, const std::string& section = "") const
	{
		ensure_property_exists(section, name);

		std::string value = sections.at(section).at(name);
		if (value == BOOL_TRUE)
		{
			return true;
		}
		else if (value == BOOL_FALSE)
		{
			return false;
		}
		else
		{
			throw std::runtime_error("unable to cast \"" + value + "\" to bool");
		}
	}

	long get_long(const std::string& name, const std::string& section = "") const
	{
		ensure_property_exists(section, name);
		return ::stol(sections.at(section).at(name));
	}

	float get_float(const std::string& name, const std::string& section = "") const
	{
		ensure_property_exists(section, name);
		return ::stof(sections.at(section).at(name));
	}

	double get_double(const std::string& name, const std::string& section = "") const
	{
		ensure_property_exists(section, name);
		return ::stod(sections.at(section).at(name));
	}

	std::string get_string(const std::string& name, const std::string& section = "") const
	{
		ensure_property_exists(section, name);
		return sections.at(section).at(name);
	}

	void set_value(const std::string& name, int value, const std::string& section = "")
	{
		set_value(name, ::to_string(value), section);
	}

	void set_value(const std::string& name, bool value, const std::string& section = "")
	{
		set_value(name, (value ? std::string(BOOL_TRUE) : std::string(BOOL_FALSE)), section);
	}

	void set_value(const std::string& name, long value, const std::string& section = "")
	{
		set_value(name, ::to_string(value), section);
	}

	void set_value(const std::string& name, float value, const std::string& section = "")
	{
		set_value(name, ::to_string(value), section);
	}

	void set_value(const std::string& name, double value, const std::string& section = "")
	{
		set_value(name, ::to_string(value), section);
	}

	void set_value(const std::string& name, const std::string& value, const std::string& section = "")
	{
		ensure_property_exists(section, name);
		sections[section][name] = value;

		bool replaced = false;
		std::string current_section = "";

		/*
		* Note that references to "current_section" refer to the local
		* variable defined above, not the member variable.
		*/
		for (unsigned i = 0; i < input.size(); ++i)
		{
			std::string& line = input[i];
			if (is_section_start_line(line))
			{
				current_section = extract_section_name(line);
			}
			else if (is_assignment_line(line))
			{
				std::string key = extract_key(line);
				if (key == name && current_section == section)
				{
					line = key + "=" + value;
					replaced = true;
				}
			}
		}

		if (replaced)
		{
			write_input_to_file();
		}
		else
		{
			throw std::runtime_error("property \"" + name + "\" in section \"" + section + "\"does not exist so cannot change its value");
		}
	}

private:
	void create_property_no_section(const std::string& name, const std::string& value)
	{
		std::string line = name + "=" + value;

		input.insert(input.begin(), line);
		write_input_to_file();

		sections[""][name] = value;
	}

	void create_property_in_section(const std::string& name, const std::string& value, const std::string& section)
	{
		std::string line = name + "=" + value;
		std::string tmp_current_section = "";

		for (auto it = input.begin(); it != input.end(); ++it)
		{
			if (is_section_start_line(*it))
			{
				tmp_current_section = extract_section_name(*it);

				if (tmp_current_section == section)
				{
					input.insert(it + 1, line);
					write_input_to_file();
					sections[section][name] = value;
					return;
				}
			}
		}

		/* Section was not found. */
		throw std::runtime_error("unable to create property \"" + name + "\" in section \"" + section + "\"");
	}

	void write_input_to_file()
	{
		std::ofstream file(filename);
		for (const auto& line : input)
		{
			file << line << '\n';
		}
		file.close();
	}

	void parse(const std::string& filename)
	{
		std::fstream file;
		file.open(filename);
		if (!file.is_open())
		{
			std::printf("error: could not open \"%s\". terminated parsing.\n", filename.c_str());
			file.close();
			return;
		}

		std::string line;
		while (std::getline(file, line))
		{
			input.push_back(line);

			if (is_comment_line(line))
			{
				continue;
			}
			else if (is_section_start_line(line))
			{
				start_section(line);
			}
			else if (is_assignment_line(line))
			{
				handle_assignment(line);
			}
		}
	}

	void start_section(const std::string& line)
	{
		current_section = extract_section_name(line);
	}

	std::string extract_section_name(const std::string& line) const
	{
		std::string name;

		for (int i = 1; line[i] != ']'; ++i)
		{
			name += line[i];
		}

		return name;
	}

	void handle_assignment(const std::string& line)
	{
		std::string key = extract_key(line);
		std::string value = extract_value(line);

		sections[current_section][key] = value;
	}

	std::string extract_key(const std::string& line) const
	{
		std::string key;

		for (int i = 0; line[i] != '='; ++i)
		{
			key += line[i];
		}

		return key;
	}

	std::string extract_value(const std::string& line) const
	{
		std::string value;

		int equals_pos;
		for (equals_pos = 0; line[equals_pos] != '='; ++equals_pos)
		{
			/* Skip to equals sign. */
		}

		/* Get everything from the character following the equals sign to the end of the line. */
		for (unsigned i = equals_pos + 1; i < line.length(); ++i)
		{
			value += line[i];
		}

		return value;
	}

	/*
	* A line is a comment if the first character is a semi-colon.
	*/
	bool is_comment_line(const std::string& line) const
	{
		return (line.length() > 0) && (line[0] == ';');
	}

	/*
	* A line is the start of a section if the first character is an open
	* bracket and the last character is a closing bracket.
	*/
	bool is_section_start_line(const std::string& line) const
	{
		return (line.length() > 0) && (line[0] == '[') && (line[line.length() - 1] == ']');
	}

	/*
	* A line contains an assignment if it contains an equals sign and
	* there is text before and after this equals sign.
	*/
	bool is_assignment_line(const std::string& line) const
	{
		std::size_t equals_pos = line.find("=");
		return (equals_pos != std::string::npos) && (equals_pos != 0) && (equals_pos != line.length() - 1);
	}

	void ensure_property_exists(const std::string& section, const std::string& name) const
	{
		if (section != "" && sections.find(section) == sections.end())
		{
			throw std::runtime_error("section \"" + section + "\" does not exist");
		}

		if (sections.at(section).find(name) == sections.at(section).end())
		{
			throw std::runtime_error("property \"" + section + "\" does not exist");
		}
	}

private:
	const std::string filename;
	std::vector<std::string> input;

	typedef std::map<std::string, std::string> properties;
	std::map<std::string, properties> sections;

	std::string current_section;
};

#endif