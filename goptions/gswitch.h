#ifndef  GSWITCH_H
#define  GSWITCH_H  1

#include <string>

/**
 * The GSwitch contains the switch description and its status
 */
class GSwitch {

public:

//    /// empty constructor, needed by compiler to know how to handle unordered_map<string, GSwitch>
//    GSwitch() { return; }

    /// empty constructor, needed by compiler to know how to handle unordered_map<string, GSwitch>
    GSwitch() = default;

    /**
     * @brief Constructor, status is false at construction
     * \param d switch description
     */
    GSwitch(const string& d) : description(d), status(false) {}

    /// @brief turn on switch
    void turnOn() { status = true; }

    /// @brief get switch status
    bool getStatus() const { return status; }

    string getDescription() const { return description; }


private:
    string description;
    bool status;

};


#endif
