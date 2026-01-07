#pragma once

/**
 * \file GDataCollection.h
 * \brief Defines the GDataCollection class for storing hit data for a detector.
 *
 * A GDataCollection holds collections of true hit data (GTrueInfoData) and
 * digitized hit data (GDigitizedData) for a single sensitive detector.
 */

#include "gTrueInfoData.h"
#include "gDigitizedData.h"

// c++
#include <memory>
#include <vector>


class GDataCollection
{
public:
    /**
     * \brief Constructs a GDataCollection.
     */
    explicit GDataCollection() = default;


    /**
     * \brief Destructor for GDataCollection.
     *
     * Smart pointers clean up automatically.
     */



    /**
     * \brief Collects true information hit data.
     * This copies the data
     * \param data Unique pointer to GDigitizedData.
     */
    void collectTrueInfosData(const std::unique_ptr<GTrueInfoData>& data) {
        // first event
        if (trueInfosData.empty()) {
            trueInfosData.push_back(std::make_unique<GTrueInfoData>(*data));
        }
        else {
            for (const auto& [varName, value] : data->getDoubleVariablesMap()) {
                trueInfosData.front()->accumulateVariable(varName, value);
            }
        }
    }

    /**
     * \brief Collects digitized hit data.
     * This copies the data
     * \param data Unique pointer to GDigitizedData.
     */
    void collectDigitizedData(const std::unique_ptr<GDigitizedData>& data) {
        // first event
        if (digitizedData.empty()) {
            digitizedData.push_back(std::make_unique<GDigitizedData>(*data));
        }
        else {
            // argument passed: 0: do not get SRO var
            for (const auto& [varName, value] : data->getIntObservablesMap(0)) {
                digitizedData.front()->accumulateVariable(varName, value);
            }
            for (const auto& [varName, value] : data->getDblObservablesMap(0)) {
                digitizedData.front()->accumulateVariable(varName, value);
            }
        }
    }


    /**
     * \brief Adds digitized hit data.
     * This moves ownership
     * \param data Unique pointer to GDigitizedData.
     */
    void addDigitizedData(std::unique_ptr<GDigitizedData> data) {
        digitizedData.push_back(std::move(data)); // taking ownership of the unique_ptr
    }

    /**
     * \brief Adds true information hit data.
     * This moves ownership
     * \param data Unique pointer to GTrueInfoData.
     */
    void addTrueInfoData(std::unique_ptr<GTrueInfoData> data) {
        trueInfosData.push_back(std::move(data)); // taking ownership of the unique_ptr
    }

    /**
     * \brief Provides read-only access to the stored true hit data.
     *
     * Returns a constant reference to the internal vector of unique pointers
     * to GTrueInfoData objects. Ownership of the data remains with this class.
     * Callers may inspect the data via the pointers but must not modify or
     * take ownership of them.
     *
     * \return Const reference to the vector of unique_ptr<GTrueInfoData>.
     */
    [[nodiscard]] auto getTrueInfoData() const -> const std::vector<std::unique_ptr<GTrueInfoData>>& {
        return trueInfosData;
    }

    /**
     * \brief Provides read-only access to the stored digitized hit data.
     *
     * Returns a constant reference to the internal vector of unique pointers
     * to GDigitizedData objects. Ownership of the data remains with this class.
     * Callers may read the data but must not modify or transfer ownership.
     *
     * \return Const reference to the vector of unique_ptr<GDigitizedData>.
     */
    [[nodiscard]] auto getDigitizedData() const -> const std::vector<std::unique_ptr<GDigitizedData>>& {
        return digitizedData;
    }

private:
    // for event data, the array index is the hit index
    // for run data, the array contains one item
    std::vector<std::unique_ptr<GTrueInfoData>>  trueInfosData; ///< Vector of true data.
    std::vector<std::unique_ptr<GDigitizedData>> digitizedData; ///< Vector of digitized data.
};
