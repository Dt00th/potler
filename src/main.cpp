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
// SDL
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

struct Point2D {
    double x, y;
};
struct Point3D {
    double x, y, z;
};
struct Object {
    struct vector<Point3D> vertices;
    vector<vector<int>> edges;
};
Object readfile(string path) { // FIXME: readfile
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
    cout<<"file close\n";
    file.close();
    return object;
}

// camera
Point3D camerapos;
double xangle=0.0f;double yangle=0.0f;
double xanglev=0.5f;double yanglev=1.5f;
double projd=10.0f; double FOV = 60.0f;
double projscale=600.0f;
// world
vector<Object> objects;

Point3D project(Point3D p) {
    double zoffset = p.z+projd;
    return {x:projscale*p.x/FOV/zoffset, y:projscale*p.y/FOV/zoffset, z:zoffset};
}

void addcube() {
    struct Object cube ={
    .vertices={
        {.x=-1, .y=-1, .z=-1},  // 0
        {.x=1, .y=-1, .z=-1},     // 1
        {.x=1, .y=1, .z=-1},     // 2
        {.x=-1, .y=1, .z=-1},     // 3
        {.x=-1, .y=-1, .z=1},     // 4
        {.x=1, .y=-1, .z=1},     // 5
        {.x=1, .y=1, .z=1},     // 6
        {.x=-1, .y=1, .z=1}     // 7
        },
    .edges={
        {0,2,1},
        {0,3,2},
        {3,7,2},
        {7,6,2},
        {4,1,5},
        {5,2,6},
        {2,5,1},
        {0,7,3},
        {0,4,7},
        {1,4,0},
        {4,6,7},
        {4,5,6}
    }
    };
    objects.push_back(cube);
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

void update() {
    xangle+=xanglev;yangle+=yanglev;
}

int scale = HEIGHT/10;
void render() {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);

    for(int i = 0; i< objects.size(); i++) { // for every object
        for(int j = 0; j <objects[i].edges.size();j++) { // for every poly
            int polylen = objects[i].edges[j].size();
            SDL_Vertex poly[polylen];
            Point3D projverts[polylen];
            for(int k = 0; k < polylen;k++) { // for every vertex index
                Point3D vertex = objects[i].vertices[objects[i].edges[j][k]];
                projverts[k]=project(vertex);
                float x = static_cast<float>(WIDTH/2+projverts[k].x*scale);
                float y = static_cast<float>(HEIGHT/2+projverts[k].y*scale);
                poly[k].position = {x, y};
                poly[k].color={1.0f, 0.2f, 0.2f, 1.0f};
                poly[k].tex_coord={0.0f,0.0f};
                //cout<<"k: "<<k<<", x: "<<objects[i].vertices[objects[i].edges[j][k]].x<<", y: "<<objects[i].vertices[objects[i].edges[j][k]].y<<", z: "<<objects[i].vertices[objects[i].edges[j][k]].z<<"; ";
            }
            //cout<<endl;
                    
            SDL_RenderGeometry(renderer,nullptr,poly,polylen,nullptr,0);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void mainloop() {
    update();
    render();
}

int main(int agrc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer("Title",WIDTH,HEIGHT,0,&window,&renderer);
    SDL_GetWindowSurface(window);
    SDL_UpdateWindowSurface(window);

    //addcube();
    //addt();
    objects.push_back(readfile("assets/tcube.obj"));
    bool running=true;
    SDL_Event Event;
    while (running) {
        while (SDL_PollEvent(&Event)) {
            mainloop();
            if (Event.type == SDL_EVENT_QUIT) {
              running = false;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    cout<<"exiting";
    return 0;
};