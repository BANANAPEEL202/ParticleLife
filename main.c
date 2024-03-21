
#include "ParticleLife.h"
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------


const int screenWidth = 1400;
const int screenHeight = 800;

//Simulation Initialization
#define numPartitions 48

const double dt = 0.005; //delta t
const double frictionHalfLife = 0.040; //friction coefficient
const double rMax = 100; //max radius to search particles in
/*
const int m = 3; //num colors
double matrix[3][3] = {{1, 0.5, -0.5},
                       {-0.5, 1, 0.5},
                       {0.5, -0.5, 1}};
*/
const int m = 6; //num colors
double matrix[6][6] = {{1, 0.5, -0.5, 0.2, 0.5, 0.6},
                       {-0.5, 1, 0.5, -0.2, -1, -0.3},
                       {1, 0, 0.5, 0.2, 1, -0.3},
                        {-0.5, -0.5, 1, -0.5, 1, -0.3},
                        {0.6, 0.2, 0.5, 0.2, 0.2, 0.3},
                       {0.5, -0.5, 1, 1, -1, 0.5}};


int partitionHash[numPartitions][9];
Vector2 partitionMid[numPartitions];
const double forceFactor = 10;
//enum color {ORANGE, RED, BLUE};
double frictionFactor;

Partition partitions[numPartitions];
Particle particles[n];
const int usePartitions = 1;
const int DEBUG = 0;
Particle playerParticle;

int main(void)
{
    // Raylib Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Particle Life");

    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    frictionFactor = pow(0.5, dt/frictionHalfLife);

    if (DEBUG) {
        playerParticle.color = floor(rand()%6);
        playerParticle.position.x = rand() % screenWidth;
        playerParticle.position.y = rand() % screenHeight;
        playerParticle.velocity.x = 0;
        playerParticle.velocity.y = 0;
        playerParticle.partition = getPartition(playerParticle.position);
        playerParticle.id = -1;
        addToPartition(&playerParticle);
    }

    for (int i = 0; i < numPartitions; i++) {
        partitions[i].numParticles = 0;
    }

    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        Particle newParticle;
        newParticle.color = floor(rand()%m);
        newParticle.position.x = rand() % screenWidth;
        newParticle.position.y = rand() % screenHeight;
        newParticle.velocity.x = 0;
        newParticle.velocity.y = 0;
        newParticle.partition = getPartition(newParticle.position);
        newParticle.id = i;
        particles[i] = newParticle;
        addToPartition(&particles[i]);
    }
    generatePartitionHash();


    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (DEBUG && IsWindowFocused()){
            double xDist = GetMouseX() - playerParticle.position.x;
            double yDist = GetMouseY() - playerParticle.position.y;
            double dist = hypot(xDist, yDist);
            double f = fmax(dist*1.5, 10);
            double netForceX = xDist / dist * f;
            double netForceY = yDist / dist * f;
            netForceX = netForceX*dist;
            netForceY = netForceY*dist;

            playerParticle.velocity.x = playerParticle.velocity.x * frictionFactor;
            playerParticle.velocity.y = playerParticle.velocity.y * frictionFactor;

            playerParticle.velocity.x += netForceX * dt;
            playerParticle.velocity.y += netForceY * dt;

            updatePosition(&playerParticle);

            //draw partitions
            int subPartitions[4] = {0};
            getSubPartitions(subPartitions, &playerParticle);
            for (int i = 0; i < 4; i++) {
                int partition = partitionHash[playerParticle.partition][subPartitions[i]];
                int x = modulo (partition, screenWidth/(rMax*2))*rMax*2;
                int y = floor(partition/(screenWidth/(rMax*2)))*rMax*2;
                DrawRectangle(x, y, rMax*2, rMax*2, (Color){ 255, 161, 0, 125 });
            }
            for (int x = 0; x < screenWidth; x += rMax*2) {
                DrawLine(x, 0, x, screenHeight, RED);
            }
            for (int y = 0; y < screenHeight; y += rMax*2) {
                DrawLine(0, y, screenWidth, y, RED);
            }

        }

        if (IsKeyDown(KEY_SPACE)) {
            updateParameters();
            }
        //----------------------------------------------------------------------------------

        BeginDrawing();
        ClearBackground(BLACK);
        float radius = 3;
        for (int i = 0; i < n; i++){
            Particle* particle = &particles[i];
            if ((*particle).color == 0) {
                DrawCircleV((*particle).position, radius, ORANGE);
            }
            else if ((*particle).color == 1) {
                DrawCircleV((*particle).position, radius, BLUE);
            }
            else if ((*particle).color == 2) {
                DrawCircleV((*particle).position, radius, LIME);
            }
            else if ((*particle).color == 3) {
                DrawCircleV((*particle).position, radius, RED);
            }
            else if ((*particle).color == 4) {
                DrawCircleV((*particle).position, radius, PURPLE);
            }
            else if ((*particle).color == 5) {
                DrawCircleV((*particle).position, radius, SKYBLUE);
            }

        }

        if (DEBUG) {
            DrawCircleV((playerParticle).position, radius*3, WHITE);
        }

        EndDrawing();
        updateParticles();
        printf("%d\n", GetFPS());
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;

}

void updateParticles() {

    //update forces and velocities
    for (int i = 0; i < n; i++) {
        updateForce (&particles[i]);
    }

    clearPartitions();
    for (int i = 0; i < n; i++) {
        updatePosition(&particles[i]);
    }
}

void updateForce(Particle *particle){
    double netForceX = 0;
    double netForceY = 0;
    int subPartitions[4] = {0};
    getSubPartitions(subPartitions, particle);

    if (usePartitions) {

        for (int k = 0; k < 4; k++) {
            //this must be a pointer. otherwise it copies the partition by value which is hella slow
            Partition* partition = &partitions[partitionHash[(*particle).partition][subPartitions[k]]];


            for (int j = 0; j < (*partition).numParticles; j++) {

                double xDist = (*partition).particles[j].position.x - (*particle).position.x;
                double yDist = (*partition).particles[j].position.y - (*particle).position.y;
                double dist = hypot(xDist, yDist);
                if (dist > 0 && dist < rMax) {
                    double f = force(dist / rMax, matrix[(*particle).color][(*partition).particles[j].color]);
                    netForceX += xDist / dist * f;
                    netForceY += yDist / dist * f;
                }

            }

        }

    }
    else {
        for (int j = 0; j < n; j++) {
            double xDist = particles[j].position.x - (*particle).position.x;
            double yDist = particles[j].position.y - (*particle).position.y;
            double dist = hypot(xDist, yDist);
            if (dist > 0 && dist < rMax) {
                double f = force(dist / rMax, matrix[(*particle).color][particles[j].color]);
                netForceX += xDist / dist * f;
                netForceY += yDist / dist * f;
            }

        }
    }

    netForceX = netForceX*rMax*forceFactor;
    netForceY = netForceY*rMax*forceFactor;

    (*particle).velocity.x = (*particle).velocity.x * frictionFactor;
    (*particle).velocity.y = (*particle).velocity.y * frictionFactor;

    (*particle).velocity.x += netForceX * dt;
    (*particle).velocity.y += netForceY * dt;
}


double force(double r, double attraction){
    const double beta = 0.5;
    if (r < beta){
        return r / beta - 1;
    }
    else if (beta < r && r < 1){
        return attraction * (1-fabs(2*r-1-beta) / (1-beta));
    }
    else {
        return 0;
    }
}

void updatePosition(Particle* particle) {
    (*particle).position.x += (*particle).velocity.x * dt;
    if ((*particle).position.x < 0) {
        (*particle).position.x = screenWidth + (*particle).position.x;
    }
    if ((*particle).position.x > screenWidth) {
        (*particle).position.x = 0 + (*particle).position.x-screenWidth ;
    }

    (*particle).position.y += (*particle).velocity.y * dt;
    if ((*particle).position.y < 0) {
        (*particle).position.y = screenHeight + (*particle).position.y;
    }
    if ((*particle).position.y > screenHeight) {
        (*particle).position.y = 0 + (*particle).position.y-screenHeight ;
    }
    int newPartition = getPartition((*particle).position);
    //if (newPartition != (*particle).partition) {
    //removeFromPartition(&(*particle));
    (*particle).partition = newPartition;
    addToPartition(&(*particle));
    //}
}

int getPartition(Vector2 coord){
    int partition = floor(coord.x/(rMax*2));
    partition += floor(coord.y/(rMax*2)) * screenWidth/(rMax*2);
    return partition;
}


void generatePartitionHash(){
    int partitionCol = screenWidth/(rMax*2);
    int partitionRow = screenHeight/(rMax*2);

    for (int r = 0; r < partitionRow; r++){
        for (int c = 0; c < partitionCol; c++){
            int index = 0;
            for (int r2 = -1; r2 <= 1; r2++){
                for (int c2 = -1; c2 <= 1; c2++){
                    partitionHash[r*partitionCol+c][index] = (modulo((r+r2), partitionRow) * partitionCol + modulo((c+c2), partitionCol));
                    index++;
                }
            }
            Vector2 midVector;
            midVector.x = rMax/2 + c*rMax*2;
            midVector.y = rMax/2 + r*rMax*2;
            partitionMid[r*partitionCol+c] = midVector;
        }
    }
}

int modulo(int a, int b) {
    int m = a % b;
    if (m < 0) {
        m = (b < 0) ? m - b : m + b;
    }
    return m;
}

void clearPartitions() {
    for (int i = 0; i < numPartitions; i++) {
        //memset(partitions[i].particles, -1, n);
        partitions[i].numParticles = 0;
    }
}

void removeFromPartition(Particle* particle) {
    Partition partition = partitions[(*particle).partition];
    for (int i = 0; i < partition.numParticles; i++) {
        if (partition.particles[i].id == (*particle).id) {
            for (int j = i; j < partition.numParticles; j++) {
                partition.particles[j] = partition.particles[j+1];
            }
            partition.numParticles--;
        }
    }

}

void addToPartition(Particle* particle) {
    int partition = (*particle).partition;
    partitions[partition].particles[partitions[partition].numParticles] = *particle;
    partitions[partition].numParticles+=1;
}

void getSubPartitions(int *arr, Particle *particle) {
    double midX = partitionMid[(*particle).partition].x;
    double midY = partitionMid[(*particle).partition].y;

    double x = (*particle).position.x;
    double y = (*particle).position.y;

    if (x < midX && y < midY) { //top left
        arr[0] = 0;
        arr[1] = 1;
        arr[2] = 3;
        arr[3] = 4;
    }
    else if (x > midX && y < midY) { //top right
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 4;
        arr[3] = 5;
    }
    else if (x < midX && y > midY) { //bottom left
        arr[0] = 3;
        arr[1] = 4;
        arr[2] = 6;
        arr[3] = 7;
    }
    else { //bottom right
        arr[0] = 4;
        arr[1] = 5;
        arr[2] = 7;
        arr[3] = 8;
    }
}

void updateParameters(){
    srand(time(NULL));

    // Fill the matrix with random floats between 0 and 1
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            matrix[i][j] = (float)rand() / (float)RAND_MAX * 2 -1;
        }
    }
}