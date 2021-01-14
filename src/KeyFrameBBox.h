/**
 * @file
 * @brief Header file for the KeyFrameBBox class
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @ref License
 */

/* LICENSE
 *
 * Copyright (c) 2008-2019 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPENSHOT_BBOXKEYFRAME_H
#define OPENSHOT_BBOXKEYFRAME_H

#include <iostream>
#include <iomanip>
#include <cmath>
#include <assert.h>
#include <vector>
#include "Exceptions.h"
#include "Fraction.h"
#include "Coordinate.h"
#include "Point.h"
#include "Json.h"
#include "KeyFrame.h"
#include "KeyFrameBase.h"
#include "protobuf_messages/trackerdata.pb.h"
#include <google/protobuf/util/time_util.h>


using google::protobuf::util::TimeUtil;

namespace openshot
{
    /**
	 * @brief This struct holds the information of a bounding-box: a rectangular shape that enclosures an object or a 
     * desired set of pixels in a digital image.
	 *
	 * The bounding-box structure holds four floating-point properties: the x and y coordinates of the rectangle's
     * top left corner (x1, y1), the rectangle's width and the rectangle's height.
	 */

    struct BBox
    {
        float cx = -1; ///< x-coordinate of the bounding box center
        float cy = -1; ///< y-coordinate of the bounding box center
        float width = -1; ///< bounding box width
        float height = -1; ///< bounding box height
        float angle = -1; ///< bounding box rotation angle [degrees]

        /// Blank constructor
        BBox()
        {
            return;
        }

        /// Default constructor, which takes the bounding box top-left corner coordinates, width and height.
        /// @param _cx X-coordinate of the bounding box center
        /// @param _cy Y-coordinate of the bounding box center
        /// @param _width Bounding box width
        /// @param _height Bounding box height
        /// @param _angle Bounding box rotation angle [degrees]
        BBox(float _cx, float _cy, float _width, float _height, float _angle)
        {
            cx = _cx;
            cy = _cy;
            width = _width;
            height = _height;
            angle = _angle;
        }

        
        /// Generate JSON string of this object
        std::string Json() const
        {
            return JsonValue().toStyledString();
        }

        /// Generate Json::Value for this object
        Json::Value JsonValue() const
        {
            Json::Value root;
            root["cx"] = cx;
            root["cy"] = cy;
            root["width"] = width;
            root["height"] = height;
            root["angle"] = angle;

            return root;
        }

        /// Load JSON string into this object
        void SetJson(const std::string value)
        {
            // Parse JSON string into JSON objects
            try
            {
                const Json::Value root = openshot::stringToJson(value);
                // Set all values that match
                SetJsonValue(root);
            }
            catch (const std::exception &e)
            {
                // Error parsing JSON (or missing keys)
                throw InvalidJSON("JSON is invalid (missing keys or invalid data types)");
            }
        }

        /// Load Json::Value into this object
        void SetJsonValue(const Json::Value root)
        {

            // Set data from Json (if key is found)
            if (!root["cx"].isNull())
                cx = root["cx"].asDouble();
            if (!root["cy"].isNull())
                cy = root["cy"].asDouble();
            if (!root["width"].isNull())
                width = root["width"].asDouble();
            if (!root["height"].isNull())
                height = root["height"].asDouble();
            if (!root["angle"].isNull())
                angle = root["angle"].asDouble();
        } 
    };

    /**
	 * @brief This class holds the information of a bounding-box (mapped by time) over the frames that contain
     * the object enclosured by it.
	 *
     * The bounding-box displacement in X and Y directions and it's width and height variation over the frames
     * are set as openshot::Keyframe objects
     * 
	 * The bounding-box information over the clip's frames are saved into a protobuf file and loaded into an
     * object of this class. 
	 */    

    class KeyFrameBBox : public KeyframeBase
    {
    private:
        bool visible;
        Fraction BaseFps;
        double TimeScale;  

    public:
        std::map<double, BBox> BoxVec; ///< Index the bounding-box by time of each frame
        Keyframe delta_x; ///< X-direction displacement Keyframe
        Keyframe delta_y; ///< Y-direction displacement Keyframe
        Keyframe scale_x; ///< X-direction scale Keyframe
        Keyframe scale_y; ///< Y-direction scale Keyframe
        Keyframe rotation; ///< Rotation Keyframe
        std::string protobufDataPath; ///< Path to the protobuf file that holds the bbox points across the frames

        /// Default Constructor
        KeyFrameBBox();

        /// Add a BBox to the BoxVec map
        void AddBox(int64_t _frame_num, float _cx, float _cy, float _width, float _height, float _angle);
        
        /// Update object's BaseFps
        void SetBaseFPS(Fraction fps);

        /// Return the object's BaseFps
        Fraction GetBaseFPS();

        /// Update the TimeScale member variable
        void ScalePoints(double scale) override;

        /// Check if there is a bounding-box in the given frame
        bool Contains(int64_t frame_number);

        /// Get the size of BoxVec map
        int64_t GetLength() const;

        /// Remove a bounding-box from the BoxVec map
        void RemoveBox(int64_t frame_number);

        /// Return a bounding-box from BoxVec with it's properties adjusted by the Keyframes
        BBox GetBox(int64_t frame_number) const
        {
            return const_cast<KeyFrameBBox *>(this)->GetBox(frame_number);
        }
        BBox GetBox(int64_t frame_number);

        /// Load the bounding-boxes information from the protobuf file 
        bool LoadBoxData(std::string inputFilePath);

        /// Get the time of the given frame
        double FrameNToTime(int64_t frame_number, double time_scale);

        /// Interpolate the bouding-boxes properties
        BBox InterpolateBoxes(double t1, double t2, BBox left, BBox right, double target);

        /// Clear the BoxVec map
        void clear(); 

        /// Get and Set JSON methods
        std::string Json() const;                  ///< Generate JSON string of this object
        Json::Value JsonValue() const;             ///< Generate Json::Value for this object
        void SetJson(const std::string value);     ///< Load JSON string into this object
        void SetJsonValue(const Json::Value root); ///< Load Json::Value into this object

        /// Get all properties for a specific frame (perfect for a UI to display the current state
		/// of all properties at any time)
        Json::Value PropertiesJSON(int64_t requested_frame) const;

        // Generate JSON for a property
        Json::Value add_property_json(std::string name, float value, std::string type, std::string memo, const Keyframe* keyframe, float min_value, float max_value, bool readonly, int64_t requested_frame) const;

        /// Return the bounding box properties and it's keyframes indexed by their names
        std::map<std::string, float> GetBoxValues(int64_t frame_number) override; 
    
    };
} // namespace openshot

#endif