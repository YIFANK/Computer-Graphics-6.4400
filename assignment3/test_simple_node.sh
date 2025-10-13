#!/bin/bash

# Test script for SimpleSystemNode with different integrators
echo "Testing SimpleSystemNode with different integrators..."
echo ""

echo "1. Testing with Forward Euler integrator (timestep: 0.01):"
echo "   Command: ./assignment3 e 0.01"
echo "   This should show a particle moving in a circular motion"
echo ""

echo "2. Testing with Trapezoidal integrator (timestep: 0.001):"
echo "   Command: ./assignment3 t 0.001"
echo "   This should show more accurate circular motion"
echo ""

echo "3. Testing with RK4 integrator (timestep: 0.005):"
echo "   Command: ./assignment3 r 0.005"
echo "   This should show the most accurate circular motion"
echo ""

echo "Usage examples:"
echo "  cd build"
echo "  ./assignment3 e 0.01    # Forward Euler, 10ms timestep"
echo "  ./assignment3 t 0.001   # Trapezoidal, 1ms timestep"
echo "  ./assignment3 r 0.005   # RK4, 5ms timestep"
echo ""

echo "Expected behavior:"
echo "- You should see a particle (represented by a teapot) moving in a circular motion"
echo "- The particle starts at position (1, 0, 0) and moves counterclockwise"
echo "- Different integrators will show different levels of accuracy"
echo "- Smaller timesteps generally provide better accuracy but may run slower"
echo ""

echo "Controls:"
echo "- Mouse: Rotate camera view"
echo "- ESC: Exit application"
echo "- The particle should move smoothly in a circular path"
