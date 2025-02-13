#include <iostream>
#include <vector>
#include <unistd.h>

#include "vertex.h"

using namespace std;

int main(int argc, char ** argv){

    int vertex_count = 0;

    vector<Vertex> vertices;

    for (int i = 0; i < argc; i ++){

        string argi = argv[i];
        if(argi.find("--count=") == 0){
            vertex_count = atoi(argi.substr(8).c_str());
        }
    }

    if (vertex_count < 1) {
        cout << "must supply a --count greater than 0" << endl;
        exit(0);
    }

    int time_seed = time(0);
    int process_seed = getpid() + 1;

    int seed = time_seed * process_seed;

    srand(time(0));

    for (int i = 0; i < vertex_count; i++){
        Vertex new_vertex;

        new_vertex.x = (rand() % 200) - 100;
        new_vertex.y = (rand() % 200) - 100;
        new_vertex.id = i;

        vertices.push_back(new_vertex);

    }

    for (int i = 0; i < vertices.size(); i++) {
        cout << vertices[i].x << ", " << vertices[i].y << endl;
    }

    return 0;

}