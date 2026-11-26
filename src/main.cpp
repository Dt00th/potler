#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <string>
#include <cmath>
#include <cstddef>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const double FPS = 60;
// SDL
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
// keys
const bool *keys = SDL_GetKeyboardState(NULL);

struct Point2D {
    double x, y;
};
struct Point3D {
    double x, y, z;
};
struct Object {
    struct vector<Point3D> vertices;
    vector<vector<int>> edges;
    Point3D pos = {0,0,0};
    vector<Point3D> normalv;
    double xangle; double yangle;
};
Object readfile(string path) {
    ifstream file(path);
    string text;
    //cout<<text.size()<<endl;
    string mode = "";
    Object object;
    int n = 0;
    int i = -1;
    while(getline(file, text)) {
        string value = "";
        Point3D vertex = {};
        i = -1;
        for(int j = 0; j < text.size(); j++) {
            if(text[j]=='v') {
                mode ="v";
            } else if(text[j]=='l') {
                if(mode=="v") {
                    n=0;
                }
                mode ="l";
            }
            if(mode=="v" && text[j]!='v') {
                if(text[j]==' ') {
                    if(i==0) { 
                        vertex.x = (stod(value));
                        value="";
                    } else if(i==1) { 
                        vertex.y = (stod(value));
                        value="";
                    } else if(i==2) { 
                        vertex.z = (stod(value));
                        value="";
                    }
                    i++;
                } else {
                    value+=text[j];
                }
            } else if(mode=="l"&&text[j]!='l') {
                if(text[j]==' ') {
                    //cout<<"value: "<<value<<", i:"<<i<<"\n";
                    if(i==-1) {
                        object.edges.push_back({0,0,0});
                    } else if(i>=0) {
                        object.edges[n][i] = (stoi(value));
                        value = "";
                        //cout<<"value is now null\n";
                    }
                    i++;
                } else {
                    value+=text[j];
                }
            }
        }
        object.vertices.push_back(vertex);
        n++;
    }
    file.close();
    return object;
}

// camera
bool console = false;
bool focus = true;
Point3D camerapos = {.x=0,.y=0,.z=-5};
double cameraspeed = 4.0f;
Point3D camerav = {.x=0,.y=0,.z=0};
double cxang = 0.0f; double cyang = 0.0f;
double xangle=0.0f;double yangle=0.0f;
double xanglev=0.0f;double yanglev=0.5f;
double projd=0.0f; double FOV = 60.0f;
double projscale=600.0f;
// world
vector<Object> objects;
Point3D lightv = {.x=1,.y=0,.z=0};

void makenormals() {
    for(int j = 0; j< objects.size();j++) {
        for(int i = 0; i < objects[j].edges.size();i++) { // for face
            Point3D p0 = objects[j].vertices[objects[j].edges[i][0]];
            Point3D p1 = objects[j].vertices[objects[j].edges[i][1]];
            Point3D p2 = objects[j].vertices[objects[j].edges[i][2]];
            double rx =(p1.y-p0.y)*(p2.z-p0.z)-(p1.z-p0.z)*(p2.y-p0.y);
            double ry =-(p1.x-p0.x)*(p2.z-p0.z)+(p1.z-p0.z)*(p2.x-p0.x);
            double rz =(p1.x-p0.x)*(p2.y-p0.y)-(p1.y-p0.y)*(p2.x-p0.x);
            double len = sqrt(rx*rx+ry*ry+rz*rz);
            Point3D normalv = {.x=rx/len, .y=ry/len, .z=rz/len};
            objects[j].normalv.push_back(normalv);
        }
    }
}

Point3D project(Point3D p) {
    double zoffset = p.z+projd;
    return {x:projscale*p.x/FOV/zoffset, y:projscale*p.y/FOV/zoffset, z:zoffset};
}

Point3D rotate(Point3D p, double anglex, double angley) {
    Point3D r = {.x=0,.y=0,.z=0};
    double cosx = cos(anglex);double sinx = sin(anglex);
    double cosy = cos(angley);double siny = sin(angley);
    double x1 = p.x*cosy-p.z*siny;
    double z1 = p.x*siny+p.z*cosy;
    r.x = x1; r.z = z1;

    double y2 = p.y*cosx-z1*sinx;
    double z2 = p.y*sinx+z1*cosx;
    r.y = y2; r.z = z2;
    return r;
}

void addt() {
    struct Object triangle = {
        .vertices={
            {.x=-.5,.y=0,.z=0},
            {.x=-0,.y=.5,.z=0},
            {.x=.5,.y=0,.z=0},
        },
        .edges={
            {0,1,2}
        }
    };
    objects.push_back(triangle);
}

double fpscount = 60;
void update(double deltatime) {
    //fps
    fpscount = 1/deltatime;
    cout<<fpscount<<"\n";
    //update angle
    xangle+=xanglev*deltatime;
    yangle+=yanglev*deltatime;
    // update pos
    //camerav.y+=-9.8*deltatime;
    if(camerapos.y<0) {
        camerapos.y = 0;
        camerav.y = 0;
    }
    camerapos.x+=camerav.x*deltatime;camerapos.y+=camerav.y*deltatime;camerapos.z+=camerav.z*deltatime;

    // move camera
    if(!console) {
        if(keys[SDL_SCANCODE_SLASH]) {
            console=true;
        }
        if(keys[SDL_SCANCODE_W]) {
            camerapos.x+=cameraspeed*deltatime*sin(cyang);
            camerapos.z+=cameraspeed*deltatime*cos(cyang);
        }if(keys[SDL_SCANCODE_A]) {
            camerapos.x+=-cameraspeed*deltatime*cos(cyang);
            camerapos.z+=cameraspeed*deltatime*sin(cyang);
        }if(keys[SDL_SCANCODE_S]) {
            camerapos.x+=-cameraspeed*deltatime*sin(cyang);
            camerapos.z+=-cameraspeed*deltatime*cos(cyang);
        }if(keys[SDL_SCANCODE_D]) {
            camerapos.x+=cameraspeed*deltatime*cos(cyang);
            camerapos.z+=-cameraspeed*deltatime*sin(cyang);
        }if(keys[SDL_SCANCODE_SPACE]) {
            camerapos.y+=cameraspeed*deltatime;
        }else if(keys[SDL_SCANCODE_C]) {
            camerapos.y+=-cameraspeed*deltatime;
        }
    } else {
        
    }
    
}

int scale = HEIGHT/10;
void render() {// TODO: flat shading
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);

    for(int i = 0; i< objects.size(); i++) { // for every object
        for(int j = 0; j <objects[i].edges.size();j++) { // for every poly
            // render faces
            bool isnull = false;
            int polylen = objects[i].edges[j].size();
            SDL_Vertex poly[polylen];
            Point3D projverts[polylen];
            SDL_FPoint points[polylen];
            for(int k = 0; k < polylen;k++) { // for every vertex index
                Point3D vertex = objects[i].vertices[objects[i].edges[j][k]];
                
                vertex = rotate(vertex, objects[i].xangle, objects[i].yangle);
                double rx = (vertex.x+objects[i].pos.x)-camerapos.x;double ry = (vertex.y+objects[i].pos.y)-camerapos.y;double rz = (vertex.z+objects[i].pos.z)-camerapos.z;
                projverts[k]=project(rotate({.x=rx,.y=ry,.z=rz},cxang,cyang));
                float x = static_cast<float>(WIDTH/2+projverts[k].x*scale);
                float y = static_cast<float>(HEIGHT/2-projverts[k].y*scale);
                
                poly[k].position = {x, y};
                poly[k].color={1.0f, 0.3f, 0.3f, 1.0f};
                poly[k].tex_coord={0.0f,0.0f};
                points[k].x = x;points[k].y = y;
                
                // behind camera check & back face //TODO: backface culling
                if(projverts[k].z<=0) {
                    isnull=true;
                } else {
                    
                }
            }
            //Point3D midd = {
            //.x=(objects[i].vertices[objects[i].edges[j][0]].x+objects[i].vertices[objects[i].edges[j][1]].x+objects[i].vertices[objects[i].edges[j][2]].x)/3+objects[i].pos.x-camerapos.x,
            //.y=(objects[i].vertices[objects[i].edges[j][0]].y+objects[i].vertices[objects[i].edges[j][1]].y+objects[i].vertices[objects[i].edges[j][2]].y)/3+objects[i].pos.y-camerapos.y,
            //.z=(objects[i].vertices[objects[i].edges[j][0]].z+objects[i].vertices[objects[i].edges[j][1]].z+objects[i].vertices[objects[i].edges[j][2]].z)/3+objects[i].pos.z-camerapos.z
            //};
            //Point3D projmidd = project(rotate(midd,cxang,cyang));
            //Point3D newnormal = {
            //    .x=objects[i].normalv[j].x+midd.x,
            //    .y=objects[i].normalv[j].y+midd.y,
            //    .z=objects[i].normalv[j].z+midd.z
            //};
            //if(acos(rotate(newnormal,cxang,cyang).z*1)<=M_PI/2) {
            //    isnull = true;
            //}
            if(!isnull) {
                //SDL_RenderGeometry(renderer,nullptr,poly,polylen,nullptr,0);
                SDL_SetRenderDrawColor(renderer,255,255,255,255);
                SDL_RenderLines(renderer, points, polylen);
                // render normals
                //Point3D projnorm = project(rotate(newnormal,cxang,cyang));
                //SDL_SetRenderDrawColor(renderer,0,0,255,255);
                //SDL_RenderLine(renderer, WIDTH/2+scale*projmidd.x, HEIGHT/2-scale*projmidd.y,WIDTH/2+scale*projnorm.x,HEIGHT/2-scale*projnorm.y);
            }
        }
    }
    
    SDL_RenderPresent(renderer);
}

double ltime;
void mainloop(double deltatime) {
    update(deltatime);
    render();
}

int main(int agrc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer("Title",WIDTH,HEIGHT,0,&window,&renderer);
    SDL_GetWindowSurface(window);
    SDL_UpdateWindowSurface(window);
    SDL_SetWindowRelativeMouseMode(window, true);
    SDL_ShowCursor();

    objects.push_back(readfile("assets/tcube.obj"));
    objects[0].pos={.x=0,.y=0,.z=5};
    //objects.push_back(readfile("assets/cilinder.obj"));
    //objects[1].pos = {.x=3,.y=0,.z=3};
    makenormals();
    bool running=true;
    SDL_Event event;
    //ltime = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count();
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if(event.type == SDL_EVENT_MOUSE_MOTION) {
                float mf = 0.0008f;
                cyang+=event.motion.xrel*mf;
                cxang+=-event.motion.yrel*mf;
            }
        }
        auto now = chrono::steady_clock::now();
        double ctime = chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch()).count();
        //cout<<abs(ctime-ltime)<<"\n";
        if((abs(ctime-ltime)/1000000000)>=1/FPS) {
            mainloop((abs(ctime-ltime))/1000000000);
        }
        ltime = ctime;
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    cout<<"exiting";
    return 0;
};