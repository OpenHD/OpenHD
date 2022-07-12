#pragma once

#include "param_value.h"
#include <map>
#include <mutex>
#include <vector>

namespace mavsdk{

// This class provides convenient methods to handle a set of mavlink parameters.
// Its public methods are written for the following premises:
// 1) Once a parameter has been added, its type can not be mutated anymore.
// 2) Once a parameter has been added, it cannot be removed.
// This restriction makes sense when viewed from both a mavlink parameter server and client perspective:
// Changing the type (not value) of a parameter (aka a Setting) most likely was a programming mistake
// and would easily lead to bugs / crashes.
class MavlinkParameterSet{
public:
    /**
     * add a new parameter to the parameter set, as long as the parameter does not exist yet, there is space available and
     * the param_id is not empty.
     * @return true on success, false otherwise.
     */
    bool add_new_parameter(const std::string& param_id,ParamValue value);
    /**
     * Possible return codes for performing a update operation on an existing parameter.
     */
    enum class UpdateExistingParamResult{
        SUCCESS,
        MISSING_PARAM,
        WRONG_PARAM_TYPE
    };
    friend std::ostream& operator<<(std::ostream& strm, const MavlinkParameterSet::UpdateExistingParamResult& obj);
    /**
     * update the value of an already existing parameter, as long as current and provided type match.
     * Does not add the parameter as a new parameter if missing.
     * @return one of the results above.
     */
    UpdateExistingParamResult update_existing_parameter(const std::string& param_id,const ParamValue& value);
    // This is how we publicly expose parameters - with a unique param_id as well as a unique param_index.
    // The param_index can be different on extended or non-extended protocol.
    struct Parameter{
        // unique parameter id
        const std::string param_id;
        // unique parameter index
        const uint16_t param_index;
        // value of this parameter.
        ParamValue value;
    };
    std::vector<Parameter> list_all_parameters(bool supports_extended);
    std::map<std::string, ParamValue> create_copy_as_map();
    // lookup a parameter using the unique string id
    std::optional<Parameter> lookup_parameter(const std::string& param_id,bool extended);
    // lookup a parameter using the unique unsigned int index
    std::optional<Parameter> lookup_parameter(uint16_t param_index,bool extended);
    // identifier can be either a string or index.
    using ParamIdentifier=std::variant<std::string,std::uint16_t>;
    std::optional<Parameter> lookup_parameter(const ParamIdentifier & identifier,bool extended);
    // we don't want to pollute the public api with a data structure that could also be used for something different.
    static std::string param_identifier_to_string(const ParamIdentifier& param_identifier);
    // Mavlink uses uint16_t for parameter indices, which allows for that many parameters maximum
    static constexpr auto MAX_N_PARAMETERS= 65535;
    /*
     * Return the n of parameters, either from an extended or non-extended perspective.
     * ( we need to hide parameters that need extended from non-extended queries).
     * Doesn't acquire the all-parameters lock, since when used it should already be locked.
     */
    [[nodiscard]] uint16_t get_current_parameters_count(bool extended);
public:
    // These methods are not necessarily related to this class, but shared between sender and receiver
    // Params can be up to 16 chars without 0-termination.
    static constexpr size_t PARAM_ID_LEN = 16;
    // add the null terminator if needed. Type-safety impossible since mavlink lib is c only.
    static std::string extract_safe_param_id(const char* param_id);
    // create a buffer that is long enough for the message pack to read from. Discards the null terminator
    // if the param_id is exactly PARAM_ID_LEN long.
    static std::array<char,PARAM_ID_LEN> param_id_to_message_buffer(const std::string& param_id);
    // returns true if the given param id is a valid param id for the mavlink protocol
    static bool validate_param_id(const std::string& param_id);
private:
    struct InternalParameter{
        // unique parameter id
        const std::string param_id;
        // value of this parameter.
        ParamValue value;
    };
    friend std::ostream& operator<<(std::ostream& strm, const MavlinkParameterSet::InternalParameter& obj);
    std::mutex _all_params_mutex{};
    // list of all the parameters added,not checked for extended/non-extended protocol
    std::vector<InternalParameter> _all_params;
    // if an element exists in this map, since we never remove parameters, it is guaranteed that the returned index is
    // inside the _all_params range.
    std::map<std::string,uint16_t> _param_id_to_idx;
    // This really messed up my brain,but no other way around - we need to be able to convert a parameter index from the
    // extended perspective into the non-extended perspective.
    std::vector<uint16_t> _param_index_to_hidden_extended;
    // parameter count from a non-extended perspective (string params hidden)
    uint16_t param_count_non_extended=0;
    const bool enable_debugging=true;
};
std::ostream& operator<<(std::ostream& strm, const MavlinkParameterSet::Parameter& obj);


// This class helps to build a complete parameter set with messages from a server.
class ParamSetFromServer{
public:
    // Add a new parameter to the parameter set.
    // returns true if the message is okay, false otherwise
    // (we might get inconsistent data from a buggy param server, an "all param synchronization" is not possible in this case).
    bool add_new_parameter(const std::string& safe_param_id,uint16_t param_index,uint16_t parameter_count,const ParamValue& value);
    // returns true if we know how many parameters the server provides.
    // (We've gotten at least one message).
    [[nodiscard]] bool param_count_known()const{
        return _server_all_param_ids.has_value();
    }
    // total number of parameters (once known)
    [[nodiscard]] uint16_t total_param_count()const{
        assert(_server_all_param_ids.has_value());
        return _server_all_param_ids.value().size();
    }
    // total number of missing parameters
    [[nodiscard]] uint16_t missing_param_count()const{
        return get_missing_param_indices().size();
    }
    // returns true if the parameter set is complete.
    [[nodiscard]] bool is_complete()const;
    // get the indices for all the parameters that are still missing.
    [[nodiscard]] std::vector<uint16_t> get_missing_param_indices()const;
    // temporary, make sure to check for completeness first.
    [[nodiscard]] std::map<std::string, ParamValue> get_all_params()const{
        return _all_params;
    }
    // create a string representation of the current state, for debugging.
    [[nodiscard]] std::string to_string()const;
    // remove all cached parameters as well as the parameter count. This might be needed when dealing with a server
    // that doesn't fully follow the recommended mavlink quidelines and has a invariant parameter set.
    void clear(){
        _all_params.clear();
        _server_all_param_ids=std::nullopt;
    }
    // On the client side, we only need to lookup parameters by their string id.
    std::optional<ParamValue> lookup_parameter(const std::string& param_id);
private:
    // filled as parameters come in
    std::map<std::string, ParamValue> _all_params{};
    // Once we got the first message with a parameter count, this becomes a valid vector
    // of size == param count from server and filled with std::nullopt.
    // Once no element in this vector is of type std::nullopt anymore, we know all the string param ids from the server.
    // once the parameter count has been set, it should not change - but we cannot say for certain since
    // the server might do whatever he wants.
    std::optional<std::vector<std::optional<std::string>>> _server_all_param_ids=std::nullopt;
};

}
