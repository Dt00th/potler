#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <cmath>
#include <cstddef>
#include <vector>
#include <iostream>
#include <chrono>
#include <math.h>

const int WIDTH = 800;
const int HEIGHT = 600;
const float FOV = 7.0f;

struct Point3D {
    float x, y, z;
};

struct Point2D {
    int x, y;
};

struct Object {
    std::vector<Point3D> vertices;
    std::vector<std::pair<int, int>> edges;
};

Object newcube(float size) {
    Object cube;
    cube.vertices = {
        {-size, -size, -size}, {size, -size, -size}, {size, size, -size}, {-size, size, -size},
        {-size, -size, size}, {size, -size, size}, {size, size, size}, {-size, size, size}
    };
    cube.edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    return cube;
};

Object readmodel() {

    return Object{};
};

Point3D rotate(Point3D p, float angleX, float angleY) {
    Point3D rotated = p;

    float cosY = cos(angleY);
    float sinY = sin(angleY);
    float x1 = rotated.x * cosY - rotated.z * sinY;
    float z1 = rotated.x * sinY + rotated.z * cosY;
    rotated.x = x1;
    rotated.z = z1;

    float cosX = cos(angleX);
    float sinX = sin(angleX);
    float y2 = rotated.y * cosX - rotated.z * sinX;
    float z2 = rotated.y * sinX + rotated.z * cosX;
    rotated.y = y2;
    rotated.z = z2;

    return rotated;
};

Point2D project(Point3D p) {
    float distance = 40.0f;
    float zOffset = p.z + distance;
    int stika = 2000;

    return Point2D{
        static_cast<int>(stika * (p.x/FOV / zOffset)),
        static_cast<int>(stika * (p.y/FOV / zOffset))
    };
};

// Object
float size = 4.0f;
Object cube = newcube(size);
std::vector<Point3D> vertices = cube.vertices;
std::vector<std::pair<int, int>> edges = cube.edges;

bool running = true;
SDL_Event event;
float angleX = 0.0f;
float angleY = 0.0f;

SDL_Window* window;
SDL_Renderer* renderer;

int n = 0;
int update(int deltatime) {
    // Rotate
    //angleX += 0.01f*deltatime;
    //angleY += 0.015f*deltatime;
    angleY+=.01*180/(M_PI);
    
    return 0;
}

int renderscale = 5;
int render() {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Project
    std::vector<Point2D> projectedpoints;
    for(const auto& vertex : vertices) {
        Point3D rotated = rotate(vertex, angleX, angleY);
        projectedpoints.push_back(project(rotated));
    }
    
    // Draw the edges of the cube
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(const auto& edge: edges) {
        Point2D p1 = projectedpoints[edge.first];
        Point2D p2 = projectedpoints[edge.second];
        //std::cout<<"1: ("; std::cout<<p1.x; std::cout<<", "; std::cout<<p1.y; std::cout<<"); 2: ("; std::cout<<p2.x; std::cout<<", "; std::cout<<p2.y; std::cout<<") ";
        SDL_RenderDrawLine(renderer, renderscale*p1.x+WIDTH/2, renderscale*p1.y+HEIGHT/2, renderscale*p2.x+WIDTH/2, renderscale*p2.y+HEIGHT/2);
    }
    
    // Test
    int vertlen = 3;
    SDL_Vertex vertices2[3] = {
        {{-100, 100}, {1, 0, 0}, {0, 1}},
        {{100, 100}, {1, 0, 0}, {0, 1}},
        {{0, -100}, {1, 0, 0}, {0, 1}}
    };
    SDL_RenderGeometry(renderer, NULL, vertices2, vertlen, NULL, vertlen);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
    return 0;
}
int lasttime=0;
int fps = 60;
int mainloop() {
    auto now = std::chrono::steady_clock::now();
    int currenttime = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    int deltatime = (currenttime-lasttime)*((16*3)/(5*10^7));
    lasttime = currenttime;

    update(deltatime);
    render();
    return 0;
}


int main(int agrc, char* argv[]) {
    // initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("potler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // running loop
    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) running = false;
        }
        mainloop();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
};