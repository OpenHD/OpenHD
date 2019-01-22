// MESSAGE ADAP_TUNING support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief ADAP_TUNING message
 *
 * Adaptive Controller tuning information.
 */
struct ADAP_TUNING : mavlink::Message {
    static constexpr msgid_t MSG_ID = 11010;
    static constexpr size_t LENGTH = 49;
    static constexpr size_t MIN_LENGTH = 49;
    static constexpr uint8_t CRC_EXTRA = 46;
    static constexpr auto NAME = "ADAP_TUNING";


    uint8_t axis; /*<  Axis. */
    float desired; /*< [deg/s] Desired rate. */
    float achieved; /*< [deg/s] Achieved rate. */
    float error; /*<  Error between model and vehicle. */
    float theta; /*<  Theta estimated state predictor. */
    float omega; /*<  Omega estimated state predictor. */
    float sigma; /*<  Sigma estimated state predictor. */
    float theta_dot; /*<  Theta derivative. */
    float omega_dot; /*<  Omega derivative. */
    float sigma_dot; /*<  Sigma derivative. */
    float f; /*<  Projection operator value. */
    float f_dot; /*<  Projection operator derivative. */
    float u; /*<  u adaptive controlled output command. */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  axis: " << +axis << std::endl;
        ss << "  desired: " << desired << std::endl;
        ss << "  achieved: " << achieved << std::endl;
        ss << "  error: " << error << std::endl;
        ss << "  theta: " << theta << std::endl;
        ss << "  omega: " << omega << std::endl;
        ss << "  sigma: " << sigma << std::endl;
        ss << "  theta_dot: " << theta_dot << std::endl;
        ss << "  omega_dot: " << omega_dot << std::endl;
        ss << "  sigma_dot: " << sigma_dot << std::endl;
        ss << "  f: " << f << std::endl;
        ss << "  f_dot: " << f_dot << std::endl;
        ss << "  u: " << u << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << desired;                       // offset: 0
        map << achieved;                      // offset: 4
        map << error;                         // offset: 8
        map << theta;                         // offset: 12
        map << omega;                         // offset: 16
        map << sigma;                         // offset: 20
        map << theta_dot;                     // offset: 24
        map << omega_dot;                     // offset: 28
        map << sigma_dot;                     // offset: 32
        map << f;                             // offset: 36
        map << f_dot;                         // offset: 40
        map << u;                             // offset: 44
        map << axis;                          // offset: 48
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> desired;                       // offset: 0
        map >> achieved;                      // offset: 4
        map >> error;                         // offset: 8
        map >> theta;                         // offset: 12
        map >> omega;                         // offset: 16
        map >> sigma;                         // offset: 20
        map >> theta_dot;                     // offset: 24
        map >> omega_dot;                     // offset: 28
        map >> sigma_dot;                     // offset: 32
        map >> f;                             // offset: 36
        map >> f_dot;                         // offset: 40
        map >> u;                             // offset: 44
        map >> axis;                          // offset: 48
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
