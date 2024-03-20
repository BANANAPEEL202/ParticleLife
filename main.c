
#include "ParticleLife.h"
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------


const int screenWidth = 1400;
const int screenHeight = 800;

//Simulation Initialization
#define numPartitions 48

const double dt = 0.01; //delta t
const double frictionHalfLife = 0.040; //friction coefficient
const double rMax = 200; //max radius to search particles in
const int m = 3; //num colors
double matrix[3][3] = {{1, 0.5, -0.5},
                       {-0.5, 1, 0.5},
                       {0.5, -0.5, 1}};
int partitionHash[numPartitions][9];
const double forceFactor = 10;
//enum color {ORANGE, RED, BLUE};

double frictionFactor;


Partition partitions[numPartitions];
Particle particles[n];


int main(void)
{
    // Raylib Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Particle Life");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    frictionFactor = pow(0.5, dt/frictionHalfLife);



    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        Particle newParticle;
        newParticle.color = floor(rand()%6);
        newParticle.position.x = rand() % screenWidth;
        newParticle.position.y = rand() % screenHeight;
        newParticle.velocity.x = 0;
        newParticle.velocity.y = 0;
        newParticle.partition = getPartition(newParticle.position);
        particles[i] = newParticle;
        addToPartition(&particles[i]);
    }
    generatePartitionHash();


    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        BeginDrawing();
        ClearBackground(BLACK);
        float radius = 3;
        for (int i = 0; i < n; i++){
            Particle particle = particles[i];
            if (particle.color == 0) {
                DrawCircleV(particle.position, radius, ORANGE);
            }
            else if (particle.color == 1) {
                DrawCircleV(particle.position, radius, BLUE);
            }
            else if (particle.color == 2) {
                DrawCircleV(particle.position, radius, GREEN);
            }

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

    int batchSize = 5000;
    pthread_t threadIds[n/batchSize];
    for (int i = 0; i < n/batchSize; i++) {
        Vector2 indexes;
        indexes.x = i*batchSize;
        indexes.y = (i+1)*batchSize;
        pthread_create(&threadIds[i], NULL, updateForce, &indexes);

    }
    for (int i = 0; i < n/batchSize; i++) {
        pthread_join(threadIds[i], NULL);
    }


    for (int i = 0; i < n; i++) {
        particles[i].position.x += particles[i].velocity.x * dt;
        if (particles[i].position.x < 0) {
            particles[i].position.x = screenWidth + particles[i].position.x;
        }
        if (particles[i].position.x > screenWidth) {
            particles[i].position.x = 0 + particles[i].position.x-screenWidth ;
        }

        particles[i].position.y += particles[i].velocity.y * dt;
        if (particles[i].position.y < 0) {
            particles[i].position.y = screenHeight + particles[i].position.y;
        }
        if (particles[i].position.y > screenHeight) {
            particles[i].position.y = 0 + particles[i].position.y-screenHeight ;
        }
        int newPartition = getPartition(particles[i].position);
        if (newPartition != particles[i].partition) {
            removeFromPartition(&particles[i]);
            particles[i].partition = newPartition;
            addToPartition(&particles[i]);
        }
    }




}


double force(double r, double attraction){
    const double beta = 0.2;
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

int getPartition(Vector2 coord){
    int partition = floor(coord.x/rMax);
    partition += floor(coord.y/rMax) * screenWidth/rMax;
    return partition;
}

int isClosePartition(int source, int compare) {
    for (int i = 0; i < 9; i++){
        if (partitionHash[source][i] == compare) {
            return 1;
        }
    }
    return 0;
}

void generatePartitionHash(){
    int partitionCol = screenWidth/rMax;
    int partitionRow = screenHeight/rMax;


    for (int r = 0; r < partitionRow; r++){
        for (int c = 0; c < partitionCol; c++){
            int index = 0;
            for (int r2 = -1; r2 <= 1; r2++){
                for (int c2 = -1; c2 <= 1; c2++){
                    partitionHash[r*partitionCol+c][index] = (modulo((r+r2), partitionRow) * partitionCol + modulo((c+c2), partitionCol));
                    index++;
                }
            }

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
        memset(partitions[i].particles, -1, n);
    }
}

void removeFromPartition(Particle* particle) {
    Partition partition = partitions[(*particle).partition];
    for (int i = 0; i < partition.numParticles; i++) {
        if (&(partition.particles[i]) == particle) {
            for (int j = i; j < partition.numParticles; j++) {
                partition.particles[j] = partition.particles[j+1];
            }
        }
    }
    partition.numParticles--;
}

void addToPartition(Particle* particle) {
    int partition = (*particle).partition;
    partitions[partition].particles[partitions[partition].numParticles] = *particle;
    partitions[partition].numParticles++;
}

void *updateForce (void *indexes2){

    Vector2 *indexes = (Vector2*)(indexes2);
    for (int i = (*indexes).x; i < (*indexes).y; i++) {
        double netForceX = 0;
        double netForceY = 0;
        for (int partitionIndex = 0; partitionIndex < 9; partitionIndex++) {
            Partition partition = partitions[partitionHash[particles[i].partition][partitionIndex]];
            for (int j = 0; j < partition.numParticles; j++) {

                double xDist = partition.particles[j].position.x - particles[i].position.x;
                double yDist = partition.particles[j].position.y - particles[i].position.y;
                double dist = hypot(xDist, yDist);
                if (dist > 0 && dist < rMax) {
                    double f = force(dist / rMax, matrix[particles[i].color][partition.particles[j].color]);
                    netForceX += xDist / dist * f;
                    netForceY += yDist / dist * f;
                }

            }
        }

        netForceX = netForceX*rMax*forceFactor;
        netForceY = netForceY*rMax*forceFactor;

        particles[i].velocity.x = particles[i].velocity.x * frictionFactor;
        particles[i].velocity.y = particles[i].velocity.y * frictionFactor;

        particles[i].velocity.x = netForceX * dt;
        particles[i].velocity.y = netForceY * dt;
    }



}