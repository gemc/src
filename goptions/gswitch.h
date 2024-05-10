#ifndef  GSWITCH_H
#define  GSWITCH_H  1

using std::string;


/**
 * The GSwitch contains the switch description and its status
 */
class GSwitch {

public:

    /// empty constructor, needed by compiler to know how to handle unordered_map<string, GSwitch>
    GSwitch() { return; }

    /**
     * @brief Constructor, status is false at construction
     * \param d switch description
     */
    GSwitch(string d) : description(d), status(false) { return; }

    /// @brief turn on switch
    void turnOn() { status = true; }

    /// @brief get switch status
    bool getStatus() { return status; }

    string getDescription() { return description; }


private:
    string description;
    bool status;

};


#endif
