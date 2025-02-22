//
//  main.cpp
//  Assignment4-RobotsAndPartPicking
//
//  Created by SeanRen on 2019/12/20.
//
////////////////////////////////////////////////////////////////////////
//
//   Harvard University
//   CS175 : Computer Graphics
//   Professor Steven Gortler
//
////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 * Original source template from book source site based on GLUT
 * Modified to use GLFW as a window manager by SeanRen
 * Corresponding codes just tested on Mac OS X and iOS
 *****************************************************************************/

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <dirent.h>


//--------------------------------------------------------------------------------
//  GLEW的作用就是将合适的OpenGL版本对应的头文件引入，以方便调用对应的OpenGL功能。若不想使用glew
//  则在对应OS上引入合适的OpenGL头文件即可
//--------------------------------------------------------------------------------
#include <GL/glew.h>
//#include <OpenGL/gl3.h>
//#include <OpenGL/gl3ext.h>
#include <GLFW/glfw3.h>

#include "cvec.h"
#include "matrix4.h"
#include "quat.h"
#include "rigtform.h"
#include "arcball.h"
#include "ppm.h"
#include "glsupport.h"
#include "asstcommon.h"
#include "scenegraph.h"
#include "drawer.h"
#include "picker.h"

#include "geometry.h"
#include "material.h"

#include "perfMonitor.h"

#include "mesh.h"

using namespace std;      // for string, vector, iostream, and other standard C++ stuff
static void pick();

extern shared_ptr<Material> g_overridingMaterial;

// G L O B A L S ///////////////////////////////////////////////////

static const float g_frustMinFov = 60.0;  // A minimal of 60 degree field of view
static float g_frustFovY = g_frustMinFov; // FOV in y direction (updated by updateFrustFovY)

static const float g_frustNear = -0.1;    // near plane
static const float g_frustFar = -50.0;    // far plane

static const float g_groundY = -2.0;      // y coordinate of the ground
static const float g_groundSize = 10.0;   // half the ground length

static int g_windowWidth = 512;
static int g_windowHeight = 512;

static bool g_mouseClickDown = false;    // is the mouse button pressed
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static int g_mouseClickX, g_mouseClickY,g_pickingMouseX,g_pickingMouseY; // coordinates for mouse click event

static bool g_pickingFlag = false;
static int g_activeEyeFrame = 1;

static GLFWwindow* window;
static float globalElapseTime;
static const float TIME_INTERVAL = 5.0f;

// --------- Materials
static shared_ptr<Material> g_redDiffuseMat,
g_blueDiffuseMat,
g_bumpFloorMat,
g_arcballMat,
g_pickingMat,
g_lightMat,
g_subdivisionMat; //the material used by mesh&subdivision assignment

//global material vaiable to support picking style drawing
shared_ptr<Material> g_overridingMaterial = NULL;

// --------- Geometry
typedef SgGeometryShapeNode<Geometry> MyShapeNode; //the basic shapeNode used by this project

// ===================================================================
// Declare the scene graph and pointers to suitable nodes in the scene
// graph
// ===================================================================

//basic scene variable
static shared_ptr<SgRootNode> g_world;
static shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node,g_subvisionNode;
static shared_ptr<SgRbtNode> g_currentPickedRbtNode; // used later when you do picking

vector<VertexPN> g_divisionVtx;

#pragma mark - Scene Variables

// Buffered Geometry Object instances
static shared_ptr<SimpleIndexedGeometryPNTBX> g_ground, g_cube, g_sphere;
static shared_ptr<SimpleGeometryPN>g_subdivisionMesh;

static Mesh g_OrignCubeMesh;
static Mesh g_CubeMesh;

// define two lights positions in world space=
static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);

// used as an eye frame
static RigTForm g_skyRbt = RigTForm::makeTranslation(0.0, 0.25, 4.0);

//初始tramsformation，将object frame的原点保持不动，每个cube使用一个object matrix。由于在shader中使用了offset，故此处对象帧的起点都为原点。
static RigTForm g_objectRbt[3] = {RigTForm(Cvec3(0,0,0)),RigTForm(Cvec3(0,0,0)),RigTForm(Cvec3(0,0,0))};

static RigTForm g_auxiliaryRbt;

static const float g_sphereRaidusScreenRatio = 0.3;
static float g_arcballScale;
static float g_arcballScreenRadius = g_sphereRaidusScreenRatio * min(g_windowWidth,g_windowHeight);
static bool g_arcballUpdateFlag = true;

static RigTForm g_motionRbt;

//for new assignments - variables
static Cvec3f g_objectColors[3] = {Cvec3f(1, 0, 0),Cvec3f(0, 0, 1),Cvec3f(0.5, 0.5, 0)};

//texture objects
shared_ptr<Texture> colorTex,normalTex;

///////////////// END OF G L O B A L S //////////////////////////////////////////////////

//--------------------------------------------------------------------------------
//  GLFW global variables
//--------------------------------------------------------------------------------
GLfloat cursor_x        = 0.f;
GLfloat cursor_y        = 0.f;

#pragma mark - Geometry Helper Function

//======================================================================
// STEP 3: Replace initGround(), initCube(), and  initSphere() functions
//         with the following defintion. This ensures VertexPNTBX and
//         SimpleIndexedGeometryPNTBX are used, which provides extra
//         vertex attributes used for Bump Mapping later
//=======================================================================
static void initGround() {
    int ibLen, vbLen;
    getPlaneVbIbLen(vbLen, ibLen);
    
    // Temporary storage for cube Geometry
    vector<VertexPNTBX> vtx(vbLen);
    vector<unsigned short> idx(ibLen);
    
    makePlane(g_groundSize*2, vtx.begin(), idx.begin());
    //此处new关键字的使用，会出现no viable operator ‘=’的提示
    g_ground.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initCubes() {
    int ibLen, vbLen;
    getCubeVbIbLen(vbLen, ibLen);
    
    // Temporary storage for cube Geometry
    vector<VertexPNTBX> vtx(vbLen);
    vector<unsigned short> idx(ibLen);
    
    makeCube(1, vtx.begin(), idx.begin());
    g_cube.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initSphere() {
    int ibLen, vbLen;
    getSphereVbIbLen(20, 10, vbLen, ibLen);
    
    // Temporary storage for sphere Geometry
    vector<VertexPNTBX> vtx(vbLen);
    vector<unsigned short> idx(ibLen);
    makeSphere(1, 20, 10, vtx.begin(), idx.begin());
    g_sphere.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vtx.size(), idx.size()));
}

static void initSubDivisionCube() {
    g_OrignCubeMesh.load("./shaders/cube.mesh");
    g_CubeMesh = g_OrignCubeMesh;
    
    int nVertices = g_OrignCubeMesh.getNumVertices();
    
    int nFaces = g_OrignCubeMesh.getNumFaces();
    
    
    
    //The valence of a vertex is simply the number of edges that use that vertex. For a regular quadrilateral mesh the valence of each vertex will be four.
    //calculate averate normal
//    Cvec3 normal(0,0,0);
//    for(int i=0;i<nFaces;i++){
//        Mesh::Face face = mesh.getFace(i);
//        //int nFaceVtx = face.getNumVertices();
//        normal = normal + face.getNormal();
//
//        for(int j=0;j<face.getNumVertices();j++){
//            face.getVertex(j).setNormal(normal);
//        }
//
//    }
    
    //The valence of a vertex is simply the number of edges that use that vertex. For a regular quadrilateral mesh the valence of each vertex will be four.
    //calculate average normal at a vertex
    //this approach may have some logical problems
//    for (int i = 0; i < mesh.getNumVertices(); ++i) {
//        const Mesh::Vertex v = mesh.getVertex(i);
//
//        Cvec3 normal(0,0,0);
//        Mesh::VertexIterator it(v.getIterator()), it0(it);
//        do{
//            //[...]                                               // can use here it.getVertex(), it.getFace()
//            //Mesh::Vertex vv=it.getVertex();
//            Mesh::Face vf=it.getFace();
//            normal = normal + vf.getNormal();
//        }while (++it != it0);                                  // go around once the 1ring
//
//        v.setNormal(normal);
//    }
    
    //So actually the two average normal calculation approach has the same effect
    Cvec3 zeroNormal(0,0,0);
    for (int i = 0; i < nVertices; ++i) {
        const Mesh::Vertex v = g_OrignCubeMesh.getVertex(i);
        v.setNormal(zeroNormal);
    }

    for(int i=0;i<nFaces;i++){
        Mesh::Face face = g_OrignCubeMesh.getFace(i);
        double vtxValence = face.getNumVertices();

        for(int j=0;j<vtxValence;j++){
            Mesh::Vertex v=face.getVertex(j);
            v.setNormal(v.getNormal()+face.getNormal());
        }
    }
    
    //for task 1
//    //each face is composed by a quad,so we need to dice them into two triangles
//    vector<VertexPN> vtx;
//    for(int i=0;i<nFaces;i++){
//        Mesh::Face face = mesh.getFace(i);
//        //int nFaceVtx = face.getNumVertices();
//
//        //for first triangle
//        VertexPN vtxPN0(face.getVertex(0).getPosition(),face.getNormal());
//        vtx.push_back(vtxPN0);
//        VertexPN vtxPN1(face.getVertex(1).getPosition(),face.getNormal());
//        vtx.push_back(vtxPN1);
//        VertexPN vtxPN2(face.getVertex(2).getPosition(),face.getNormal());
//        vtx.push_back(vtxPN2);
//
//        //for second triangle
//        vtx.push_back(vtxPN0);
//        vtx.push_back(vtxPN2);
//        VertexPN vtxPN3(face.getVertex(3).getPosition(),face.getNormal());
//        vtx.push_back(vtxPN3);
//    }
    
    //int nEdges = mesh.getNumEdges();
    
    // for task 2
    //each face is composed by a quad,so we need to dice them into two triangles
    for(int i=0;i<nFaces;i++){
        Mesh::Face face = g_OrignCubeMesh.getFace(i);
        double vtxValence = face.getNumVertices();
        
        //for first triangle
        VertexPN vtxPN0(face.getVertex(0).getPosition(),face.getVertex(0).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN0);
        VertexPN vtxPN1(face.getVertex(1).getPosition(),face.getVertex(1).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN1);
        VertexPN vtxPN2(face.getVertex(2).getPosition(),face.getVertex(2).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN2);
        
        //for second triangle
        g_divisionVtx.push_back(vtxPN0);
        g_divisionVtx.push_back(vtxPN2);
        VertexPN vtxPN3(face.getVertex(3).getPosition(),face.getVertex(3).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN3);
    }
    
    g_subdivisionMesh.reset(new SimpleGeometryPN(&g_divisionVtx[0], (int)g_divisionVtx.size()));
}

static void animateVerticesOfSubdivision(){
    static float amplify = 1.8;
    vector<VertexPN> vtx;
    int size = (int)g_divisionVtx.size();
    for(int i=0;i<size;i++){
        float scale=sin((CS175_PI)*(globalElapseTime/TIME_INTERVAL)) * amplify;
        VertexPN vtxPN = g_divisionVtx[i];
        vtx.push_back(VertexPN(vtxPN.p*scale,vtxPN.n));
    }
    g_subdivisionMesh->upload(&vtx[0], (int)vtx.size());
}

static void subdivide() {
    g_divisionVtx.clear();
    
    static const int VTX_VALENCE = 4;
    
    //compute new face vertex geometries
    int nFaces = g_CubeMesh.getNumFaces();
    for(int i=0;i<nFaces;i++){
        Mesh::Face face = g_CubeMesh.getFace(i);
        double vtxValence = face.getNumVertices();
        
        Cvec3 accumVtxPos;
        for(int j=0;j<vtxValence;j++){
            Mesh::Vertex v=face.getVertex(j);
            accumVtxPos += v.getPosition();
        }
        g_CubeMesh.setNewFaceVertex(face, accumVtxPos/vtxValence);
    }
    
    //compute edge vertex geometries
    int nEdges = g_CubeMesh.getNumEdges();
    for(int i=0;i<nEdges;i++){
        Mesh::Edge edge= g_CubeMesh.getEdge(i);
        Mesh::Face eface0 = edge.getFace(0);
        Mesh::Face eface1 = edge.getFace(1);
        Mesh::Vertex evtx0 = edge.getVertex(0);
        Mesh::Vertex evtx1 = edge.getVertex(0);
        
        Cvec3 accumPos = g_CubeMesh.getNewFaceVertex(eface0) + g_CubeMesh.getNewFaceVertex(eface1) + evtx0.getPosition() + evtx1.getPosition();
        
        g_CubeMesh.setNewEdgeVertex(edge, accumPos/VTX_VALENCE);
    }
    
    
    int nVertices = g_CubeMesh.getNumVertices();
    //compute edge vertex geometries
    for (int i = 0; i < nVertices; ++i) {
        const Mesh::Vertex v = g_CubeMesh.getVertex(i);
        
        Cvec3 accumVVPos;
        Cvec3 accumFVPos;
        Mesh::VertexIterator it(v.getIterator()), it0(it);
        do{
            //can use here it.getVertex(), it.getFace()
            Mesh::Vertex vv=it.getVertex();
            Mesh::Face vf=it.getFace();
            
            accumVVPos += vv.getPosition();
            accumFVPos += g_CubeMesh.getNewFaceVertex(vf);
        }while (++it != it0);// go around once the 1ring
        
        g_CubeMesh.setNewVertexVertex(v, v.getPosition()/2 + accumVVPos/16 + accumVVPos/16);
    }
    
    g_CubeMesh.subdivide();
    
    //flat shading data
//    nFaces = g_CubeMesh.getNumFaces();
//    for(int i=0;i<nFaces;i++){
//        Mesh::Face face = g_CubeMesh.getFace(i);
//
//        //for first triangle
//        VertexPN vtxPN0(face.getVertex(0).getPosition(),face.getNormal());
//        g_divisionVtx.push_back(vtxPN0);
//        VertexPN vtxPN1(face.getVertex(1).getPosition(),face.getNormal());
//        g_divisionVtx.push_back(vtxPN1);
//        VertexPN vtxPN2(face.getVertex(2).getPosition(),face.getNormal());
//        g_divisionVtx.push_back(vtxPN2);
//
//        //for second triangle
//        g_divisionVtx.push_back(vtxPN0);
//        g_divisionVtx.push_back(vtxPN2);
//        VertexPN vtxPN3(face.getVertex(3).getPosition(),face.getNormal());
//        g_divisionVtx.push_back(vtxPN3);
//    }
    
    // smooth shading
    Cvec3 zeroNormal(0,0,0);
    for (int i = 0; i < nVertices; ++i) {
        const Mesh::Vertex v = g_CubeMesh.getVertex(i);
        v.setNormal(zeroNormal);
    }

    for(int i=0;i<g_CubeMesh.getNumFaces();i++){
        Mesh::Face face = g_CubeMesh.getFace(i);
        double vtxValence = face.getNumVertices();

        for(int j=0;j<vtxValence;j++){
            Mesh::Vertex v=face.getVertex(j);
            v.setNormal(v.getNormal()+face.getNormal());
        }
    }
    //calculate average normals by vertex iterator
//    for (int i = 0; i < g_CubeMesh.getNumFaces(); ++i) {
//        const Mesh::Vertex v = g_CubeMesh.getVertex(i);
//
//        Cvec3 normal(0,0,0);
//        Mesh::VertexIterator it(v.getIterator()), it0(it);
//        do{
//            Mesh::Face vf=it.getFace();
//            normal = normal + vf.getNormal();
//        }while (++it != it0);     // go around once the 1ring
//
//        v.setNormal(normal);
//    }
    //each face is composed by a quad,so we need to dice them into two triangles
    for(int i=0;i<g_CubeMesh.getNumFaces();i++){
        Mesh::Face face = g_CubeMesh.getFace(i);
        double vtxValence = face.getNumVertices();

        //for first triangle
        VertexPN vtxPN0(face.getVertex(0).getPosition(),face.getVertex(0).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN0);
        VertexPN vtxPN1(face.getVertex(1).getPosition(),face.getVertex(1).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN1);
        VertexPN vtxPN2(face.getVertex(2).getPosition(),face.getVertex(2).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN2);

        //for second triangle
        g_divisionVtx.push_back(vtxPN0);
        g_divisionVtx.push_back(vtxPN2);
        VertexPN vtxPN3(face.getVertex(3).getPosition(),face.getVertex(3).getNormal()/vtxValence);
        g_divisionVtx.push_back(vtxPN3);
    }
    
    g_subdivisionMesh->upload(&g_divisionVtx[0], (int)g_divisionVtx.size());
}

static void initMaterials(){
    Material diffuse("./shaders/basic-gl3.vshader","./shaders/diffuse-gl3.fshader");
    Material solid("./shaders/basic-gl3.vshader","./shaders/solid-gl3.fshader");
    Material texture("./shaders/basic-gl3.vshader","./shaders/texture-gl3.fshader");
    
    Material bump("./shaders/normalmap.vs.glsl","./shaders/normalmap.fs.glsl");
    
    Material subdivision("./shaders/subdivision.vs.glsl","./shaders/specular-gl3.fshader");
    
    g_redDiffuseMat.reset(new Material(diffuse));
    g_blueDiffuseMat.reset(new Material(diffuse));
    //g_bumpFloorMat,
    g_arcballMat.reset(new Material(solid));
    g_arcballMat->getUniforms().put("uColor", Cvec3(0.97f,0.1f,0.0f));
    g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    g_subdivisionMat.reset(new Material(subdivision));
    g_subdivisionMat->getUniforms().put("uColor", Cvec3(0.1f,0.0f,0.57f));
    g_subdivisionMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    g_lightMat.reset(new Material(bump));
    g_lightMat->getUniforms().put("uTexColor",colorTex);
    g_lightMat->getUniforms().put("uTexNormal",normalTex);
    g_lightMat->getRenderStates().polygonMode(GL_FRONT, GL_FILL);
    g_lightMat->getUniforms().put("uColor",Cvec3{0.1,1,0.1});
    
    g_pickingMat.reset(new Material("./shaders/normal-gl3.vshader","./shaders/pick-gl3.fshader"));
    
}

// update g_frustFovY from g_frustMinFov, g_windowWidth, and g_windowHeight
static void updateFrustFovY() {
    if (g_windowWidth >= g_windowHeight)
        g_frustFovY = g_frustMinFov;
    else {
        const double RAD_PER_DEG = 0.5 * CS175_PI/180;
        g_frustFovY = atan2(sin(g_frustMinFov * RAD_PER_DEG) * g_windowHeight / g_windowWidth, cos(g_frustMinFov * RAD_PER_DEG)) / RAD_PER_DEG;
    }
}

static Matrix4 makeProjectionMatrix() {
    return Matrix4::makeProjection(
                                   g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight),
                                   g_frustNear, g_frustFar);
}

//--------------------------------------------------------------------------------
//  compute screen to eye scale for object frame origin in eye frame
//--------------------------------------------------------------------------------
static float computeArcballScale(const Cvec4 objectRbtOrigin){
    float screenToEyeScale;
    double zCoord = objectRbtOrigin[2];
    screenToEyeScale = getScreenToEyeScale(zCoord, g_frustFovY, g_windowHeight);
    
    return screenToEyeScale;
}

static RigTForm getEyeRbt(){
    RigTForm eyeRbt;
    if(g_activeEyeFrame == 1){
        g_currentPickedRbtNode = g_skyNode;
        eyeRbt = getPathAccumRbt(g_world, g_skyNode);
    }else if(g_activeEyeFrame == 2){
        //when you get a path from g_world to g_robot1Node,the path is a direct path and just includes g_world,g_robot1Node.
        // make g_skyCamera RBT left multiply the path to get an appropriate view angle.
        g_currentPickedRbtNode = g_robot1Node;
        eyeRbt = getPathAccumRbt(g_world, g_skyNode) * getPathAccumRbt(g_world, g_robot1Node);
    }else{
        g_currentPickedRbtNode=g_robot2Node;
        eyeRbt = getPathAccumRbt(g_world, g_skyNode) * getPathAccumRbt(g_world, g_robot2Node);
    }
    return eyeRbt;
}


//=========================================================================
// STEP 5: Simplify your arcball drawing and drawStuff() by using materials
//         In particular, all occurence of
//             const ShaderState& curSS
//         should probably be replaced with
//             Uniforms& uniforms
//         and occurences of `curSS' be replaced with `uniforms'
//=========================================================================

//公用的属性可以放到extraUniforms中，可以供每个drawer使用。
//这样代码的结构才体现出简洁高效
//extraUniforms作为参数传递，意味着作为参数之前设置的参数更有通用性才对。
static void drawStuff(Uniforms& extraUniforms, bool picking){
    //glBindFramebuffer(GL_FRAMEBUFFER,fboId);
    
    // build & send proj. matrix to vshader
    const Matrix4 projmat = makeProjectionMatrix();
    sendProjectionMatrix(extraUniforms, projmat);
    
    
    // use the skyRbt as the eyeRbt
    const RigTForm eyeRbt = getEyeRbt();
    const RigTForm invEyeRbt = inv(eyeRbt);
    
//    RigTForm mvmRbt = invEyeRbt * g_objectRbt[2];
    
    const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1)); // g_light1 position in eye coordinates
    const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1)); // g_light2 position in eye coordinates
    
    //        g_lightMat->getUniforms().put("uLight", eyeLight1);
    //        g_lightMat->getUniforms().put("uLight2", eyeLight2);
    //
    //        g_redDiffuseMat->getUniforms().put("uLight", eyeLight1);
    //        g_redDiffuseMat->getUniforms().put("uLight2", eyeLight2);
    //
    //        g_blueDiffuseMat->getUniforms().put("uLight", eyeLight1);
    //        g_blueDiffuseMat->getUniforms().put("uLight2", eyeLight2);
    //
    //        sendProjectionMatrix(g_lightMat->getUniforms(), projmat);
    //
    //        sendProjectionMatrix(g_redDiffuseMat->getUniforms(), projmat);
    //        sendProjectionMatrix(g_blueDiffuseMat->getUniforms(), projmat);

    //公用的属性可以放到extraUniforms中，可以供每个drawer使用。这样才可以借用uniforms存储即用的uniforms变量
    extraUniforms.put("uLight", eyeLight1);
    extraUniforms.put("uLight2", eyeLight2);
    
    
    //draw arcball in wireframe
    RigTForm mvmRbt = invEyeRbt * g_objectRbt[2];
    g_arcballScale = computeArcballScale(Cvec4(mvmRbt.getTranslation(),0));
    float screenRadiusScale = g_arcballScreenRadius*g_arcballScale;
    Matrix4 scaleMatrix = Matrix4::makeScale(Cvec3(screenRadiusScale,screenRadiusScale,screenRadiusScale));
    Matrix4 MVM = rigTFormToMatrix(mvmRbt) * scaleMatrix;
    Matrix4 NMVM = normalMatrix(MVM);
    sendModelViewNormalMatrix(g_arcballMat->getUniforms(), MVM, NMVM);
    
    sendProjectionMatrix(g_arcballMat->getUniforms(), projmat);

    g_arcballMat->draw(*g_sphere, extraUniforms);
    
    
    //draw the cube supported by subdivision Mesh data structure
//    sendModelViewNormalMatrix(g_subdivisionMat->getUniforms(), MVM, NMVM);
//
//    sendProjectionMatrix(g_subdivisionMat->getUniforms(), projmat);
//    g_subdivisionMat->draw(*g_subdivisionMesh,extraUniforms);
    
    
    //Material sphereMat("./shaders/normal-gl3.vshader","./shaders/normal-gl3.fshader");
    if (!picking) {
        
//        Matrix4 MVM = rigTFormToMatrix(mvmRbt); //* scaleMatrix;
//        Matrix4 NMVM = normalMatrix(MVM);
//        sendModelViewNormalMatrix(g_lightMat->getUniforms(), MVM, NMVM);
//
//        sendModelViewNormalMatrix(g_redDiffuseMat->getUniforms(), MVM, NMVM);
//        sendModelViewNormalMatrix(g_blueDiffuseMat->getUniforms(), MVM, NMVM);

        //extraUniforms.put("uTexColor",colorTex);
        
        g_redDiffuseMat->getRenderStates().polygonMode(GL_FRONT, GL_FILL);
        g_blueDiffuseMat->getRenderStates().polygonMode(GL_FRONT, GL_FILL);
        //sphereMat.draw(*g_cube, extraUniforms);
        
        
        g_redDiffuseMat->getUniforms().put("uColor",Cvec3{0.9,0.1,0.1});
        g_blueDiffuseMat->getUniforms().put("uColor",Cvec3{0.1,1,0.1});
        
//        Drawer ground_drawer(invEyeRbt,*g_lightMat);
//        g_groundNode->accept(ground_drawer);
        
//        Drawer robot1_drawer(invEyeRbt,*g_arcballMat);
//        g_robot1Node->accept(robot1_drawer);
//        g_robot2Node->accept(ground_drawer);

        Drawer drawer(invEyeRbt,extraUniforms);
        g_world->accept(drawer);
        //BlitTextures();
        
    }else {

        Picker picker(invEyeRbt, extraUniforms,g_currentPickedRbtNode,g_world,getEyeRbt(),g_motionRbt);
        g_world->accept(picker);
        //BlitTextures();
        glFlush();
        g_currentPickedRbtNode = picker.getRbtNodeAtXY(g_pickingMouseX, g_pickingMouseY);
        if (g_currentPickedRbtNode == g_groundNode)
            g_currentPickedRbtNode = shared_ptr<SgRbtNode>();   // set to NULL
    }
}

static void display() {
    //glUseProgram(g_shaderStates[g_activeShader]->program);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // clear framebuffer color&depth
    
    //Todo,temprarily declaring an Unifroms variable
    Uniforms uniforms;
    
    if(g_pickingFlag){
        g_overridingMaterial = g_pickingMat;
        pick();
    }else{
        g_overridingMaterial = NULL;
        drawStuff(uniforms,false);
    }
    
}

static void reshape(GLFWwindow* window,const int w, const int h) {
    g_windowWidth = w;
    g_windowHeight = h;
    glViewport(0, 0, w, h);
    cerr << "Size of window is now " << w << "x" << h << endl;
    updateFrustFovY();
    g_arcballScreenRadius = g_sphereRaidusScreenRatio * min(g_windowWidth,g_windowHeight);
}

static void displayWindow(GLFWwindow* window){
    display();
}


static void motion(const float x, const float y) {
    if(x<0 || y<0)
        return;
    
    Cvec2 startScreenPos = Cvec2(g_mouseClickX,g_mouseClickY);
    Cvec2 endScreenPos = Cvec2(x,g_windowHeight - y - 1); //convert from window coordnate to OpenGL window coordinate.
    Cvec2 centerScreenPos = getScreenSpaceCoord(g_objectRbt[0].getTranslation(),makeProjectionMatrix(), 0.0, 0.0, g_windowWidth, g_windowHeight);
    Quat arcballQuat = arcball(Cvec3(centerScreenPos,0), g_arcballScreenRadius, startScreenPos, endScreenPos);
    
    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;
    
    g_arcballUpdateFlag = true;
    
    RigTForm m;
    if (g_mouseLClickButton && !g_mouseRClickButton) { // left button down?
  //    m = RigTForm::makeXRotation(-dy) * RigTForm::makeYRotation(dx);
        m = RigTForm(arcballQuat);
    }
    else if (g_mouseRClickButton && !g_mouseLClickButton) { // right button down?
        m = RigTForm(Cvec3(dx, dy, 0) * g_arcballScale/**0.01*/);
    }
    else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {  // middle or (left and right) button down?
        m = RigTForm(Cvec3(0, 0, -dy) * g_arcballScale /**0.01*/);
        g_arcballUpdateFlag = false;
    }
    
    
//    if (g_mouseClickDown) {
//        if(g_activeCube == 0){
//            //g_objectRbt[0] *= m; // Simply right-multiply is WRONG
//            g_auxiliaryRbt = makeMixedFrame(g_objectRbt[0], g_skyRbt);
//            g_objectRbt[0] = doQtoOwrtA(m, g_objectRbt[0], g_auxiliaryRbt);
//            
//        }else if(g_activeCube == 1){
//            g_auxiliaryRbt = makeMixedFrame(g_objectRbt[1], g_skyRbt);
//            g_objectRbt[1] = doQtoOwrtA(m, g_objectRbt[1], g_auxiliaryRbt);
//        }else{
//            RigTForm invMouseMotionMatrix = inv(m);
//            g_skyRbt = doQtoOwrtA(m, g_skyRbt, g_skyRbt);
//        }
        g_motionRbt = m;
//    }
    
    
    
    g_mouseClickX = x;
    g_mouseClickY = g_windowHeight - y - 1;
}

void cursor_position_callback( GLFWwindow* window, double x, double y )
{
    cursor_x = (float) x;
    cursor_y = (float) y;
}

static void mouse(GLFWwindow* window, const int button, const int action, int mods) {
    g_mouseClickX = cursor_x;
    g_mouseClickY = g_windowHeight - cursor_y - 1;  // conversion from GLFW window-coordinate-system to OpenGL window-coordinate-system
    g_mouseLClickButton=g_mouseMClickButton=g_mouseRClickButton=false;
    
    g_mouseLClickButton |= (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
    g_mouseRClickButton |= (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
    g_mouseMClickButton |= (button == GLFW_MOUSE_BUTTON_4 && action == GLFW_PRESS);
    
    g_mouseLClickButton &= !(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE);
    g_mouseRClickButton &= !(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE);
    g_mouseMClickButton &= !(button == GLFW_MOUSE_BUTTON_4 && action == GLFW_RELEASE);
    
    g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;
    
    if(g_mouseClickDown){
        g_pickingMouseX = g_mouseClickX;
        g_pickingMouseY = g_mouseClickY;
    }
    
    
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

//static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GLFW_TRUE);
//
//    if (key == GLFW_MOUSE_BUTTON_LEFT | GLFW_MOUSE_BUTTON_RIGHT | GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
//
//    }
//}

static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS){
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);//exit(0);   // ESC
            case GLFW_KEY_H:
                cout << " ============== H E L P ==============\n\n"
                << "h\t\thelp menu\n"
                << "s\t\tsave screenshot\n"
                << "f\t\tToggle flat shading on/off.\n"
                << "o\t\tCycle object to edit\n"
                << "p\t\tPart Picking\n"
                << "v\t\tCycle view\n"
                << "drag left mouse to rotate\n" << endl;
                break;
            case GLFW_KEY_S:
                glFlush();
                writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
                break;
            case GLFW_KEY_O:
//                g_activeCube++;
//                if(g_activeCube > 2)
//                    g_activeCube=0;
                break;
            case GLFW_KEY_P:
                g_pickingFlag ^= true;
                break;
            case GLFW_KEY_I:
                subdivide();
                break;
            case GLFW_KEY_V:
                g_activeEyeFrame++;
                if(g_activeEyeFrame > 3)
                    g_activeEyeFrame=1;
                break;
        }
    }
}

//initialize GLFW
static int initGlfwState(){
    if(!glfwInit())
        return -1;
    
    //设置OpenGL core profile
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    
    //创建本地窗口
    window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Robots and Part Picking", NULL, NULL);
    if(!window){
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    cout << "GLFW version                : " << glfwGetVersionString() << endl;
    cout << "GL_VERSION                  : " << glGetString( GL_VERSION ) << endl;
    cout << "GL_VENDOR                   : " << glGetString( GL_VENDOR ) << endl;
    cout << "GL_RENDERER                 : " << glGetString( GL_RENDERER ) << endl;
    cout << "GL_SHADING_LANGUAGE_VERSION : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;
    
    //鼠标指针位置回调
    glfwSetCursorPosCallback(window, cursor_position_callback);
    //鼠标动作回调设置
    glfwSetMouseButtonCallback(window, mouse);
    
    //窗口尺寸变化事件
    glfwSetFramebufferSizeCallback(window, reshape);
    //键盘事件回调
    glfwSetKeyCallback(window, keyboard);
    
    //error callback
    glfwSetErrorCallback(error_callback);
    
    return 0;
    
}

static void initGLState() {
    glClearColor(128./255., 200./255., 255./255., 0.);
    glClearDepth(0.f);
    //glClear(GL_DEPTH_BUFFER_BIT);
    
    //  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //  glPixelStorei(GL_PACK_ALIGNMENT, 1);
    //  glCullFace(GL_BACK);
    //  glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    //  glReadBuffer(GL_BACK);
    
    glEnable(GL_FRAMEBUFFER_SRGB);
}

static void initGeometry() {
    initGround();
    initCubes();
    initSphere();
    initSubDivisionCube();
    
    colorTex.reset(new ImageTexture("./shaders/Fieldstone.ppm",true));
    normalTex.reset(new ImageTexture("./shaders/FieldstoneNormal.ppm",true));
}

static void constructRobot(shared_ptr<SgTransformNode> base, shared_ptr<Material> material,const Cvec3& color) {
    
    const float ARM_LEN = 0.7,ARM_THICK = 0.25,TORSO_LEN = 1.5,TORSO_THICK = 0.25,TORSO_WIDTH = 1, HEAD_LEN = 0.5,HEAD_THICK = 0.45,LIMB_LEN = 0.9,LIMB_THICK = 0.25;
    const int NUM_JOINTS = 10,NUM_SHAPES = 10;
    
    struct JointDesc {
        int parent;
        float x, y, z;
    };
    
    //each joint is just a respected frame
    JointDesc jointDesc[NUM_JOINTS] = {
        {-1}, // torso
        {0, 0, TORSO_LEN/2 + 0.1f, 0}, // header
        {0,  TORSO_WIDTH/2, TORSO_LEN/2, 0}, // upper right arm
        {0,  -TORSO_WIDTH/2, TORSO_LEN/2, 0}, // upper left arm
        {2,  ARM_LEN, 0, 0}, // lower right arm
        {3,  -ARM_LEN, 0, 0}, // lower left arm
        {0,  TORSO_WIDTH/2 - LIMB_THICK/2, -TORSO_LEN/2, 0}, // upper right limb
        {0,  -TORSO_WIDTH/2 + LIMB_THICK/2, -TORSO_LEN/2, 0}, // upper left limb
        {6,  0, -LIMB_LEN, 0}, // lower right limb
        {7,  0, -LIMB_LEN, 0}, // lower left limb
    };
    
    struct ShapeDesc {
        int parentJointId;
        float x, y, z, sx, sy, sz;
        shared_ptr<SimpleIndexedGeometryPNTBX> geometry;
    };
    
    ShapeDesc shapeDesc[NUM_SHAPES] = {
        {0, 0,         0, 0, TORSO_WIDTH, TORSO_LEN, TORSO_THICK, g_cube}, // torso
        {1, 0, HEAD_LEN/2, 0, HEAD_LEN/2, HEAD_LEN/2, HEAD_THICK/2, g_sphere}, // head
        {2, ARM_LEN/2, 0, 0, ARM_LEN/2, ARM_THICK/2, ARM_THICK/2, g_sphere}, // upper right arm
        {3, -ARM_LEN/2, 0, 0, ARM_LEN/2, ARM_THICK/2, ARM_THICK/2, g_sphere}, // upper left arm
        {4, ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // lower right arm
        {5, -ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // lower left arm
        {6, LIMB_THICK/2 , -LIMB_LEN/2, 0, LIMB_THICK/2, LIMB_LEN/2, LIMB_THICK/2, g_sphere}, // upper right limb
        {7, -LIMB_THICK/2, -LIMB_LEN/2, 0, LIMB_THICK/2, LIMB_LEN/2, LIMB_THICK/2, g_sphere}, // upper left limb
        {8, LIMB_THICK/2, -LIMB_LEN/2, 0, LIMB_THICK, LIMB_LEN, LIMB_THICK, g_cube}, // lower right limb
        {9, -LIMB_THICK/2, -LIMB_LEN/2, 0, LIMB_THICK, LIMB_LEN, LIMB_THICK, g_cube}, // lower left limb
        
    };
    
    shared_ptr<SgTransformNode> jointNodes[NUM_JOINTS];
    
    for (int i = 0; i < NUM_JOINTS; ++i) {
        if (jointDesc[i].parent == -1)
            jointNodes[i] = base;
        else {
            jointNodes[i].reset(new SgRbtNode(RigTForm(Cvec3(jointDesc[i].x, jointDesc[i].y, jointDesc[i].z))));
            jointNodes[jointDesc[i].parent]->addChild(jointNodes[i]);
        }
    }
    for (int i = 0; i < NUM_SHAPES; ++i) {
        shared_ptr<MyShapeNode> shape(
                                      new MyShapeNode(shapeDesc[i].geometry,
                                                      material,
                                                      color,
                                                      Cvec3(shapeDesc[i].x, shapeDesc[i].y, shapeDesc[i].z),
                                                      Cvec3(0, 0, 0),
                                                      Cvec3(shapeDesc[i].sx, shapeDesc[i].sy, shapeDesc[i].sz)));
        jointNodes[shapeDesc[i].parentJointId]->addChild(shape);
    }
}

static void initScene() {
    g_world.reset(new SgRootNode());
    
    g_skyNode.reset(new SgRbtNode(RigTForm(Cvec3(0.0, 0.25, 4.0))));
    
    g_groundNode.reset(new SgRbtNode(RigTForm(Cvec3(0.0, -1.55, 4.0))));
    
    shared_ptr<SimpleIndexedGeometryPNTBX> g_ground2 = g_ground;
    g_groundNode->addChild(shared_ptr<MyShapeNode>(
                                                   new MyShapeNode(g_ground2,g_lightMat, Cvec3(0.1, 0.95, 0.1))));
    
    g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, 1, 0))));
    g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(2, 1, 0))));
    
    g_subvisionNode.reset(new SgRbtNode(RigTForm(Cvec3(0, 1, 0))*RigTForm::makeYRotation(50)));
    g_subvisionNode->addChild(shared_ptr<MyShapeNode>(
                                                      new MyShapeNode(g_subdivisionMesh,g_subdivisionMat, Cvec3(0.1, 0.95, 0.1))));
    
    constructRobot(g_robot1Node,g_redDiffuseMat, Cvec3(1, 0, 0)); // a Red robot
    constructRobot(g_robot2Node,g_blueDiffuseMat, Cvec3(0, 1, 0)); // a Blue robot
    
    g_world->addChild(g_skyNode);
    g_world->addChild(g_groundNode);
    g_world->addChild(g_subvisionNode);
    //g_world->addChild(g_robot1Node);
    //g_world->addChild(g_robot2Node);
}

static void pick() {
    // We need to set the clear color to black, for pick rendering.
    // so let's save the clear color
    GLdouble clearColor[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);
    
    glClearColor(0, 0, 0, 0);
    
    // using PICKING_SHADER as the shader
    //glUseProgram(g_shaderStates[PICKING_SHADER]->program);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    Uniforms extraUniforms;
    
    drawStuff(extraUniforms, true);
    
    
    // Uncomment below and comment out the glutPostRedisplay in mouse(...) call back
    // to see result of the pick rendering pass
    // glutSwapBuffers();
    
    //Now set back the clear color
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    
    checkGlError(__func__);
}

int main(int argc, char * argv[]) {
    try {
        //initGLFW
        initGlfwState();
        
        GLenum err = glewInit(); // load the OpenGL extensions
        
        if (GLEW_OK != err){
            /* Problem: glewInit failed, something is seriously wrong. */
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        }
        fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
        
        if(GLEW_VERSION_4_1)
            fprintf(stdout, "GLEW VERSION 4.1 Supported \n");
        
        
        initGLState();
        initGeometry();
        initMaterials();
        initScene();
        
        //      // open current directory:
        //      unique_ptr<DIR> pDir(opendir("."));
        //
        //      // process each directory entry:
        //
        //      struct dirent *dp;
        //      while ((dp = readdir(pDir.get())) != nullptr) {
        //
        //          string filename(dp->d_name);
        //
        //          cout << "process " << filename << endl;
        //      }
        
        glfwSetWindowRefreshCallback(window, displayWindow); //根据窗口刷新回调函数
        
        OpenGL_Helper::PerfMonitor perfMonitor;
        float fps;
        
        //set timer interval
        globalElapseTime = TIME_INTERVAL;
        float previous = glfwGetTime();
        
        //mac os x的窗口调用会限制帧数
        while( !glfwWindowShouldClose(window) ){
            animateVerticesOfSubdivision();
            
            display();
            perfMonitor.Update(fps);
            //cout << "Current FPS at timeinterval:" << glfwGetTime() << " is " << fps << endl;
            
            motion(cursor_x, cursor_y);
            
            // common part, do this only once
            float now = glfwGetTime();
            float delta = now - previous;
            previous = now;
            
            // for each timer do this
            globalElapseTime -= delta;
            if (globalElapseTime <= 0.f){
                cout << "Timer triggered: " << globalElapseTime << endl;
                
                globalElapseTime = TIME_INTERVAL;
            }

            
            glfwSwapBuffers( window );
            
            glfwPollEvents(); //this call will keep glfw application running,so if run a timer,this mode shuld be turned on
            
            //glfwWaitEvents(); //if window is put in background,then didn't return immediatetly until at least one available window event is call.
        }
        
        glfwTerminate();
        
        return 0;
    }
    catch (const runtime_error& e) {
        cout << "Exception caught: " << e.what() << endl;
        return -1;
    }
}

