#ifndef CONFLIB_CPP_CONFLIB_H
#define CONFLIB_CPP_CONFLIB_H


#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>


namespace conflib {


static const std::string VERSION_ = "0.0.1";


#ifdef _WIN32
    static const std::string OS_PATHSEP_("\\");
#else
    static const std::string OS_PATHSEP_("/");
#endif


static const std::string DEFAULT_CONFIGURATION_NAME_ = "coderrect.json";
static const std::string HOME_CONFIGURATION_PATH_ = "~/.coderrect.json";
static const std::string ENV_INSTALLATION_DIRECTORY_ = "CODERRECT_HOME";


struct ConfLibException : public std::exception {
public:
    static const int UNKNOWN = 1;
    static const int INVALID_PARAMETER = 2;
    static const int INVALID_DATA_TYPE = 3;

public:
    ConfLibException(int code) : code_(code) {}
    ~ConfLibException() = default;

    int code() {
        return code_;
    }

private:
    int code_;
};


static jsoncons::json default_conf_;
static jsoncons::json home_conf_;
static jsoncons::json project_conf_;
static jsoncons::json custom_conf_;
static jsoncons::json cmdline_conf_;


/**
 * @param s is a key-value string like "-abc=def".
 * @return the value
 */
static inline std::string GetConfValue_(const char* s) {
    std::string str(s);
    size_t pos = str.find_first_of('=');
    if (pos == std::string::npos) {
        throw ConfLibException(ConfLibException::INVALID_PARAMETER);
    }

    return str.substr(pos+1);
}


static inline bool FileExists_(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}


static inline bool IsInteger_(const std::string& s) {
    char *p = nullptr;
    strtol(s.c_str(), &p, 10);
    return (*p == 0);
}


static inline bool IsDouble_(const std::string& s) {
    char *p = nullptr;
    strtof(s.c_str(), &p);
    return (*p == 0);
}


static inline void Split_(std::vector<std::string>& stages, std::string s) {
    size_t pos_of_first_char = 0;
    size_t pos = s.find_first_of('.');
    while (pos != std::string::npos) {
        stages.push_back(s.substr(pos_of_first_char, pos-pos_of_first_char));
        pos_of_first_char = pos + 1;
        pos = s.find_first_of('.', pos_of_first_char);
    }

    stages.push_back(s.substr(pos_of_first_char));
}

static inline std::string BuildPath_(const std::vector<std::string>& stages, int last_stage) {
    std::string s;
    for (int i = 0; i <= last_stage; i++) {
        if (!s.empty()) {
            s += '.' + stages[i];
        }
        else {
            s += stages[i];
        }
    }

    return "$." + s;
}


static inline void AssignValue_(jsoncons::json& j, const std::string& key, const std::string& value) {
    if (value == "true") {
        j[key] = true;
    }
    else if (value == "false") {
        j[key] = false;
    }
    else if (IsDouble_(value)) {
        j[key] = atof(value.c_str());
    }
    else if (IsInteger_(value)) {
        j[key] = atol(value.c_str());
    }
    else {
        j[key] = value;
    }
}


static inline void ReplaceValue_(jsoncons::json& j, const std::string& jpath, const std::string& value) {
    if (value == "true") {
        jsoncons::jsonpath::json_replace(j, jpath, true);
    }
    else if (value == "false") {
        jsoncons::jsonpath::json_replace(j, jpath, false);
    }
    else if (IsDouble_(value)) {
        jsoncons::jsonpath::json_replace(j, jpath, atof(value.c_str()));
    }
    else if (IsInteger_(value)) {
        jsoncons::jsonpath::json_replace(j, jpath, atol(value.c_str()));
    }
    else {
        jsoncons::jsonpath::json_replace(j, jpath, value);
    }
}

/**
 * 's' has the format like '-abc.def=ghi'. This method needs to
 * insert the value of 'ghi' into the json object 'j' at the location
 *
 * {
 *   "abc": {
 *     "def": "ghi"
 *   }
 * }
 *
 */
static inline void InsertConfiguration_(jsoncons::json& j, const std::string& s) {
    size_t pos = s.find_first_of('=');
    std::string key = s.substr(1, pos-1);
    std::string value = s.substr(pos+1);

    std::vector<std::string> stages;
    Split_(stages, key);

    jsoncons::json jtmp;
    for (int i = stages.size()-1; i>=0; i--) {
        std::string jpath = BuildPath_(stages, i);
        auto r = jsoncons::jsonpath::json_query(j, jpath);
        if (!r.empty()) {
            if (jtmp.empty()) {
                jsoncons::jsonpath::json_replace(j, jpath, value);
            }
            else {
                if (r[0].is_object()) {
                    jtmp.merge(std::move(r[0]));
                }
                jsoncons::jsonpath::json_replace(j, jpath, jtmp);
            }

            break;
        }
        else {
            if (jtmp.empty()) {
                AssignValue_(jtmp, stages[i], value);
            }
            else {
                if (i == 0) {
                    j[stages[i]] = jtmp;
                }
                else {
                    jsoncons::json jtmp2;
                    jtmp2[stages[i]] = jtmp;
                    jtmp = jtmp2;
                }
            }
        }
    }
}


static inline void LoadConfFile_(const std::string& conf_path, jsoncons::json& j) {
    if (!conf_path.empty() && FileExists_(conf_path)) {
        std::ifstream ins(conf_path);
        j = jsoncons::json::parse(ins);
    }
    else {
        j = jsoncons::json::parse("{}");
    }
}

/**
 * Assumes 'cwd' is the project directory.
 *
 * cmdline < custom < project < home < default
 */
void Initialize(int argc, char* argv[]) {
    std::string conf_path;

    // load the default configuration file
    char* home_dir = std::getenv(ENV_INSTALLATION_DIRECTORY_.c_str());
    if (home_dir != nullptr) {
        conf_path = std::string(home_dir) + OS_PATHSEP_ + "conf" + OS_PATHSEP_ + DEFAULT_CONFIGURATION_NAME_;
    }
    else {
        // we assume the base dir of the application is the installation dir
        // todo
    }
    LoadConfFile_(conf_path, default_conf_);

    // load the home configuration file
    LoadConfFile_(HOME_CONFIGURATION_PATH_, home_conf_);

    // load the project configuration file
    LoadConfFile_("./.coderrect.json", project_conf_);

    // load the custom configuration file
    conf_path = "";
    for ( int i = 1; i < argc; i++) {
        if (strstr(argv[i], "-conf=") == argv[i]) {
            conf_path = GetConfValue_(argv[i]);
            break;
        }
    }
    LoadConfFile_(conf_path, custom_conf_);

    // convert all cmdline parameters into a json
    for (int i = 1; i < argc; i++) {
        if (argv[i][0]!= '-' || strstr(argv[i], "-conf=") == argv[i] || argv[i][0] == '=')
            continue;

        std::string arg(argv[i]);
        size_t pos = arg.find_first_of('=');
        if (pos == std::string::npos) {
            // sth like -racedetector.enableFunction
            arg += "=true";
        }

        InsertConfiguration_(cmdline_conf_, arg);
    }
    std::cout << jsoncons::pretty_print(cmdline_conf_) << std::endl;
}



template <typename T>
T Get(const std::string& key, T defaultValue) {
    std::string jpath = "$." + key;

    jsoncons::json r = jsoncons::jsonpath::json_query(cmdline_conf_, jpath);
    if (!r.empty()) {
        return r[0].as<T>();
    }

    r = jsoncons::jsonpath::json_query(custom_conf_, jpath);
    if (!r.empty()) {
        return r[0].as<T>();
    }

    r = jsoncons::jsonpath::json_query(project_conf_, jpath);
    if (!r.empty()) {
        return r[0].as<T>();
    }

    r = jsoncons::jsonpath::json_query(home_conf_, jpath);
    if (!r.empty()) {
        return r[0].as<T>();
    }

    r = jsoncons::jsonpath::json_query(default_conf_, jpath);
    if (!r.empty()) {
        return r[0].as<T>();
    }

    return defaultValue;
}


} // namespace conflib


#endif //CONFLIB_CPP_CONFLIB_H
