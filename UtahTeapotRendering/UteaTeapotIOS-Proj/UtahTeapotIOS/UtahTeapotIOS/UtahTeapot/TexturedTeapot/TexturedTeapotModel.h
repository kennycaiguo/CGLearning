#ifndef TEXTUREDTEAPOTMODEL_H
#define TEXTUREDTEAPOTMODEL_H

#include <errno.h>
#include <vector>

#include "matrix4.h"
#include "shadersasst.h"

#include "Geometry.h"

#include "Texture.h"

#include "../MotionControl.h"


#define BUFFER_OFFSET(i) ((char*)NULL + (i))

struct TEAPOT_VERTEX {
    float pos[3];
    float normal[3];
};

enum SHADER_ATTRIBUTES {
    ATTRIB_VERTEX,
    ATTRIB_NORMAL,
    ATTRIB_UV,
};

struct TEAPOT_MATERIALS {
    float diffuse_color[3];
    float specular_color[4];
    float ambient_color[3];
};

class TexturedTeapotModel{
    int32_t num_indices_;
    int32_t num_vertices_;
    GLuint ibo_;
    GLuint vbo_;
    
    GLuint vao_;
    
    Texture* texObj_  = nullptr;
    
    std::shared_ptr<Geometry> geometry_;
    
    std::shared_ptr<TexturedTeapotShaderState> teapotShaderState_;
    
    virtual GLint GetTextureType(void);
    
    Matrix4 mat_projection_;
    Matrix4 mat_view_;
    Matrix4 mat_model_;
    Matrix4 mat_local_model_;
    Matrix4 mat_motion_matrix_;
    
    std::shared_ptr<MotionControl> motionControl_;
    
public:
    TexturedTeapotModel();
    virtual ~TexturedTeapotModel();
    void Init();
    void Render(float r, float g, float b);
    void Update(double time);
    void Unload();
    void UpdateViewport();
    void SetMotionControl(std::shared_ptr<MotionControl> mControl);
    
};

#endif
