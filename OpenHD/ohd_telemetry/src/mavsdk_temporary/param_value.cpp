#include "param_value.h"

#include <cassert>

namespace mavsdk {

// std::to_string doesn't work for std::string, so we need this workaround.
template <typename T>
std::string to_string(T&& value) {
  return std::to_string(std::forward<T>(value));
}

inline std::string& to_string(std::string& value) { return value; }

bool ParamValue::set_from_mavlink_param_value_bytewise(
    const mavlink_param_value_t& mavlink_value) {
  switch (mavlink_value.param_type) {
    case MAV_PARAM_TYPE_UINT8: {
      uint8_t temp;
      memcpy(&temp, &mavlink_value.param_value, sizeof(temp));
      _value = temp;
    } break;

    case MAV_PARAM_TYPE_INT8: {
      int8_t temp;
      memcpy(&temp, &mavlink_value.param_value, sizeof(temp));
      _value = temp;
    } break;

    case MAV_PARAM_TYPE_UINT16: {
      uint16_t temp;
      memcpy(&temp, &mavlink_value.param_value, sizeof(temp));
      _value = temp;
    } break;

    case MAV_PARAM_TYPE_INT16: {
      int16_t temp;
      memcpy(&temp, &mavlink_value.param_value, sizeof(temp));
      _value = temp;
    } break;

    case MAV_PARAM_TYPE_UINT32: {
      uint32_t temp;
      memcpy(&temp, &mavlink_value.param_value, sizeof(temp));
      _value = temp;
    } break;

    case MAV_PARAM_TYPE_INT32: {
      int32_t temp;
      memcpy(&temp, &mavlink_value.param_value, sizeof(temp));
      _value = temp;
    } break;

    case MAV_PARAM_TYPE_REAL32: {
      _value = mavlink_value.param_value;
    } break;

    default:
      // This would be worrying
      LogErr() << "Error: unknown mavlink param type: "
               << std::to_string(mavlink_value.param_type);
      return false;
  }
  return true;
}

bool ParamValue::set_from_mavlink_param_value_cast(
    const mavlink_param_value_t& mavlink_value) {
  switch (mavlink_value.param_type) {
    case MAV_PARAM_TYPE_UINT8: {
      _value = static_cast<uint8_t>(mavlink_value.param_value);
    } break;

    case MAV_PARAM_TYPE_INT8: {
      _value = static_cast<int8_t>(mavlink_value.param_value);
    } break;

    case MAV_PARAM_TYPE_UINT16: {
      _value = static_cast<uint16_t>(mavlink_value.param_value);
    } break;

    case MAV_PARAM_TYPE_INT16: {
      _value = static_cast<int16_t>(mavlink_value.param_value);
    } break;

    case MAV_PARAM_TYPE_UINT32: {
      _value = static_cast<uint32_t>(mavlink_value.param_value);
    } break;

    case MAV_PARAM_TYPE_INT32: {
      _value = static_cast<int32_t>(mavlink_value.param_value);
    } break;

    case MAV_PARAM_TYPE_REAL32: {
      float temp = mavlink_value.param_value;
      _value = temp;
    } break;

    default:
      // This would be worrying
      LogErr() << "Error: unknown mavlink param type: "
               << std::to_string(mavlink_value.param_type);
      return false;
  }
  return true;
}

bool ParamValue::set_from_mavlink_param_value(
    const mavlink_param_value_t& mavlink_value,
    const ParamValue::Conversion& conversion) {
  if (conversion == Conversion::CAST) {
    return set_from_mavlink_param_value_cast(mavlink_value);
  } else {
    return set_from_mavlink_param_value_bytewise(mavlink_value);
  }
}

bool ParamValue::set_from_mavlink_param_set_bytewise(
    const mavlink_param_set_t& mavlink_set) {
  mavlink_param_value_t mavlink_value{};
  mavlink_value.param_value = mavlink_set.param_value;
  mavlink_value.param_type = mavlink_set.param_type;

  return set_from_mavlink_param_value_bytewise(mavlink_value);
}

bool ParamValue::set_from_mavlink_param_ext_set(
    const mavlink_param_ext_set_t& mavlink_ext_set) {
  switch (mavlink_ext_set.param_type) {
    case MAV_PARAM_EXT_TYPE_UINT8: {
      uint8_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT8: {
      int8_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_UINT16: {
      uint16_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT16: {
      int16_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_UINT32: {
      uint32_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT32: {
      int32_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_UINT64: {
      uint64_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT64: {
      int64_t temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_REAL32: {
      float temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_REAL64: {
      double temp;
      memcpy(&temp, &mavlink_ext_set.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_CUSTOM: {
      std::size_t len =
          std::min(std::size_t(128), strlen(mavlink_ext_set.param_value));
      _value = std::string(mavlink_ext_set.param_value,
                           mavlink_ext_set.param_value + len);
    } break;
    default:
      // This would be worrying
      LogErr() << "Error: unknown mavlink ext param type";
      assert(false);
      return false;
  }
  return true;
}

// FIXME: this function can collapse with the one above.
bool ParamValue::set_from_mavlink_param_ext_value(
    const mavlink_param_ext_value_t& mavlink_ext_value) {
  switch (mavlink_ext_value.param_type) {
    case MAV_PARAM_EXT_TYPE_UINT8: {
      uint8_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT8: {
      int8_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_UINT16: {
      uint16_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT16: {
      int16_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_UINT32: {
      uint32_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT32: {
      int32_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_UINT64: {
      uint64_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_INT64: {
      int64_t temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_REAL32: {
      float temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_REAL64: {
      double temp;
      memcpy(&temp, &mavlink_ext_value.param_value[0], sizeof(temp));
      _value = temp;
    } break;
    case MAV_PARAM_EXT_TYPE_CUSTOM: {
      std::size_t len =
          std::min(std::size_t(128), strlen(mavlink_ext_value.param_value));
      _value = std::string(mavlink_ext_value.param_value,
                           mavlink_ext_value.param_value + len);
    } break;
    default:
      // This would be worrying
      LogErr() << "Error: unknown mavlink ext param type";
      assert(false);
      return false;
  }
  return true;
}

bool ParamValue::set_from_xml(const std::string& type_str,
                              const std::string& value_str) {
  if (strcmp(type_str.c_str(), "uint8") == 0) {
    _value = static_cast<uint8_t>(std::stoi(value_str));
  } else if (strcmp(type_str.c_str(), "int8") == 0) {
    _value = static_cast<int8_t>(std::stoi(value_str));
  } else if (strcmp(type_str.c_str(), "uint16") == 0) {
    _value = static_cast<uint16_t>(std::stoi(value_str));
  } else if (strcmp(type_str.c_str(), "int16") == 0) {
    _value = static_cast<int16_t>(std::stoi(value_str));
  } else if (strcmp(type_str.c_str(), "uint32") == 0) {
    _value = static_cast<uint32_t>(std::stoi(value_str));
  } else if (strcmp(type_str.c_str(), "int32") == 0) {
    _value = static_cast<int32_t>(std::stoi(value_str));
  } else if (strcmp(type_str.c_str(), "uint64") == 0) {
    _value = static_cast<uint64_t>(std::stoll(value_str));
  } else if (strcmp(type_str.c_str(), "int64") == 0) {
    _value = static_cast<int64_t>(std::stoll(value_str));
  } else if (strcmp(type_str.c_str(), "float") == 0) {
    _value = static_cast<float>(std::stof(value_str));
  } else if (strcmp(type_str.c_str(), "double") == 0) {
    _value = static_cast<double>(std::stod(value_str));
  } else {
    LogErr() << "Unknown type: " << type_str;
    return false;
  }
  return true;
}

bool ParamValue::set_empty_type_from_xml(const std::string& type_str) {
  if (strcmp(type_str.c_str(), "uint8") == 0) {
    _value = uint8_t(0);
  } else if (strcmp(type_str.c_str(), "int8") == 0) {
    _value = int8_t(0);
  } else if (strcmp(type_str.c_str(), "uint16") == 0) {
    _value = uint16_t(0);
  } else if (strcmp(type_str.c_str(), "int16") == 0) {
    _value = int16_t(0);
  } else if (strcmp(type_str.c_str(), "uint32") == 0) {
    _value = uint32_t(0);
  } else if (strcmp(type_str.c_str(), "int32") == 0) {
    _value = int32_t(0);
  } else if (strcmp(type_str.c_str(), "uint64") == 0) {
    _value = uint64_t(0);
  } else if (strcmp(type_str.c_str(), "int64") == 0) {
    _value = int64_t(0);
  } else if (strcmp(type_str.c_str(), "float") == 0) {
    _value = 0.0f;
  } else if (strcmp(type_str.c_str(), "double") == 0) {
    _value = 0.0;
  } else {
    LogErr() << "Unknown type: " << type_str;
    return false;
  }
  return true;
}

[[nodiscard]] MAV_PARAM_TYPE ParamValue::get_mav_param_type() const {
  if (std::get_if<uint8_t>(&_value)) {
    return MAV_PARAM_TYPE_UINT8;
  } else if (std::get_if<int8_t>(&_value)) {
    return MAV_PARAM_TYPE_INT8;
  } else if (std::get_if<uint16_t>(&_value)) {
    return MAV_PARAM_TYPE_UINT16;
  } else if (std::get_if<int16_t>(&_value)) {
    return MAV_PARAM_TYPE_INT16;
  } else if (std::get_if<uint32_t>(&_value)) {
    return MAV_PARAM_TYPE_UINT32;
  } else if (std::get_if<int32_t>(&_value)) {
    return MAV_PARAM_TYPE_INT32;
  } else if (std::get_if<uint64_t>(&_value)) {
    return MAV_PARAM_TYPE_UINT64;
  } else if (std::get_if<int64_t>(&_value)) {
    return MAV_PARAM_TYPE_INT64;
  } else if (std::get_if<float>(&_value)) {
    return MAV_PARAM_TYPE_REAL32;
  } else if (std::get_if<double>(&_value)) {
    return MAV_PARAM_TYPE_REAL64;
  } else {
    LogErr() << "Unknown data type for param.";
    assert(false);
    return MAV_PARAM_TYPE_INT32;
  }
}

[[nodiscard]] MAV_PARAM_EXT_TYPE ParamValue::get_mav_param_ext_type() const {
  if (std::get_if<uint8_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_UINT8;
  } else if (std::get_if<int8_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_INT8;
  } else if (std::get_if<uint16_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_UINT16;
  } else if (std::get_if<int16_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_INT16;
  } else if (std::get_if<uint32_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_UINT32;
  } else if (std::get_if<int32_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_INT32;
  } else if (std::get_if<uint64_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_UINT64;
  } else if (std::get_if<int64_t>(&_value)) {
    return MAV_PARAM_EXT_TYPE_INT64;
  } else if (std::get_if<float>(&_value)) {
    return MAV_PARAM_EXT_TYPE_REAL32;
  } else if (std::get_if<double>(&_value)) {
    return MAV_PARAM_EXT_TYPE_REAL64;
  } else if (std::get_if<std::string>(&_value)) {
    return MAV_PARAM_EXT_TYPE_CUSTOM;
  } else {
    LogErr() << "Unknown data type for param.";
    assert(false);
    return MAV_PARAM_EXT_TYPE_INT32;
  }
}

bool ParamValue::set_as_same_type(const std::string& value_str) {
  if (std::get_if<uint8_t>(&_value)) {
    _value = uint8_t(std::stoi(value_str));
  } else if (std::get_if<int8_t>(&_value)) {
    _value = int8_t(std::stoi(value_str));
  } else if (std::get_if<uint16_t>(&_value)) {
    _value = uint16_t(std::stoi(value_str));
  } else if (std::get_if<int16_t>(&_value)) {
    _value = int16_t(std::stoi(value_str));
  } else if (std::get_if<uint32_t>(&_value)) {
    _value = uint32_t(std::stoi(value_str));
  } else if (std::get_if<int32_t>(&_value)) {
    _value = int32_t(std::stoi(value_str));
  } else if (std::get_if<uint64_t>(&_value)) {
    _value = uint64_t(std::stoll(value_str));
  } else if (std::get_if<int64_t>(&_value)) {
    _value = int64_t(std::stoll(value_str));
  } else if (std::get_if<float>(&_value)) {
    _value = float(std::stof(value_str));
  } else if (std::get_if<double>(&_value)) {
    _value = double(std::stod(value_str));
  } else {
    LogErr() << "Unknown type";
    return false;
  }
  return true;
}

[[nodiscard]] float ParamValue::get_4_float_bytes_bytewise() const {
  if (std::get_if<float>(&_value)) {
    return std::get<float>(_value);
  } else if (std::get_if<int32_t>(&_value)) {
    return *(reinterpret_cast<const float*>(&std::get<int32_t>(_value)));
  } else {
    LogErr() << "Unknown type";
    assert(false);
    return NAN;
  }
}

[[nodiscard]] float ParamValue::get_4_float_bytes_cast() const {
  if (std::get_if<float>(&_value)) {
    return std::get<float>(_value);
  } else if (std::get_if<uint32_t>(&_value)) {
    return static_cast<float>(std::get<uint32_t>(_value));
  } else if (std::get_if<int32_t>(&_value)) {
    return static_cast<float>(std::get<int32_t>(_value));
  } else if (std::get_if<uint16_t>(&_value)) {
    return static_cast<float>(std::get<uint16_t>(_value));
  } else if (std::get_if<int16_t>(&_value)) {
    return static_cast<float>(std::get<int16_t>(_value));
  } else if (std::get_if<uint8_t>(&_value)) {
    return static_cast<float>(std::get<uint8_t>(_value));
  } else if (std::get_if<int8_t>(&_value)) {
    return static_cast<float>(std::get<int8_t>(_value));
  } else {
    LogErr() << "Unknown type";
    assert(false);
    return NAN;
  }
}

[[nodiscard]] std::optional<int> ParamValue::get_int() const {
  if (std::get_if<uint32_t>(&_value)) {
    return static_cast<int>(std::get<uint32_t>(_value));
  } else if (std::get_if<int32_t>(&_value)) {
    return static_cast<int>(std::get<int32_t>(_value));
  } else if (std::get_if<uint16_t>(&_value)) {
    return static_cast<int>(std::get<uint16_t>(_value));
  } else if (std::get_if<int16_t>(&_value)) {
    return static_cast<int>(std::get<int16_t>(_value));
  } else if (std::get_if<uint8_t>(&_value)) {
    return static_cast<int>(std::get<uint8_t>(_value));
  } else if (std::get_if<int8_t>(&_value)) {
    return static_cast<int>(std::get<int8_t>(_value));
  } else {
    LogErr() << "Not int type";
    return {};
  }
}

bool ParamValue::set_int(int new_value) {
  if (std::get_if<uint8_t>(&_value)) {
    _value = static_cast<uint8_t>(new_value);
    return true;
  } else if (std::get_if<int8_t>(&_value)) {
    _value = static_cast<int8_t>(new_value);
    return true;
  } else if (std::get_if<uint16_t>(&_value)) {
    _value = static_cast<uint16_t>(new_value);
    return true;
  } else if (std::get_if<int16_t>(&_value)) {
    _value = static_cast<int16_t>(new_value);
    return true;
  } else if (std::get_if<uint32_t>(&_value)) {
    _value = static_cast<uint32_t>(new_value);
    return true;
  } else if (std::get_if<int32_t>(&_value)) {
    _value = static_cast<int32_t>(new_value);
    return true;
  } else {
    return false;
  }
}

void ParamValue::set_float(float new_value) { _value = new_value; }

void ParamValue::set_custom(const std::string& new_value) {
  _value = new_value;
}

[[nodiscard]] std::optional<float> ParamValue::get_float() const {
  if (std::get_if<float>(&_value)) {
    return std::get<float>(_value);
  } else {
    LogErr() << "Not float type";
    return {};
  }
}

[[nodiscard]] std::optional<std::string> ParamValue::get_custom() const {
  if (std::get_if<std::string>(&_value)) {
    return std::get<std::string>(_value);
  } else {
    LogErr() << "Not custom type";
    return {};
  }
}

std::array<char, 128> ParamValue::get_128_bytes() const {
  std::array<char, 128> bytes{};

  if (std::get_if<uint8_t>(&_value)) {
    memcpy(bytes.data(), &std::get<uint8_t>(_value), sizeof(uint8_t));
  } else if (std::get_if<int8_t>(&_value)) {
    memcpy(bytes.data(), &std::get<int8_t>(_value), sizeof(int8_t));
  } else if (std::get_if<uint16_t>(&_value)) {
    memcpy(bytes.data(), &std::get<uint16_t>(_value), sizeof(uint16_t));
  } else if (std::get_if<int16_t>(&_value)) {
    memcpy(bytes.data(), &std::get<int16_t>(_value), sizeof(int16_t));
  } else if (std::get_if<uint32_t>(&_value)) {
    memcpy(bytes.data(), &std::get<uint32_t>(_value), sizeof(uint32_t));
  } else if (std::get_if<int32_t>(&_value)) {
    memcpy(bytes.data(), &std::get<int32_t>(_value), sizeof(int32_t));
  } else if (std::get_if<uint64_t>(&_value)) {
    memcpy(bytes.data(), &std::get<uint64_t>(_value), sizeof(uint64_t));
  } else if (std::get_if<int64_t>(&_value)) {
    memcpy(bytes.data(), &std::get<int64_t>(_value), sizeof(int64_t));
  } else if (std::get_if<float>(&_value)) {
    memcpy(bytes.data(), &std::get<float>(_value), sizeof(float));
  } else if (std::get_if<double>(&_value)) {
    memcpy(bytes.data(), &std::get<double>(_value), sizeof(double));
  } else if (std::get_if<std::string>(&_value)) {
    auto str_ptr = &std::get<std::string>(_value);
    // Copy all data in string, max 128 bytes
    memcpy(bytes.data(), str_ptr->data(),
           std::min(bytes.size(), str_ptr->size()));
  } else {
    LogErr() << "Unknown type";
    assert(false);
  }

  return bytes;
}

[[nodiscard]] std::string ParamValue::get_string() const {
  return std::visit([](auto value) { return to_string(value); }, _value);
}

[[nodiscard]] bool ParamValue::is_same_type(const ParamValue& rhs) const {
  if ((std::get_if<uint8_t>(&_value) && std::get_if<uint8_t>(&rhs._value)) ||
      (std::get_if<int8_t>(&_value) && std::get_if<int8_t>(&rhs._value)) ||
      (std::get_if<uint16_t>(&_value) && std::get_if<uint16_t>(&rhs._value)) ||
      (std::get_if<int16_t>(&_value) && std::get_if<int16_t>(&rhs._value)) ||
      (std::get_if<uint32_t>(&_value) && std::get_if<uint32_t>(&rhs._value)) ||
      (std::get_if<int32_t>(&_value) && std::get_if<int32_t>(&rhs._value)) ||
      (std::get_if<uint64_t>(&_value) && std::get_if<uint64_t>(&rhs._value)) ||
      (std::get_if<int64_t>(&_value) && std::get_if<int64_t>(&rhs._value)) ||
      (std::get_if<float>(&_value) && std::get_if<float>(&rhs._value)) ||
      (std::get_if<double>(&_value) && std::get_if<double>(&rhs._value)) ||
      (std::get_if<std::string>(&_value) &&
       std::get_if<std::string>(&rhs._value))) {
    return true;
  } else {
    LogWarn() << "Comparison type mismatch between " << typestr() << " and "
              << rhs.typestr();
    return false;
  }
}

bool ParamValue::operator==(const std::string& value_str) const {
  // LogDebug() << "Compare " << value_str() << " and " << rhs.value_str();
  if (std::get_if<uint8_t>(&_value)) {
    return std::get<uint8_t>(_value) == std::stoi(value_str);
  } else if (std::get_if<int8_t>(&_value)) {
    return std::get<int8_t>(_value) == std::stoi(value_str);
  } else if (std::get_if<uint16_t>(&_value)) {
    return std::get<uint16_t>(_value) == std::stoi(value_str);
  } else if (std::get_if<int16_t>(&_value)) {
    return std::get<int16_t>(_value) == std::stoi(value_str);
  } else if (std::get_if<uint32_t>(&_value)) {
    return std::get<uint32_t>(_value) == std::stoul(value_str);
  } else if (std::get_if<int32_t>(&_value)) {
    return std::get<int32_t>(_value) == std::stol(value_str);
  } else if (std::get_if<uint64_t>(&_value)) {
    return std::get<uint64_t>(_value) == std::stoull(value_str);
  } else if (std::get_if<int64_t>(&_value)) {
    return std::get<int64_t>(_value) == std::stoll(value_str);
  } else if (std::get_if<float>(&_value)) {
    return std::get<float>(_value) == std::stof(value_str);
  } else if (std::get_if<double>(&_value)) {
    return std::get<double>(_value) == std::stod(value_str);
  } else {
    // This also covers custom_type_t
    return false;
  }
}

std::ostream& operator<<(std::ostream& strm, const ParamValue& obj) {
  strm << "ParamValue{" << obj.typestr() << ":" << obj.get_string() << "}";
  return strm;
}

[[nodiscard]] std::string ParamValue::typestr() const {
  if (std::get_if<uint8_t>(&_value)) {
    return "uint8_t";
  } else if (std::get_if<int8_t>(&_value)) {
    return "int8_t";
  } else if (std::get_if<uint16_t>(&_value)) {
    return "uint16_t";
  } else if (std::get_if<int16_t>(&_value)) {
    return "int16_t";
  } else if (std::get_if<uint32_t>(&_value)) {
    return "uint32_t";
  } else if (std::get_if<int32_t>(&_value)) {
    return "int32_t";
  } else if (std::get_if<uint64_t>(&_value)) {
    return "uint64_t";
  } else if (std::get_if<int64_t>(&_value)) {
    return "int64_t";
  } else if (std::get_if<float>(&_value)) {
    return "float";
  } else if (std::get_if<double>(&_value)) {
    return "double";
  } else if (std::get_if<std::string>(&_value)) {
    return "custom";
  }
  // FIXME: Added to fix CI error (control reading end of non-void function)
  return "unknown";
}

void ParamValue::update_value_typesafe(const ParamValue& new_value) {
  assert(this->is_same_type(new_value));
  _value = new_value._value;
}

}  // namespace mavsdk