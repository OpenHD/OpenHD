#ifndef __INI_H__
 #define __INI_H__
 
 #include <ctype.h>
 #include <stdio.h>
 #include <string.h>
 #include <sys/stat.h>
 
 #include <algorithm>
 #include <cctype>
 #include <cstdlib>
 #include <fstream>
 #include <iostream>
 #include <iterator>
 #include <map>
 #include <set>
 #include <sstream>
 #include <string>
 #include <type_traits>
 #include <unordered_map>
 #include <vector>
 
 namespace inih {
 
 /* Typedef for prototype of handler function. */
 typedef int (*ini_handler)(void* user, const char* section, const char* name,
                            const char* value);
 
 /* Typedef for prototype of fgets-style reader function. */
 typedef char* (*ini_reader)(char* str, int num, void* stream);
 
 #define INI_STOP_ON_FIRST_ERROR 1
 #define INI_MAX_LINE 2000
 #define INI_INITIAL_ALLOC 200
 #define MAX_SECTION 50
 #define MAX_NAME 50
 #define INI_START_COMMENT_PREFIXES ";#"
 #define INI_INLINE_COMMENT_PREFIXES ";"
 
 /* Strip whitespace chars off end of given string, in place. Return s. */
 inline static char* rstrip(char* s) {
     char* p = s + strlen(s);
     while (p > s && isspace((unsigned char)(*--p))) *p = '\0';
     return s;
 }
 
 /* Return pointer to first non-whitespace char in given string. */
 inline static char* lskip(const char* s) {
     while (*s && isspace((unsigned char)(*s))) s++;
     return (char*)s;
 }
 
 /* Return pointer to first char (of chars) or inline comment in given string,
    or pointer to null at end of string if neither found. Inline comment must
    be prefixed by a whitespace character to register as a comment. */
 inline static char* find_chars_or_comment(const char* s, const char* chars) {
     int was_space = 0;
     while (*s && (!chars || !strchr(chars, *s)) &&
            !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
         was_space = isspace((unsigned char)(*s));
         s++;
     }
     return (char*)s;
 }
 
 /* Version of strncpy that ensures dest (size bytes) is null-terminated. */
 inline static char* strncpy0(char* dest, const char* src, size_t size) {
     strncpy(dest, src, size - 1);
     dest[size - 1] = '\0';
     return dest;
 }
 
 /* See documentation in header file. */
 inline int ini_parse_stream(ini_reader reader, void* stream,
                             ini_handler handler, void* user) {
     /* Uses a fair bit of stack (use heap instead if you need to) */
     char* line;
     size_t max_line = INI_INITIAL_ALLOC;
     char* new_line;
     size_t offset;
     char section[MAX_SECTION] = "";
     char prev_name[MAX_NAME] = "";
 
     char* start;
     char* end;
     char* name;
     char* value;
     int lineno = 0;
     int error = 0;
 
     line = (char*)malloc(INI_INITIAL_ALLOC);
     if (!line) {
         return -2;
     }
 
 #if INI_HANDLER_LINENO
 #define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
 #else
 #define HANDLER(u, s, n, v) handler(u, s, n, v)
 #endif
 
     /* Scan through stream line by line */
     while (reader(line, (int)max_line, stream) != NULL) {
         offset = strlen(line);
         while (offset == max_line - 1 && line[offset - 1] != '\n') {
             max_line *= 2;
             if (max_line > INI_MAX_LINE) max_line = INI_MAX_LINE;
             new_line = (char*)realloc(line, max_line);
             if (!new_line) {
                 free(line);
                 return -2;
             }
             line = new_line;
             if (reader(line + offset, (int)(max_line - offset), stream) == NULL)
                 break;
             if (max_line >= INI_MAX_LINE) break;
             offset += strlen(line + offset);
         }
 
         lineno++;
 
         start = line;
         if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
             (unsigned char)start[1] == 0xBB &&
             (unsigned char)start[2] == 0xBF) {
             start += 3;
         }
         start = lskip(rstrip(start));
 
         if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
             /* Start-of-line comment */
         } else if (*start == '[') {
             /* A "[section]" line */
             end = find_chars_or_comment(start + 1, "]");
             if (*end == ']') {
                 *end = '\0';
                 strncpy0(section, start + 1, sizeof(section));
                 *prev_name = '\0';
             } else if (!error) {
                 /* No ']' found on section line */
                 error = lineno;
             }
         } else if (*start) {
             /* Not a comment, must be a name[=:]value pair */
             end = find_chars_or_comment(start, "=:");
             if (*end == '=' || *end == ':') {
                 *end = '\0';
                 name = rstrip(start);
                 value = end + 1;
                 end = find_chars_or_comment(value, NULL);
                 if (*end) *end = '\0';
                 value = lskip(value);
                 rstrip(value);
 
                 /* Valid name[=:]value pair found, call handler */
                 strncpy0(prev_name, name, sizeof(prev_name));
                 if (!HANDLER(user, section, name, value) && !error)
                     error = lineno;
             } else if (!error) {
                 /* No '=' or ':' found on name[=:]value line */
                 error = lineno;
             }
         }
 
         if (error) break;
     }
 
     free(line);
 
     return error;
 }
 
 inline int ini_parse_file(FILE* file, ini_handler handler, void* user) {
     return ini_parse_stream((ini_reader)fgets, file, handler, user);
 }
 
 inline int ini_parse(const char* filename, ini_handler handler, void* user) {
     FILE* file;
     int error;
 
     file = fopen(filename, "r");
     if (!file) return -1;
     error = ini_parse_file(file, handler, user);
     fclose(file);
     return error;
 }
 
 #endif /* __INI_H__ */
 
 #ifndef __INIREADER_H__
 #define __INIREADER_H__
 
 // Read an INI file into easy-to-access name/value pairs. (Note that I've gone
 // for simplicity here rather than speed, but it should be pretty decent.)
 class INIReader {
    public:
     // Empty Constructor
     INIReader(){};
 
     // Construct INIReader and parse given filename. See ini.h for more info
     // about the parsing.
     INIReader(std::string filename);
 
     // Construct INIReader and parse given file. See ini.h for more info
     // about the parsing.
     INIReader(FILE* file);
 
     // Return the result of ini_parse(), i.e., 0 on success, line number of
     // first error on parse error, or -1 on file open error.
     int ParseError() const;
 
     // Return the list of sections found in ini file
     const std::set<std::string> Sections() const;
 
     // Return the list of keys in the given section
     const std::set<std::string> Keys(std::string section) const;
 
     const std::unordered_map<std::string, std::string> Get(
         std::string section) const;
 
     template <typename T = std::string>
     T Get(const std::string& section, const std::string& name) const;
 
     template <typename T>
     T Get(const std::string& section, const std::string& name,
           T&& default_v) const;
 
     template <typename T = std::string>
     std::vector<T> GetVector(const std::string& section,
                              const std::string& name) const;
 
     template <typename T>
     std::vector<T> GetVector(const std::string& section,
                              const std::string& name,
                              const std::vector<T>& default_v) const;
 
     template <typename T = std::string>
     void InsertEntry(const std::string& section, const std::string& name,
                      const T& v);
 
     template <typename T = std::string>
     void InsertEntry(const std::string& section, const std::string& name,
                      const std::vector<T>& vs);
 
     template <typename T = std::string>
     void UpdateEntry(const std::string& section, const std::string& name,
                      const T& v);
 
     template <typename T = std::string>
     void UpdateEntry(const std::string& section, const std::string& name,
                      const std::vector<T>& vs);
 
    protected:
     int _error;
     std::unordered_map<std::string,
                        std::unordered_map<std::string, std::string>>
         _values;
     static int ValueHandler(void* user, const char* section, const char* name,
                             const char* value);
 
     template <typename T>
     T Converter(const std::string& s) const;
 
     const bool BoolConverter(std::string s) const;
 
     template <typename T>
     std::string V2String(const T& v) const;
 
     template <typename T>
     std::string Vec2String(const std::vector<T>& v) const;
 };
 
 #endif  // __INIREADER_H__
 
 #ifndef __INIREADER__
 #define __INIREADER__
 
 inline INIReader::INIReader(std::string filename) {
     _error = ini_parse(filename.c_str(), ValueHandler, this);
     ParseError();
 }
 
 inline INIReader::INIReader(FILE* file) {
     _error = ini_parse_file(file, ValueHandler, this);
     ParseError();
 }
 
 inline int INIReader::ParseError() const {
     switch (_error) {
         case 0:
             break;
         case -1:
             throw std::runtime_error("ini file not found.");
         case -2:
             throw std::runtime_error("memory alloc error");
         default:
             throw std::runtime_error("parse error on line no: " +
                                      std::to_string(_error));
     }
     return 0;
 }
 
 inline const std::set<std::string> INIReader::Sections() const {
     std::set<std::string> retval;
     for (auto const& element : _values) {
         retval.insert(element.first);
     }
     return retval;
 }
 
 inline const std::set<std::string> INIReader::Keys(std::string section) const {
     auto const _section = Get(section);
     std::set<std::string> retval;
     for (auto const& element : _section) {
         retval.insert(element.first);
     }
     return retval;
 }
 
 inline const std::unordered_map<std::string, std::string> INIReader::Get(
     std::string section) const {
     auto const _section = _values.find(section);
     if (_section == _values.end()) {
         throw std::runtime_error("section '" + section + "' not found.");
     }
     return _section->second;
 }
 
 template <typename T>
 inline T INIReader::Get(const std::string& section,
                         const std::string& name) const {
     auto const _section = Get(section);
     auto const _value = _section.find(name);
 
     if (_value == _section.end()) {
         throw std::runtime_error("key '" + name + "' not found in section '" +
                                  section + "'.");
     }
 
     std::string value = _value->second;
 
     if constexpr (std::is_same<T, std::string>()) {
         return value;
     } else if constexpr (std::is_same<T, bool>()) {
         return BoolConverter(value);
     } else {
         return Converter<T>(value);
     };
 }
 
 template <typename T>
 inline T INIReader::Get(const std::string& section, const std::string& name,
                         T&& default_v) const {
     try {
         return Get<T>(section, name);
     } catch (std::runtime_error& e) {
         return default_v;
     }
 }
 
 template <typename T>
 inline std::vector<T> INIReader::GetVector(const std::string& section,
                                            const std::string& name) const {
     std::string value = Get(section, name);
 
     std::istringstream out{value};
     const std::vector<std::string> strs{std::istream_iterator<std::string>{out},
                                         std::istream_iterator<std::string>()};
     try {
         std::vector<T> vs{};
         for (const std::string& s : strs) {
             vs.emplace_back(Converter<T>(s));
         }
         return vs;
     } catch (std::exception& e) {
         throw std::runtime_error("cannot parse value " + value +
                                  " to vector<T>.");
     }
 }
 
 template <typename T>
 inline std::vector<T> INIReader::GetVector(
     const std::string& section, const std::string& name,
     const std::vector<T>& default_v) const {
     try {
         return GetVector<T>(section, name);
     } catch (std::runtime_error& e) {
         return default_v;
     };
 }
 
 template <typename T>
 inline void INIReader::InsertEntry(const std::string& section,
                                    const std::string& name, const T& v) {
     if (_values[section][name].size() > 0) {
         throw std::runtime_error("duplicate key '" + std::string(name) +
                                  "' in section '" + section + "'.");
     }
     _values[section][name] = V2String(v);
 }
 
 template <typename T>
 inline void INIReader::InsertEntry(const std::string& section,
                                    const std::string& name,
                                    const std::vector<T>& vs) {
     if (_values[section][name].size() > 0) {
         throw std::runtime_error("duplicate key '" + std::string(name) +
                                  "' in section '" + section + "'.");
     }
     _values[section][name] = Vec2String(vs);
 }
 
 template <typename T>
 inline void INIReader::UpdateEntry(const std::string& section,
                                    const std::string& name, const T& v) {
     if (!_values[section][name].size()) {
         throw std::runtime_error("key '" + std::string(name) +
                                  "' not exist in section '" + section + "'.");
     }
     _values[section][name] = V2String(v);
 }
 
 template <typename T>
 inline void INIReader::UpdateEntry(const std::string& section,
                                    const std::string& name,
                                    const std::vector<T>& vs) {
     if (!_values[section][name].size()) {
         throw std::runtime_error("key '" + std::string(name) +
                                  "' not exist in section '" + section + "'.");
     }
     _values[section][name] = Vec2String(vs);
 }
 
 template <typename T>
 inline std::string INIReader::V2String(const T& v) const {
     std::stringstream ss;
     ss << v;
     return ss.str();
 }
 
 template <typename T>
 inline std::string INIReader::Vec2String(const std::vector<T>& v) const {
     if (v.empty()) {
         return "";
     }
     std::ostringstream oss;
     std::copy(v.begin(), v.end() - 1, std::ostream_iterator<T>(oss, " "));
     oss << v.back();
 
     return oss.str();
 }
 
 template <typename T>
 inline T INIReader::Converter(const std::string& s) const {
     try {
         T v{};
         std::istringstream _{s};
         _.exceptions(std::ios::failbit);
         _ >> v;
         return v;
     } catch (std::exception& e) {
         throw std::runtime_error("cannot parse value '" + s + "' to type<T>.");
     };
 }
 
 inline const bool INIReader::BoolConverter(std::string s) const {
     std::transform(s.begin(), s.end(), s.begin(), ::tolower);
     static const std::unordered_map<std::string, bool> s2b{
         {"1", true},  {"true", true},   {"yes", true}, {"on", true},
         {"0", false}, {"false", false}, {"no", false}, {"off", false},
     };
     auto const value = s2b.find(s);
     if (value == s2b.end()) {
         throw std::runtime_error("'" + s + "' is not a valid boolean value.");
     }
     return value->second;
 }
 
 inline int INIReader::ValueHandler(void* user, const char* section,
                                    const char* name, const char* value) {
     INIReader* reader = (INIReader*)user;
     if (reader->_values[section][name].size() > 0) {
         throw std::runtime_error("duplicate key '" + std::string(name) +
                                  "' in section '" + section + "'.");
     }
     reader->_values[section][name] = value;
     return 1;
 }
 #endif  // __INIREADER__
 
 #ifndef __INIWRITER_H__
 #define __INIWRITER_H__
 
 class INIWriter {
    public:
     INIWriter(){};
     inline static void write(const std::string& filepath,
                              const INIReader& reader) {
         if (struct stat buf; stat(filepath.c_str(), &buf) == 0) {
             throw std::runtime_error("file: " + filepath + " already exist.");
         }
         std::ofstream out;
         out.open(filepath);
         if (!out.is_open()) {
             throw std::runtime_error("cannot open output file: " + filepath);
         }
         for (const auto& section : reader.Sections()) {
             out << "[" << section << "]\n";
             for (const auto& key : reader.Keys(section)) {
                 out << key << "=" << reader.Get(section, key) << "\n";
             }
         }
         out.close();
     }
 };
 }
 #endif /* __INIWRITER_H__ */
