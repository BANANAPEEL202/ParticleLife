//
// Created by Jason on 3/18/24.
//

#ifndef EXAMPLE_PARTICLELIFE_H
#define EXAMPLE_PARTICLELIFE_H

#include "raylib.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>


#define n 5000

typedef struct  {
    Vector2 position;
    Vector2 velocity;
    int color;
    int partition;
} Particle;

typedef struct {
    Particle particles[n];
    int numParticles;
} Partition;

void updateParticles();
double force(double, double);
int getPartition(Vector2);
int isClosePartition(int, int);
void generatePartitionHash();
int modulo(int, int);
void removeFromPartition(Particle*);
void addToPartition(Particle*);

void *updateForce (void*);

#endif //EXAMPLE_PARTICLELIFE_H
