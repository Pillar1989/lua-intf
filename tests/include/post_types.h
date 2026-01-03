#pragma once

#include <vector>
#include <algorithm>

namespace PostLib {

/**
 * Box represents a detected object with bounding box and classification.
 * Used in YOLO postprocessing pipeline.
 */
class Box {
public:
    float x1, y1, x2, y2;       // Bounding box coordinates
    float confidence;            // Detection confidence score
    int class_id;                // Class ID

    Box() : x1(0), y1(0), x2(0), y2(0), confidence(0), class_id(0) {}
    
    Box(float x1, float y1, float x2, float y2, float conf, int cls)
        : x1(x1), y1(y1), x2(x2), y2(y2), confidence(conf), class_id(cls) {}
    
    // Lua-accessible getters (read-only from Lua)
    float getX1() const { return x1; }
    float getY1() const { return y1; }
    float getX2() const { return x2; }
    float getY2() const { return y2; }
    float getConfidence() const { return confidence; }
    int getClassId() const { return class_id; }
    
    // Area calculation for NMS
    float area() const {
        return (x2 - x1) * (y2 - y1);
    }
    
    // IoU calculation for NMS
    float iou(const Box& other) const {
        float inter_x1 = std::max(x1, other.x1);
        float inter_y1 = std::max(y1, other.y1);
        float inter_x2 = std::min(x2, other.x2);
        float inter_y2 = std::min(y2, other.y2);
        
        if (inter_x2 <= inter_x1 || inter_y2 <= inter_y1) {
            return 0.0f;
        }
        
        float inter_area = (inter_x2 - inter_x1) * (inter_y2 - inter_y1);
        float union_area = area() + other.area() - inter_area;
        
        return inter_area / union_area;
    }
};

} // namespace PostLib
