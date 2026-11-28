#!/bin/bash

for lowedge in {10..299}; do
    sbatch run-using-local-scratch $lowedge
done
